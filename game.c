// lane_dodger_v3.c
// Improved Lane Dodger for Windows console
// Features: ascii car, multiple lanes, multiple obstacles, difficulty, score, lives, highscore file

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define LANE_COUNT 5           // number of lanes (changeable)
#define SCREEN_ROWS 14         // drawing rows above player
#define MAX_OBSTACLES 12
#define PLAYER_HEIGHT 3        // ascii car height lines
#define PLAYER_WIDTH 7         // ascii car width columns
#define HIGH_SCORE_FILE "highscore.txt"

typedef struct {
    int lane;    // 0..LANE_COUNT-1
    int row;     // 0..SCREEN_ROWS (0 top)
    int type;    // type of obstacle (for future use)
    int active;  // 0 or 1
} Obstacle;

int laneX[LANE_COUNT];  // column X center for each lane

// Player representation (3 lines)
const char *player_sprite[PLAYER_HEIGHT] = {
    "  /-\\  ",
    " [o=o] ",
    "  \\_/  "
};

// Obstacle sprites (single-line for simplicity)
const char *obst_sprite = "[#]"; // you can make an array of types later

// Global game state
Obstacle obstacles[MAX_OBSTACLES];
int playerLane;
int lives;
int score;
int level;
int speed_ms;           // sleep time in ms (lower => faster)
int spawn_chance;       // 0..100 chance to spawn each tick per free obstacle slot
int highscore = 0;
int quit_to_menu = 0;

// small helper to set console cursor to top-left before drawing (avoid flicker)
void clearScreenFast() {
    // Use system("cls") is simple but flickers; we can use FillConsoleOutputCharacter for a bit nicer clear
    // But for reliability we will use system("cls") here (works fine)
    system("cls");
}

void loadHighscore() {
    FILE *f = fopen(HIGH_SCORE_FILE, "r");
    if (f) {
        fscanf(f, "%d", &highscore);
        fclose(f);
    } else {
        highscore = 0;
    }
}

void saveHighscore() {
    if (score > highscore) {
        FILE *f = fopen(HIGH_SCORE_FILE, "w");
        if (f) {
            fprintf(f, "%d\n", score);
            fclose(f);
        }
    }
}

void initObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        obstacles[i].active = 0;
    }
}

// spawn obstacle in random free slot
void spawnObstacle() {
    // find free slot
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        if (!obstacles[i].active) {
            obstacles[i].active = 1;
            obstacles[i].lane = rand() % LANE_COUNT;
            obstacles[i].row = 0;
            obstacles[i].type = 0;
            break;
        }
    }
}

// spawn pattern: can spawn more than one per tick depending on spawn_chance
void maybeSpawn() {
    // for each free slot, roll spawn
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        if (!obstacles[i].active) {
            int r = rand() % 100;
            if (r < spawn_chance) {
                obstacles[i].active = 1;
                obstacles[i].lane = rand() % LANE_COUNT;
                obstacles[i].row = 0;
                obstacles[i].type = 0;
            }
        }
    }
}

// draw entire screen
void drawScreen() {
    clearScreenFast();

    // HUD
    printf("Lane Dodger v3    Score: %d    Lives: %d    Level: %d    Highscore: %d\n",
           score, lives, level, highscore);
    printf("Controls: LEFT/RIGHT arrows  P=Pause  Q=Quit to menu\n");
    printf("+---------------------------------------------+\n");

    // Prepare a buffer of SCREEN_ROWS + PLAYER_HEIGHT lines to render characters at lane positions
    // Each line width will be enough to cover all lanes; we'll render as simple text with fixed lane column positions
    int total_lines = SCREEN_ROWS + PLAYER_HEIGHT;
    for (int r = 0; r < total_lines; ++r) {
        // We'll build line by placing ' ' and lane separators
        char line[200];
        memset(line, ' ', sizeof(line));
        line[0] = '|';
        int width = laneX[LANE_COUNT - 1] + 5;
        for (int c = 1; c < width; ++c) line[c] = ' ';
        line[width] = '|';
        line[width + 1] = '\0';

        // place obstacle(s) if their row matches this r
        for (int i = 0; i < MAX_OBSTACLES; ++i) {
            if (obstacles[i].active && obstacles[i].row == r) {
                int lx = laneX[obstacles[i].lane];
                const char *s = obst_sprite;
                int slen = (int)strlen(s);
                int start = lx - slen/2;
                for (int p = 0; p < slen; ++p) {
                    if (start + p > 0 && start + p < width) line[start + p] = s[p];
                }
            }
        }

        // place player sprite if r is within bottom PLAYER_HEIGHT lines
        int playerTopRow = SCREEN_ROWS; // player occupies rows SCREEN_ROWS .. SCREEN_ROWS+PLAYER_HEIGHT-1
        if (r >= playerTopRow && r < playerTopRow + PLAYER_HEIGHT) {
            int spriteLine = r - playerTopRow;
            int lx = laneX[playerLane];
            const char *ps = player_sprite[spriteLine];
            int slen = (int)strlen(ps);
            int start = lx - slen/2;
            for (int p = 0; p < slen; ++p) {
                if (start + p > 0 && start + p < width) line[start + p] = ps[p];
            }
        }

        // print the line
        printf("%s\n", line);
    }

    printf("+---------------------------------------------+\n");
}

