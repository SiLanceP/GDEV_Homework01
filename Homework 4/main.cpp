#include <raylib.h>
#include <raymath.h>
#include <vector>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float elasticity = 1.0f;
const int SPAWNS_BEFORE_BIG = 10;
const int MAX_DEPTH = 5;

//g++ Main.cpp -o out -I raylib/ -L raylib/ -lraylib -lopengl32 -lgdi32 -lwinmm


struct particle {
    int id;
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass

    float min_x;
    float min_y;
    float max_x;
    float max_y;
};

struct QuadTreeNode {
    Vector2 center;
    float half_width;
    int depth;
    QuadTreeNode* parent = nullptr;
    QuadTreeNode* children[4] = { nullptr, nullptr, nullptr, nullptr };
    std::vector<particle*> particles;

    bool IsLeaf() const {
        return children[0] == nullptr;
    }
};



//helper code for the collision equation (ball-ball collision)
void BalltoBallCollision(particle& a, particle& b) {
    float distance = Vector2Distance(a.position, b.position);
    float radiisum = a.radius + b.radius;

    if (radiisum > distance && distance > 0.0f) {
        Vector2 normal = Vector2Normalize(Vector2Subtract(a.position, b.position));
        Vector2 relative_velocity = Vector2Subtract(a.velocity, b.velocity);
        float velocity_along_normal = Vector2DotProduct(relative_velocity, normal);

        if (velocity_along_normal < 0) {
            float j_impulse = -(1.0f + elasticity) * velocity_along_normal;
            j_impulse = j_impulse / (a.inverse_mass + b.inverse_mass);

            Vector2 impulse = Vector2Scale(normal,j_impulse);

            a.velocity = Vector2Add(a.velocity, Vector2Scale(impulse,a.inverse_mass));
            b.velocity = Vector2Subtract(b.velocity, Vector2Scale(impulse,b.inverse_mass));
        }

        float intersect = radiisum - distance;
        Vector2 correction = Vector2Scale(normal, intersect/(a.inverse_mass + b.inverse_mass));
        a.position = Vector2Add(a.position, Vector2Scale(correction,a.inverse_mass));
        b.position = Vector2Subtract(b.position, Vector2Scale(correction,b.inverse_mass));
    }
}

//helper function for Ball to Wall collision (AABB)
void BalltoWallCollision(particle& ball) {
    if((ball.position.x + ball.radius >= WINDOW_WIDTH && ball.velocity.x > 0) || (ball.position.x - ball.radius <= 0 && ball.velocity.x < 0)) {
        ball.velocity.x *= -1;
    }
    if((ball.position.y + ball.radius >= WINDOW_HEIGHT && ball.velocity.y > 0) || (ball.position.y - ball.radius <= 0 && ball.velocity.y < 0)) {
        ball.velocity.y *= -1;
    }

    // clamp to keep ball inside the screen
    ball.position.x = Clamp(ball.position.x, ball.radius, WINDOW_WIDTH - ball.radius);
    ball.position.y = Clamp(ball.position.y, ball.radius, WINDOW_HEIGHT - ball.radius);
}

//check if particle AABB intersects with node AABB
bool AABBIntersectNode(const particle* p, const QuadTreeNode* node) {
    float nodeMinX = node->center.x - node->half_width;
    float nodeMaxX = node->center.x + node->half_width;
    float nodeMinY = node->center.y - node->half_width;
    float nodeMaxY = node->center.y + node->half_width;

    return !(p->max_x < nodeMinX || p->min_x > nodeMaxX || p->max_y < nodeMinY || p->min_y > nodeMaxY);
}

//check particle AABB intersects with another particle AABB
bool IntersectParticleAABB(const particle* a, const particle* b) {
    return !(a->max_x < b->min_x || a->min_x > b->max_x || a->max_y < b->min_y || a->min_y > b->max_y);
}


//helper to refresh a particles AABB from its position and radius
void UpdateAABB(particle* p) {
    p->min_x = p->position.x - p->radius;
    p->max_x = p->position.x + p->radius;
    p->min_y = p->position.y - p->radius;
    p->max_y = p->position.y + p->radius;
}

//helper to allocate one node
QuadTreeNode* CreateNode(Vector2 center, float half_width, int depth, QuadTreeNode* parent) {
    QuadTreeNode* node = new QuadTreeNode();
    node->center = center;
    node->half_width = half_width;
    node->depth = depth;
    node->parent = parent;
    return node;
}

QuadTreeNode* BuildStaticTree(Vector2 center, float half_width, int depth, QuadTreeNode* parent) {
    QuadTreeNode* node = CreateNode(center, half_width, depth, parent);

    if (depth < MAX_DEPTH) {
        float quarter = half_width / 2.0f;
        for (int i = 0; i < 4; i++) {
            float dx = (i % 2 == 0) ? -quarter : quarter;
            float dy = (i < 2) ? -quarter : quarter;
            Vector2 child_center = { center.x + dx, center.y + dy };
            node->children[i] = BuildStaticTree(child_center, quarter, depth + 1, node);
        }
    }
    return node;
}

int GetChildIndex(const QuadTreeNode* node, const particle* p) {
    bool left   = p->max_x <= node->center.x;
    bool right  = p->min_x >= node->center.x;
    bool top    = p->max_y <= node->center.y;
    bool bottom = p->min_y >= node->center.y;

    if (!(left || right) || !(top || bottom)) return -1;
    return (right ? 1 : 0) + (bottom ? 2 : 0);
}

