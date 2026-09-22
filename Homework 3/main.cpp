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

struct Ball {
    Vector2 position;
    float radius;
    Color color;
    float mass;
    float inverse_mass;
    Vector2 velocity;
};

struct Wall {
    Rectangle rect;
};
