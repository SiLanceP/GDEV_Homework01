#include <raylib.h>
#include <raymath.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>

// create a particle struct 
struct Particle {
    bool isActive;
    Vector2 position;
    Vector2 direction;
    float speed;
    float lifeTime;
    float maxTime;
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

    float rateX = 25.0f; //amount of particles per second
    float rateY = 25.0f;

    float timerX = 0.0f; //lifeTime
    float timerY = 0.0f;

    // default fallback
    int SPACE_BAR = KEY_SPACE;
    int MOUSE_BUTTON = MOUSE_BUTTON_LEFT;
    int keyRateXup = KEY_RIGHT;
    int keyRateXdown = KEY_LEFT;
    int keyRateYup = KEY_UP;
    int keyRateYdown = KEY_DOWN;

    //config loader
    std::ifstream configFile("config.ini");
    std::string line;
    while (std::getline(configFile, line)){
        size_t delimiterPos = line.find("=");
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0,delimiterPos);
            int value = std::stoi(line.substr(delimiterPos+1));

            if (key == "SPACE_BAR") SPACE_BAR = value;
            if (key == "MOUSE_BUTTON") MOUSE_BUTTON = value;
            if (key == "RATE_X_UP") keyRateXup = value;
            if (key == "RATE_X_DOWN") keyRateXdown = value;
            if (key == "RATE_Y_UP") keyRateYup = value;
            if (key == "RATE_Y_DOWN") keyRateYdown = value;
        }
    }

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

        if (IsKeyDown(keyRateXup) && rateX < 50.0f) rateX++;
        if (IsKeyDown(keyRateXdown) && rateX > 1.0f) rateX--;
        if (IsKeyDown(keyRateYup) && rateY < 50.0f) rateY++;
        if (IsKeyDown(keyRateYdown) && rateY > 1.0f) rateY--;

        if (IsKeyDown(SPACE_BAR)) {
            timerX += deltaTime;
            float intervalX = 1.0f / rateX;
            while (timerX >= intervalX) {
                timerX -= intervalX;
                Vector2 pos = {(800.0f / 2.0f), 600.0f};
                Vector2 dir = {(float)GetRandomValue(-100.0f,100.0f) / 100.0f , -1.0f};
                float speed = GetRandomValue(50.0f,100.0f);
                float lifeTime = GetRandomValue(2.0f,5.0f);
                Color color = ColorFromHSV(GetRandomValue(0,360),1.0f,1.0f);

                EmitParticle(particles,PARTICLE_COUNT, pos,dir,speed,lifeTime,color);
            }
        } else {
            timerX = 0.0f;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON)) {
            timerY += deltaTime;
            float intervalY = 1.0f / rateY;
            while (timerY >= intervalY) {
                timerY -= intervalY;
                Vector2 pos = GetMousePosition();
                Vector2 dir = {(float)GetRandomValue(-1.0f,1.0f), (float)GetRandomValue(-1.0f,1.0f)};
                float speed = GetRandomValue(50.0f,100.0f);
                float lifeTime = GetRandomValue(0.5f,2.0f);
                Color color = ColorFromHSV(GetRandomValue(0,360),1.0f,1.0f);

                EmitParticle(particles,PARTICLE_COUNT, pos,dir,speed,lifeTime,color);
            }
        } else {
            timerY = 0.0f;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    delete[] particles;
    CloseWindow();

    return 0;
}