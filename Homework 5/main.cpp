#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "entt.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float TIMESTEP = 1 / FPS; // Sets the timestep to 1 / FPS. But timestep can be any very small value.
const float spawn_interval = 0.5f;
const float spawn_region_half = 100.0f;
const float mouse_force = 300.0f;
const float max_speed = 400.0f;

//g++ Main.cpp -o out -std=c++17 -I raylib/ -L raylib/ -lraylib -lopengl32 -lgdi32 -lwinmm

struct TransformComponent {
    Vector2 position;
}

struct VelocityComponent {
    Vector2 velocity;
};

struct CircleComponent {
    float radius;
};

struct ColorComponent {
    Color color;
};

struct MouseAttractorComponent {
    float force;
};

struct MouseRepelComponent {
    float force;
};

struct SelfDestructComponent {
    float time_left;
};

struct SplitOnDestroyComponent {
    int count;
};

//RandomFloat helper
float RandomFloat(float min, float max) {
    return min + (max - min) * (GetRandomValue(0, 10000) / 10000.0f);
}

entt::entity SpawnAsteroid(entt::registry& registry, Vector2 position) {
    entt::entity e = registry.create();

    registry.emplace<TransformComponent>(e, position);
    registry.emplace<CircleComponent>(e, RandomFloat(20.0f,50.0f));

    float direction = RandomFloat(0.0f, 2.0f * PI);
    float speed = RandomFloat(50.0f, 250.0f);

    registry.emplace<VelocityComponent>(e, Vector2{ cosf(direction) * speed, sinf(direction) * speed });

    ColorComponent& color = registry.emplace<ColorComponent>(e, GRAY);
    RandomCircleProperties(registry, e, color);

    return e;
}

Vector2 RandomCenterPosition() {
    return {
        RandomFloat(WINDOW_WIDTH / 2.0f - spawn_region_half, WINDOW_WIDTH / 2.0f + spawn_region_half),
        RandomFloat(WINDOW_HEIGHT / 2.0f - spawn_region_half, WINDOW_HEIGHT / 2.0f + spawn_region_half)
    };
}


void CircleMovement(entt::registry& registry, float delta_time) {
    auto view = registry.view<TransformComponent, VelocityComponent>();
    for (auto e : view) {
        auto& t = registry.get<TransformComponent>(e);
        auto& v = registry.get<VelocityComponent>(e);
        t.position = Vector2Add(t.position, Vector2Scale(v.velocity, delta_time));
    }
}

void Bounce(entt::registry& registry) {
    auto view = registry.view<TransformComponent, VelocityComponent, CircleComponent>();
    for (auto e : view) {
        auto& t = registry.get<TransformComponent>(e);
        auto& v = registry.get<VelocityComponent>(e);
        float r = registry.get<CircleComponent>(e).radius;

        if (t.position.x - r < 0 && v.velocity.x < 0) {
            v.velocity.x *= -1;
        }
        if (t.position.x + r > WINDOW_WIDTH && v.velocity.x > 0) {
            v.velocity.x *= -1;
        }
        if (t.position.y - r < 0 && v.velocity.y < 0) {
            v.velocity.y *= -1;
        }
        if (t.position.y + r > WINDOW_HEIGHT && v.velocity.y > 0) {
            v.velocity.y *= -1;
        }

        t.position.x = Clamp(t.position.x, r, WINDOW_WIDTH - r);
        t.position.y = Clamp(t.position.y, r, WINDOW_HEIGHT - r);
    }
}

void DrawCircles(entt::registry& registry) {
    auto view = registry.view<TransformComponent, CircleComponent, ColorComponent>();
    for (auto e : view) {
        DrawCircleV(registry.get<TransformComponent>(e).position, registry.get<CircleComponent>(e).radius, registry.get<ColorComponent>(e).color);
    }
}

void RandomCircleProperties(entt::registry& registry, entt::entity e, ColorComponent& color) {
    if (GetRandomValue(1,100) <= 50) {
        if (GetRandomValue(0,1) == 0) {
            registry.emplace<MouseAttractorComponent>(e, mouse_force);
            color.color = BLUE;
        } else {
            registry.emplace<MouseRepelComponent>(e, mouse_force);
            color.color = GREEN;
        }
    }

    if (GetRandomValue(1,100) <= 35) {
        registry.emplace<SelfDestructComponent>(e, RandomFloat(2.0f, 5.0f));
    }

    if (GetRandomValue(1,100) <= 10) {
        registry.emplace<SplitOnDestroyComponent>(e, GetRandomValue(2, 5));
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
        DrawText("PRESS [T] TO TOGGLE QUAD TREE", 10, 60, 12, RED);

        EndDrawing();
    }

    DestroyTree(root);

    for (particle* p : particles) {
        delete p;
    }

    CloseWindow();
    return 0;
}