//insert a particle into the deepest node that completely contains it
void InsertParticle(QuadTreeNode* node, particle* p) {
    if (node->IsLeaf()) {
        node->particles.push_back(p);
        return;
    }

    int index = GetChildIndex(node, p);
    if (index != -1) {
        InsertParticle(node->children[index], p);
    } else {
        node->particles.push_back(p);
    }
}

// free the whole tree
void DestroyTree(QuadTreeNode* node) {
    if (node == nullptr) return;
    for (int i = 0; i < 4; i++) {
        DestroyTree(node->children[i]);
    }
    delete node;
}

QuadTreeNode* CreateTree() {
    float half = (WINDOW_WIDTH > WINDOW_HEIGHT ? WINDOW_WIDTH : WINDOW_HEIGHT) / 2.0f;
    Vector2 center = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
    return BuildStaticTree(center, half, 0, nullptr);
}

void ClearTree(QuadTreeNode* node) {
    node->particles.clear();
    if (!node->IsLeaf()) {
        for (int i = 0; i < 4; i++) ClearTree(node->children[i]);
    }
}

void ReinsertParticles(QuadTreeNode* root, std::vector<particle*>& particles) {
    ClearTree(root);
    for (particle* p : particles) {
        UpdateAABB(p);
        InsertParticle(root, p);
    }
}

void CheckNodeCollisions(QuadTreeNode* node, particle* p) {
    if (!AABBIntersectNode(p, node)) {
        return;
    }

    for (particle* other : node->particles) {
        if (p != other) {
            if (IntersectParticleAABB(p, other)) {
                BalltoBallCollision(*p, *other);
                UpdateAABB(p);
                UpdateAABB(other);
            }
        }
    }

    if(!node->IsLeaf()) {
        for (int i = 0; i < 4; i++) {
            CheckNodeCollisions(node->children[i], p);
        }
    }
}

void QuadTreeCollisions(QuadTreeNode* node, std::vector<particle*>& particles) {
    for (particle* p : particles) {
        CheckNodeCollisions(node, p);
    }
}

void DrawTree(const QuadTreeNode* node) {
    if (node == nullptr) return;
    if (!node->particles.empty()) {
        float x = node->center.x - node->half_width;
        float y = node->center.y - node->half_width;
        float size = node->half_width * 2.0f;

        DrawRectangleLines(int(x), int(y), int(size), int(size), RED);
        DrawText(TextFormat("%d", (int)node->particles.size()), int(x+3), int(y+3), 10, RED);
    }

    if(!node->IsLeaf()) {
        for (int i = 0; i < 4; i++) {
            DrawTree(node->children[i]);
        }
    }
}

// UNCHANGED
void Spawnparticles(std::vector<particle*>& particles, int& spawnCount, int& nextId) {
    // Function to spawn particles
    if (spawnCount == SPAWNS_BEFORE_BIG) {
        spawnCount = 0;
        particle* p = new particle();
        p->id = nextId++;
        p->radius = 25.0f;
        p->mass = 10.0f;
        p->inverse_mass = 1.0f / p->mass;
        p->position = { (float)GetRandomValue(30, WINDOW_WIDTH - 30), (float)GetRandomValue(30, WINDOW_HEIGHT  - 30) };
        p->velocity = { (float)GetRandomValue(-200, 200), (float)GetRandomValue(-200, 200) };
        p->color = { (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255 };
        particles.push_back(p);
    } else {
        spawnCount++;
        for (int i = 0; i < 25; i++) {
            particle* a = new particle();
            a->id = nextId++;
            a->radius = (float)GetRandomValue(5, 10);
            a->mass = 1.0f;
            a->inverse_mass = 1.0f / a->mass;
            a->color = { (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), (unsigned char)GetRandomValue(0, 255), 255 };
            a->position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
            a->velocity = { (float)GetRandomValue(-200, 200), (float)GetRandomValue(-200, 200) };
            particles.push_back(a);
        }
    }

}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Quad Tree");
    SetTargetFPS(FPS);
    float accumulator = 0.0f;
    std::vector<particle*> particles;
    int spawnCount = 0;
    int nextId = 0;
    bool showTree = true;

    QuadTreeNode* root = CreateTree();

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        accumulator += delta_time;

        if (IsKeyPressed(KEY_SPACE)) {
            Spawnparticles(particles, spawnCount, nextId);
        }

        if (IsKeyPressed(KEY_T)) {
            showTree = !showTree;
        }

        while (accumulator >= TIMESTEP) {
            // update particle position based on velocity
            for (particle* p : particles) {
                p->position = Vector2Add(p->position, Vector2Scale(p->velocity, TIMESTEP));
            }

            ReinsertParticles(root, particles);

            QuadTreeCollisions(root, particles);

            // check for wall collisions
            for (particle* p : particles) {
                BalltoWallCollision(*p);
            }

            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        for (particle* p : particles) {
            DrawCircleV(p->position, p->radius, p->color);
        }

        if (showTree) {
            DrawTree(root);
        }

        DrawText(TextFormat("Particles: %d  FPS: %d", (int)particles.size(), GetFPS()), 10, 35, 20, BLACK);

        EndDrawing();
    }

    DestroyTree(root);

    for (particle* p : particles) {
        delete p;
    }

    CloseWindow();
    return 0;
}