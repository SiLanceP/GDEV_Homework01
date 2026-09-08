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

const int PARTICLE_COUNT = 1000;

void EmitParticle(Particle* particles, int count, Vector2 position, Vector2 direction, float speed, float lifeTime, Color color) {
    for (int i = 0; i < count; i++) {
        if (!particles[i].isActive) {
            particles[i].isActive = true;
            particles[i].position = position;
            particles[i].direction = direction;
            particles[i].speed = speed;
            particles[i].lifeTime = lifeTime;
            particles[i].color = color;
            return;
        }
    }
}

int main() {
    InitWindow(800, 600, "Particle System");
    SetTargetFPS(60);

    Particle* particles = new Particle[PARTICLE_COUNT];
 
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].isActive = false;
        particles[i].position = { 0.0f, 0.0f };
        particles[i].direction = { 0.0f, 0.0f };
        particles[i].speed = 0.0f;
        particles[i].lifeTime = 0.0f;
        particles[i].color = WHITE;
    }
    
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    delete[] particles;
    CloseWindow();

    return 0;
}