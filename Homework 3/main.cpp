#include <raylib.h>
#include <raymath.h>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float VELOCITY_THRESHOLD = 5.0f;
const float MAX_FORCE = 800.0f;
const float BORDER_THICKNESS = 20.0f;
const float POCKET_RADIUS = 25.0f;
const float TIMESTEP = 1.0f / FPS;
const float FRICTION = 1.0f;
const float elasticity = 0.9f;

const int num_walls = 4;
const int num_pockets = 4;
const int num_balls = 5;

struct Ball {
    Vector2 position;
    float radius;
    Color color;
    float mass;
    float inverse_mass;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 forces;
    bool active;
};

struct Wall {
    Rectangle rect;
};

struct pocket {
    Vector2 position;
};

//helper code for the collision equation (ball-ball collision)
void BalltoBallCollision(Ball& a, Ball& b) {
    if (!a.active || !b.active) return;

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
void BalltoWallCollision(Ball& ball, const Wall& wall) {
    if (!ball.active) return;

    float pointX = Clamp(ball.position.x, wall.rect.x, wall.rect.x + wall.rect.width);
    float pointY = Clamp(ball.position.y, wall.rect.y, wall.rect.y + wall.rect.height);

    float distX = ball.position.x - pointX;
    float distY = ball.position.y - pointY;
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 3 - Pool");
    SetTargetFPS(FPS);
    float accumulator = 0.0f;


    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        accumulator += delta_time;

        BeginDrawing();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
