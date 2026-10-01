#include <raylib.h>
#include <raymath.h>
#include <vector>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const int CELL_SIZE = 80;
const int GRID_WIDTH = WINDOW_WIDTH / CELL_SIZE;
const int GRID_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;
const float elasticity = 1.0f;
const int SPAWNS_BEFORE_BIG = 10;

//g++ UnifiedGrid.cpp -o out -I raylib/ -L raylib/ -lraylib -lopengl32 -lgdi32 -lwinmm


struct particle {
    int id;
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    float mass;
    float inverse_mass; // A variable for 1 / mass. Used in the calculation for acceleration = sum of forces / mass

    int min_x;
    int min_y;
    int max_x;
    int max_y;
};

struct GridCell {
    std::vector<particle*> particles;
    Vector2 position;
    Vector2 size;
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

//helper to get the cell index of a coordinate
int GetCellIndex(float value, int count) {
    int index = (int)(value / CELL_SIZE);
    if (index < 0) index = 0;
    if (index > count - 1) index = count - 1;
    return index;
}

//helper to rebuild the particle list of every cell using each particle's AABB
void UpdateGrid(GridCell grid[GRID_WIDTH][GRID_HEIGHT], std::vector<particle*>& particles) {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            grid[x][y].particles.clear();
        }
    }
 
    for (particle* p : particles) {
        p->min_x = GetCellIndex(p->position.x - p->radius, GRID_WIDTH);
        p->max_x = GetCellIndex(p->position.x + p->radius, GRID_WIDTH);
        p->min_y = GetCellIndex(p->position.y - p->radius, GRID_HEIGHT);
        p->max_y = GetCellIndex(p->position.y + p->radius, GRID_HEIGHT);
 
        for (int x = p->min_x; x <= p->max_x; x++) {
            for (int y = p->min_y; y <= p->max_y; y++) {
                grid[x][y].particles.push_back(p);
            }
        }
    }
}
 
//helper to check collisions only between particles that share a cell
void GridCollisions(GridCell grid[GRID_WIDTH][GRID_HEIGHT]) {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            std::vector<particle*>& cell = grid[x][y].particles;
 
            for (size_t i = 0; i < cell.size(); i++) {
                for (size_t j = i + 1; j < cell.size(); j++) {
                    particle* a = cell[i];
                    particle* b = cell[j];
 
                    int shared_x = (a->min_x > b->min_x) ? a->min_x : b->min_x;
                    int shared_y = (a->min_y > b->min_y) ? a->min_y : b->min_y;
                    if (shared_x != x || shared_y != y) continue;
 
                    BalltoBallCollision(*a, *b);
                }
            }
        }
    }
}

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
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Grid");
    SetTargetFPS(FPS);
    float accumulator = 0.0f;
    std::vector<particle*> particles;
    int spawnCount = 0;
    int nextId = 0;

    GridCell grid[GRID_WIDTH][GRID_HEIGHT];
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            grid[x][y].position = { (float)(x * CELL_SIZE), (float)(y * CELL_SIZE) };
            grid[x][y].size = { (float)CELL_SIZE, (float)CELL_SIZE };
        }
    }

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        accumulator += delta_time;

        if (IsKeyPressed(KEY_SPACE)) {
            Spawnparticles(particles, spawnCount, nextId);
        }
        
        while (accumulator >= TIMESTEP) {
            // update particle position based on velocity
            for (particle* p : particles) {
                p->position = Vector2Add(p->position, Vector2Scale(p->velocity, TIMESTEP));
            }
 
            UpdateGrid(grid, particles);
            GridCollisions(grid);
 
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
 
        // grid visualization with the number of particles in each cell
        for (int x = 0; x < GRID_WIDTH; x++) {
            for (int y = 0; y < GRID_HEIGHT; y++) {
                DrawRectangleLines(grid[x][y].position.x, grid[x][y].position.y, grid[x][y].size.x, grid[x][y].size.y, GRAY);
 
                int count = (int)grid[x][y].particles.size();
                if (count > 0) {
                    DrawText(TextFormat("%d", count), (int)grid[x][y].position.x + 3, (int)grid[x][y].position.y + 3, 10, RED);
                }
            }
        }
 
        DrawText("Press SPACE to spawn particles", 10, 10, 20, BLACK);
        DrawText(TextFormat("Particles: %d  FPS: %d", (int)particles.size(), GetFPS()), 10, 35, 20, BLACK);
        EndDrawing();
    }

    for (particle* p : particles) {
        delete p;
    }

    CloseWindow();
    return 0;
}
