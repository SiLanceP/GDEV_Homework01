#include <raylib.h>
#include <raymath.h>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FPS = 60;
const float VELOCITY_THRESHOLD = 5.0f;
const float MAX_FORCE = 1000.0f;
const float BORDER_THICKNESS = 20.0f;
const float POCKET_RADIUS = 50.0f;
const float TIMESTEP = 1.0f / FPS;
const float FRICTION = 0.4f;
const float elasticity = 0.9f;
const float POCKET_INSET = 35.0f;
const float MAX_DRAG_LENGTH = 150.0f;

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
    float distance = sqrtf(distX * distX + distY * distY);

    if (distance < ball.radius) {
        Vector2 normal;
        if (distance > 0.0f) {
            normal = { distX / distance, distY / distance };
        } else {
            normal = { 0.0f, -1.0f };
        }

        float overlap = ball.radius - distance;
        ball.position = Vector2Add(ball.position, Vector2Scale(normal, overlap));

        float velocity_along_normal = Vector2DotProduct(ball.velocity, normal);
        if (velocity_along_normal < 0) {
            ball.velocity = Vector2Subtract(ball.velocity,
                Vector2Scale(normal, (1.0f + elasticity) * velocity_along_normal));
        }
    }
}

//helper function for Ball to pocket collision
void BalltoPocketCollision(Ball& ball, const pocket& p, Vector2 cue_start_position, bool is_cue_ball) {
    if (!ball.active) return;

    float distance = Vector2Distance(ball.position, p.position);

    if (distance < POCKET_RADIUS) {
        if (is_cue_ball) {
            ball.position = cue_start_position;
            ball.velocity = Vector2Zero();
        } else {
            ball.active = false;
            ball.velocity = Vector2Zero();
        }
    }
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Homework 3 - Pool");
    SetTargetFPS(FPS);
    float accumulator = 0.0f;

    Wall walls[num_walls];
    Ball balls[num_balls];
    pocket pockets[num_pockets];

    walls[0].rect = { 0, 0, (float)WINDOW_WIDTH, BORDER_THICKNESS };
    walls[1].rect = { 0, WINDOW_HEIGHT - BORDER_THICKNESS, (float)WINDOW_WIDTH, BORDER_THICKNESS };
    walls[2].rect = { 0, 0, BORDER_THICKNESS, (float)WINDOW_HEIGHT };
    walls[3].rect = { WINDOW_WIDTH - BORDER_THICKNESS, 0, BORDER_THICKNESS, (float)WINDOW_HEIGHT };

    balls[0].position = { WINDOW_WIDTH / 4.0f, WINDOW_HEIGHT / 2.0f };
    balls[0].radius = 30.0f;
    balls[0].color = WHITE;
    balls[0].mass = 1.0f;
    balls[0].inverse_mass = 1.0f / balls[0].mass;
    balls[0].velocity = Vector2Zero();
    balls[0].acceleration = Vector2Zero();
    balls[0].forces = Vector2Zero();
    balls[0].active = true;

    Vector2 cue_start_position = balls[0].position;

    balls[1].position = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 80 };
    balls[1].radius = 30.0f;
    balls[1].color = BLUE;
    balls[1].mass = 1.0f;
    balls[1].inverse_mass = 1.0f / balls[1].mass;
    balls[1].velocity = Vector2Zero();
    balls[1].acceleration = Vector2Zero();
    balls[1].forces = Vector2Zero();
    balls[1].active = true;

    balls[2].position = { WINDOW_WIDTH / 2.0f - 80, WINDOW_HEIGHT / 2.0f };
    balls[2].radius = 30.0f;
    balls[2].color = BLUE;
    balls[2].mass = 1.0f;
    balls[2].inverse_mass = 1.0f / balls[2].mass;
    balls[2].velocity = Vector2Zero();
    balls[2].acceleration = Vector2Zero();
    balls[2].forces = Vector2Zero();
    balls[2].active = true;

    balls[3].position = { WINDOW_WIDTH / 2.0f + 80, WINDOW_HEIGHT / 2.0f };
    balls[3].radius = 30.0f;
    balls[3].color = BLUE;
    balls[3].mass = 1.0f;
    balls[3].inverse_mass = 1.0f / balls[3].mass;
    balls[3].velocity = Vector2Zero();
    balls[3].acceleration = Vector2Zero();
    balls[3].forces = Vector2Zero();
    balls[3].active = true;

    balls[4].position = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 80 };
    balls[4].radius = 30.0f;
    balls[4].color = BLUE;
    balls[4].mass = 1.0f;
    balls[4].inverse_mass = 1.0f / balls[4].mass;
    balls[4].velocity = Vector2Zero();
    balls[4].acceleration = Vector2Zero();
    balls[4].forces = Vector2Zero();
    balls[4].active = true;

    pockets[0].position = { POCKET_INSET, POCKET_INSET };
    pockets[1].position = { WINDOW_WIDTH - POCKET_INSET, POCKET_INSET };
    pockets[2].position = { POCKET_INSET, WINDOW_HEIGHT - POCKET_INSET };
    pockets[3].position = { WINDOW_WIDTH - POCKET_INSET, WINDOW_HEIGHT - POCKET_INSET };

    bool dragging = false;
    Vector2 drag_start = Vector2Zero();

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();
        accumulator += delta_time;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            dragging = true;
            drag_start = GetMousePosition();
        }

        if (dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            Vector2 current = GetMousePosition();
            Vector2 drag_vector = Vector2Subtract(drag_start, current);
            float magnitude = Vector2Length(drag_vector);

            // caps the force
            if (magnitude > MAX_FORCE) {
                magnitude = MAX_FORCE;
            }

            Vector2 direction = Vector2Normalize(drag_vector);
            Vector2 force = Vector2Scale(direction, magnitude * balls[0].inverse_mass);

            balls[0].velocity = Vector2Add(balls[0].velocity, force);

            dragging = false;
        }

        while (accumulator >= TIMESTEP) {
            for (int i = 0; i < num_balls; i++) {
                if (!balls[i].active) continue;

                // friction so ball slows down
                balls[i].velocity = Vector2Subtract(balls[i].velocity,
                    Vector2Scale(balls[i].velocity, FRICTION * TIMESTEP));

                // stop after moving slow for awhile
                if (Vector2Length(balls[i].velocity) < VELOCITY_THRESHOLD) {
                    balls[i].velocity = Vector2Zero();
                }

                balls[i].position = Vector2Add(balls[i].position,
                    Vector2Scale(balls[i].velocity, TIMESTEP));
            }

            for (int i = 0; i < num_balls; i++) {
                for (int j = i + 1; j < num_balls; j++) {
                    BalltoBallCollision(balls[i], balls[j]);
                }
            }

            for (int i = 0; i < num_balls; i++) {
                for (int w = 0; w < num_walls; w++) {
                    BalltoWallCollision(balls[i], walls[w]);
                }
            }

            for (int i = 0; i < num_balls; i++) {
                for (int p = 0; p < num_pockets; p++) {
                    bool is_cue_ball = (i == 0);
                    BalltoPocketCollision(balls[i], pockets[p], cue_start_position, is_cue_ball);
                }
            }

            accumulator -= TIMESTEP;
        }

        BeginDrawing();
        ClearBackground(GREEN);

        // walls
        DrawRectangle(0, 0, WINDOW_WIDTH, BORDER_THICKNESS, RED);
        DrawRectangle(0, WINDOW_HEIGHT - BORDER_THICKNESS, WINDOW_WIDTH, BORDER_THICKNESS, RED);
        DrawRectangle(0, 0, BORDER_THICKNESS, WINDOW_HEIGHT, RED);
        DrawRectangle(WINDOW_WIDTH - BORDER_THICKNESS, 0, BORDER_THICKNESS, WINDOW_HEIGHT, RED);

        // pockets
        DrawCircleV({ POCKET_INSET, POCKET_INSET }, POCKET_RADIUS, BLACK);
        DrawCircleV({ WINDOW_WIDTH - POCKET_INSET, POCKET_INSET }, POCKET_RADIUS, BLACK);
        DrawCircleV({ POCKET_INSET, WINDOW_HEIGHT - POCKET_INSET }, POCKET_RADIUS, BLACK);
        DrawCircleV({ WINDOW_WIDTH - POCKET_INSET, WINDOW_HEIGHT - POCKET_INSET }, POCKET_RADIUS, BLACK);

        for (int i = 0; i < num_balls; i++) {
            if (balls[i].active) {
                DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
            }
        }

        if (dragging) {
            Vector2 current = GetMousePosition();
            Vector2 drag_vector = Vector2Subtract(current, balls[0].position);
            float drag_length = Vector2Length(drag_vector);

            if (drag_length > MAX_DRAG_LENGTH) {
                drag_vector = Vector2Scale(Vector2Normalize(drag_vector), MAX_DRAG_LENGTH);
            }

            Vector2 line_end = Vector2Add(balls[0].position, drag_vector);
            DrawLineEx(balls[0].position, line_end, 4.0f, YELLOW);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
