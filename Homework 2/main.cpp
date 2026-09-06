#include <raylib.h>

// create a particle struct 
struct Particle {
    bool isActive;
    Vector2 position;
    Vector2 direction;
    float speed;
    float lifeTime;
    Color color;
};

int main() {
    InitWindow(800, 600, "Particle System");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}