// move obstacles down and check collision; return 1 if collision happened this tick
int stepObstacles() {
    int collided = 0;
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        if (!obstacles[i].active) continue;
        obstacles[i].row += 1;

        // if obstacle reached player's top row + some overlap -> check collision
        // player's top row = SCREEN_ROWS
        // If obstacle row >= SCREEN_ROWS and obstacle lane == playerLane => collision
        if (obstacles[i].row >= SCREEN_ROWS) {
            if (obstacles[i].lane == playerLane) {
                collided = 1;
                obstacles[i].active = 0; // remove obstacle after collision
            } else {
                // passed safely, award score
                score++;
                obstacles[i].active = 0;
                // level up every 7 points
                if (score % 7 == 0) {
                    level++;
                    if (speed_ms > 40) speed_ms -= 10; // speed up
                    if (spawn_chance < 70) spawn_chance += 6; // spawn more often
                }
            }
        }

        // safety: deactivate if out of screen
        if (obstacles[i].row > SCREEN_ROWS + PLAYER_HEIGHT + 2) {
            obstacles[i].active = 0;
        }
    }
    return collided;
}

// show small pause screen
void doPause() {
    printf("\n-- PAUSED (press P again to resume) --\n");
    while (1) {
        if (_kbhit()) {
            int c = getch();
            if (c == 'p' || c == 'P') break;
            if (c == 'q' || c == 'Q') { quit_to_menu = 1; break; }
        }
        Sleep(100);
    }
}

// menu for difficulty + start
int showMainGameMenu() {
    while (1) {
        system("cls");
        printf("====================================\n");
        printf("         LANE DODGER v3\n");
        printf("====================================\n\n");
        printf("1) Easy   - relaxed spawn, slow\n");
        printf("2) Medium - normal\n");
        printf("3) Hard   - faster spawn and speed\n");
        printf("4) Instructions\n");
        printf("5) Quit\n\n");
        printf("Choose (1-5): ");

        int ch = getch();
        if (ch == '1') return 1;
        if (ch == '2') return 2;
        if (ch == '3') return 3;
        if (ch == '4') {
            system("cls");
            printf("Instructions:\n");
            printf("- Move with LEFT and RIGHT arrow keys.\n");
            printf("- Avoid obstacles marked by %s.\n", obst_sprite);
            printf("- You have 3 lives. Each collision loses 1 life.\n");
            printf("- Score increases when you dodge obstacles.\n");
            printf("- Levels increase every 7 points; game speeds up.\n");
            printf("- Press Q to quit to menu anytime.\n");
            printf("\nPress any key to return...");
            getch();
        }
        if (ch == '5') return 0;
    }
}

// Start actual gameplay loop
void runGameWithDifficulty(int diff) {
    // initialize
    playerLane = LANE_COUNT / 2; // center
    lives = 3;
    score = 0;
    level = 1;
    quit_to_menu = 0;

    // difficulty presets
    if (diff == 1) { // Easy
        speed_ms = 150;
        spawn_chance = 18;
    } else if (diff == 2) { // Medium
        speed_ms = 120;
        spawn_chance = 26;
    } else { // Hard
        speed_ms = 90;
        spawn_chance = 34;
    }

    initObstacles();
    loadHighscore();

    // game loop
    int tick = 0;
    while (1) {
        // input
        if (_kbhit()) {
            int c = getch();
            if (c == 224) { // arrow keys prefix
                c = getch();
                if (c == 75) { // left
                    if (playerLane > 0) playerLane--;
                } else if (c == 77) { // right
                    if (playerLane < LANE_COUNT - 1) playerLane++;
                }
            } else {
                if (c == 'p' || c == 'P') {
                    doPause();
                    if (quit_to_menu) break;
                } else if (c == 'q' || c == 'Q') {
                    quit_to_menu = 1;
                    break;
                }
            }
        }

        // spawn sometimes
        if (tick % 1 == 0) { // each loop
            maybeSpawn();
        }

        // draw
        drawScreen();

        // advance obstacles
        int collided = stepObstacles();
        if (collided) {
            // sound + lives--
            Beep(600, 120);
            lives--;
            if (lives <= 0) {
                clearScreenFast();
                printf("\n\n===== GAME OVER =====\n");
                printf("Final Score: %d\n", score);
                if (score > highscore) {
                    printf("New Highscore! %d (saved)\n", score);
                } else {
                    printf("Highscore: %d\n", highscore);
                }
                saveHighscore();
                printf("Press 'R' to restart game, or any other key to go to main menu.\n");
                int c = getch();
                if (c == 'r' || c == 'R') {
                    // restart at same difficulty
                    runGameWithDifficulty(diff);
                    return;
                } else {
                    quit_to_menu = 1;
                    break;
                }
            }
        }

        // small sleep based on speed_ms
        Sleep(speed_ms);
        ++tick;

        if (quit_to_menu) break;
    }
}

// main
int main(void) {
    srand((unsigned int)time(NULL));

    // compute lane X positions (center columns) - pick width = 6 per lane approx
    // We'll place lanes across the printable width; choose columns manually for simple display
    // Make these fixed so sprites align nicely.
    int startX = 6;
    int spacing = 8; // spacing between lane centers
    for (int i = 0; i < LANE_COUNT; ++i) laneX[i] = startX + i * spacing;

    loadHighscore();

    while (1) {
        int sel = showMainGameMenu();
        if (sel >= 1 && sel <= 3) {
            runGameWithDifficulty(sel);
        } else if (sel == 0) {
            printf("\nGoodbye!\n");
            return 0;
        }
    }

    return 0;
}
