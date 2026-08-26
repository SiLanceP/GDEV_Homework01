#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>
#include <string>

//https://en.cppreference.com/w/cpp/language/operator_arithmetic

struct Position {
    int x;
    int y;
};

struct Enemy {
    Position pos;
    int speed;
    bool alive;
};

struct GameState {
    int width;
    int height;
    Position player;
    std::vector<Enemy> enemies;
};

// helper func that wraps a coordinate around the grid, Karnaugh-map style
int Wrap(int value, int dimension) {
    return (value + dimension) % dimension;
}

// reads settings.txt
bool LoadSettings(const std::string& filename, GameState& state) {
    std::ifstream file(filename);

    if (!file) {
        std::cout << "Could not open " << filename << "\n";
        return false;
    }

    state.enemies.resize(2);
    for (int i = 0; i < 2; i++) {
        state.enemies[i].speed = 0;
        state.enemies[i].alive = true;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t equalsPos = line.find('=');

        if (equalsPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, equalsPos);
        std::string value = line.substr(equalsPos + 1);
        int val = std::stoi(value);

        if (key == "grid_w") {
            state.width = val;
        } else if (key == "grid_h") {
            state.height = val;
        } else if (key == "player_x") {
            state.player.x = val;
        } else if (key == "player_y") {
            state.player.y = val;
        } else if (key == "enemy1_x") {
            state.enemies[0].pos.x = val;
        } else if (key == "enemy1_y") {
            state.enemies[0].pos.y = val;
        } else if (key == "enemy1_speed") {
            state.enemies[0].speed = val;
        } else if (key == "enemy2_x") {
            state.enemies[1].pos.x = val;
        } else if (key == "enemy2_y") {
            state.enemies[1].pos.y = val;
        } else if (key == "enemy2_speed") {
            state.enemies[1].speed = val;
        }
    }

    state.player.x = Wrap(state.player.x, state.width);
    state.player.y = Wrap(state.player.y, state.height);
    for (int i = 0; i < (int)state.enemies.size(); i++) {
        state.enemies[i].pos.x = Wrap(state.enemies[i].pos.x, state.width);
        state.enemies[i].pos.y = Wrap(state.enemies[i].pos.y, state.height);
    }

    return true;
}

// prints the border
void PrintBorder(int width) {
    for (int x = 0; x < width; x++) {
        std::cout << "+---";
    }
    std::cout << "+\n";
}

// prints the grid thats boxed in like a table
void DisplayGrid(const GameState& state) {
    char* grid = new char[state.width * state.height];

    for (int i = 0; i < state.width * state.height; i++) {
        grid[i] = '.';
    }

    for (int i = 0; i < (int)state.enemies.size(); i++) {
        if (state.enemies[i].alive) {
            int index = state.enemies[i].pos.y * state.width + state.enemies[i].pos.x;
            grid[index] = 'E';
        }
    }

    grid[state.player.y * state.width + state.player.x] = 'P';

    for (int y = 0; y < state.height; y++) {
        PrintBorder(state.width);
        for (int x = 0; x < state.width; x++) {
            std::cout << "| " << grid[y * state.width + x] << " ";
        }
        std::cout << "|\n";
    }
    PrintBorder(state.width);

    delete[] grid;
}

// moves the player by (dx, dy) while also wrapping around the grid edges
void MovePlayer(GameState& state, int dx, int dy) {
    state.player.x = Wrap(state.player.x + dx, state.width);
    state.player.y = Wrap(state.player.y + dy, state.height);
}

// checks every living enemy against the player's position
bool AttackEnemies(GameState& state) {
    bool hitSomething = false;

    for (int i = 0; i < (int)state.enemies.size(); i++) {
        if (state.enemies[i].alive &&
            state.enemies[i].pos.x == state.player.x &&
            state.enemies[i].pos.y == state.player.y) {
            state.enemies[i].alive = false;
            hitSomething = true;
        }
    }

    return hitSomething;
}

bool AllEnemiesDefeated(const GameState& state) {
    for (int i = 0; i < (int)state.enemies.size(); i++) {
        if (state.enemies[i].alive) {
            return false;
        }
    }
    return true;
}


int main() {
    GameState state;

    if (!LoadSettings("settings.txt", state)) {
        return 0;
    }

    std::cout << "Find the enemies (E) and attack them!\n";
    std::cout << "Commands: north/n, south/s, east/e, west/w, attack/a, exit\n\n";

    while (true) {
        DisplayGrid(state);

        std::cout << "\n> ";
        std::string input;
        std::getline(std::cin, input);

        if (!std::cin) {
            break;
        }

        if (input == "north" || input == "n") {
            MovePlayer(state, 0, -1);
        } else if (input == "south" || input == "s") {
            MovePlayer(state, 0, 1);
        } else if (input == "east" || input == "e") {
            MovePlayer(state, 1, 0);
        } else if (input == "west" || input == "w") {
            MovePlayer(state, -1, 0);
        } else if (input == "attack" || input == "a") {
            if (AttackEnemies(state)) {
                std::cout << "\nYou attacked an enemy!\n";
                if (AllEnemiesDefeated(state)) {
                    DisplayGrid(state);
                    std::cout << "\nAll enemies defeated. You win!\n";
                    break;
                }
            } else {
                std::cout << "\nNothing to attack here.\n";
            }
        } else if (input == "exit") {
            std::cout << "\nExiting game.\n";
            break;
        }

        std::cout << "\n";
    }

    return 0;
}