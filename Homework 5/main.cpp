#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "entt.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const float FPS = 60;
const float spawn_interval = 1.67f;
const float spawn_region_half = 100.0f;
const float mouse_force = 300.0f;
const float max_speed = 400.0f;

//g++ Main.cpp -o out -std=c++17 -I raylib/ -L raylib/ -lraylib -lopengl32 -lgdi32 -lwinmm

struct TransformComponent {
    Vector2 position;
};

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

struct DestroyComponent {};

void RandomCircleProperties(entt::registry& registry, entt::entity e, ColorComponent& color);

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

void Spawn(entt::registry& registry, float delta_time, float& spawn_timer) {
    spawn_timer += delta_time;
    while (spawn_timer >= spawn_interval) {
        SpawnAsteroid(registry, RandomCenterPosition());
        spawn_timer -= spawn_interval;
    }
}
 
void MouseAttract(entt::registry& registry, float delta_time) {
    Vector2 mouse = GetMousePosition();
    auto view = registry.view<TransformComponent, VelocityComponent, MouseAttractorComponent>();
    for (auto e : view) {
        TransformComponent& t = registry.get<TransformComponent>(e);
        VelocityComponent& v = registry.get<VelocityComponent>(e);
        MouseAttractorComponent& a = registry.get<MouseAttractorComponent>(e);
 
        Vector2 to_mouse = Vector2Subtract(mouse, t.position);
        if (Vector2Length(to_mouse) < 1.0f) continue;
 
        v.velocity = Vector2Add(v.velocity, Vector2Scale(Vector2Normalize(to_mouse), a.force * delta_time));
 
        if (Vector2Length(v.velocity) > max_speed) {
            v.velocity = Vector2Scale(Vector2Normalize(v.velocity), max_speed);
        }
    }
}

void MouseRepel(entt::registry& registry, float delta_time) {
    Vector2 mouse = GetMousePosition();
    auto view = registry.view<TransformComponent, VelocityComponent, MouseRepelComponent>();
    for (auto e : view) {
        TransformComponent& t = registry.get<TransformComponent>(e);
        VelocityComponent& v = registry.get<VelocityComponent>(e);
        MouseRepelComponent& r = registry.get<MouseRepelComponent>(e);
 
        Vector2 away = Vector2Subtract(t.position, mouse);
        if (Vector2Length(away) < 1.0f) continue;
 
        v.velocity = Vector2Add(v.velocity, Vector2Scale(Vector2Normalize(away), r.force * delta_time));
 
        if (Vector2Length(v.velocity) > max_speed) {
            v.velocity = Vector2Scale(Vector2Normalize(v.velocity), max_speed);
        }
    }
}

void ClickDestroy(entt::registry& registry) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
 
    Vector2 mouse = GetMousePosition();
    auto view = registry.view<TransformComponent, CircleComponent>();
    for (auto e : view) {
        TransformComponent& t = registry.get<TransformComponent>(e);
        CircleComponent& c = registry.get<CircleComponent>(e);
        if (CheckCollisionPointCircle(mouse, t.position, c.radius)) {
            registry.emplace_or_replace<DestroyComponent>(e);
            break;
        }
    }
}
 
void Destroy(entt::registry& registry) {
    std::vector<entt::entity> to_destroy;
    auto view = registry.view<DestroyComponent>();
    for (auto e : view) {
        to_destroy.push_back(e);
    }
 
    for (entt::entity e : to_destroy) {
        if (registry.all_of<SplitOnDestroyComponent, TransformComponent>(e)) {
            int count = registry.get<SplitOnDestroyComponent>(e).count;
            Vector2 pos = registry.get<TransformComponent>(e).position;
            for (int i = 0; i < count; i++) {
                SpawnAsteroid(registry, pos);
            }
        }
        registry.destroy(e);
    }
}

void SelfDestruct(entt::registry& registry, float delta_time) {
    auto view = registry.view<SelfDestructComponent>();
    for (auto e : view) {
        SelfDestructComponent& sd = registry.get<SelfDestructComponent>(e);
        sd.time_left -= delta_time;
        if (sd.time_left <= 0.0f) {
            registry.emplace_or_replace<DestroyComponent>(e);
        }
    }
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Asteroids ECS");
    SetTargetFPS(FPS);
 
    entt::registry registry;
    float spawn_timer = 0.0f;
 
    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
 
        Spawn(registry, delta_time, spawn_timer);
        MouseAttract(registry, delta_time);
        MouseRepel(registry, delta_time);
        CircleMovement(registry, delta_time);
        Bounce(registry);
        SelfDestruct(registry, delta_time);
        ClickDestroy(registry);
        Destroy(registry);
 
        BeginDrawing();
        ClearBackground(WHITE);
        DrawCircles(registry);
        EndDrawing();
    }
 
    CloseWindow();
    return 0;
}