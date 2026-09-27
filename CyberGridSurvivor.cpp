#include <SDL.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

struct Bullet {
    float x, y, vx, vy;
    bool active = true;
};

struct Enemy {
    float x, y, speed;
    int size;
    bool active = true;
};

struct Particle {
    float x, y, vx, vy;
    int life;
    Uint8 r, g, b;
};

// collision for bullets
bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y);
}

void spawnParticles(std::vector<Particle>& particles, float x, float y, Uint8 r, Uint8 g, Uint8 b, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 50) / 10.0f + 1.0f;
        particles.push_back({ x, y, cosf(angle) * speed, sinf(angle) * speed, 15 + (rand() % 15), r, g, b });
    }
}

// Procedurally draws a digit (0-9) using 7-segment style rectangles
void renderDigit(SDL_Renderer* renderer, int digit, int x, int y, int size, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    int t = size / 5; // segment thickness
    int w = size;     // width
    int h = size * 2; // height

    // Segment definitions: top, top-left, top-right, middle, bottom-left, bottom-right, bottom
    bool seg[10][7] = {
        {1, 1, 1, 0, 1, 1, 1}, // 0
        {0, 0, 1, 0, 0, 1, 0}, // 1
        {1, 0, 1, 1, 1, 0, 1}, // 2
        {1, 0, 1, 1, 0, 1, 1}, // 3
        {0, 1, 1, 1, 0, 1, 0}, // 4
        {1, 1, 0, 1, 0, 1, 1}, // 5
        {1, 1, 0, 1, 1, 1, 1}, // 6
        {1, 0, 1, 0, 0, 1, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}  // 9
    };

    if (digit < 0 || digit > 9) return;

    SDL_Rect rects[7] = {
        { x, y, w, t },                     // Top
        { x, y, t, h / 2 },                 // Top-Left
        { x + w - t, y, t, h / 2 },         // Top-Right
        { x, y + h / 2 - t / 2, w, t },     // Middle
        { x, y + h / 2, t, h / 2 },         // Bottom-Left
        { x + w - t, y + h / 2, t, h / 2 }, // Bottom-Right
        { x, y + h - t, w, t }              // Bottom
    };

    for (int i = 0; i < 7; ++i) {
        if (seg[digit][i]) {
            SDL_RenderFillRect(renderer, &rects[i]);
        }
    }
}

// Renders an entire integer number on screen
void renderNumber(SDL_Renderer* renderer, int number, int x, int y, int size, Uint8 r, Uint8 g, Uint8 b) {
    std::string s = std::to_string(number);
    int curX = x;
    for (char c : s) {
        renderDigit(renderer, c - '0', curX, y, size, r, g, b);
        curX += size + (size / 2); // spacing between digits
    }
}

int main(int argc, char* args[]) {
    srand(static_cast<unsigned int>(time(NULL)));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Init Failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Cyber Grid Survivor | Score: 0 | WASD: Move | Arrows: Shoot | Space: EMP",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    // Hardware accelerated GPU Renderer with VSync
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Game State Variables
    float playerX = SCREEN_WIDTH / 2.0f;
    float playerY = SCREEN_HEIGHT / 2.0f;
    const int playerSize = 24;
    const float playerSpeed = 5.0f;
    int playerHP = 100;
    int score = 0;
    int highScore = 0;
    int empCharge = 0;
    int shootCooldown = 0;
    int spawnTimer = 0;
    int screenShake = 0;
    bool gameOver = false;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Particle> particles;

    bool quit = false;
    SDL_Event e;

    // game loop

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();

        // event handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) quit = true;

                // Restart Game on R
                if (gameOver && e.key.keysym.sym == SDLK_r) {
                    playerX = SCREEN_WIDTH / 2.0f;
                    playerY = SCREEN_HEIGHT / 2.0f;
                    playerHP = 100;
                    score = 0;
                    empCharge = 0;
                    bullets.clear();
                    enemies.clear();
                    particles.clear();
                    gameOver = false;
                }

                // Blast on Spacebar 
                if (!gameOver && e.key.keysym.sym == SDLK_SPACE && empCharge >= 100) {
                    empCharge = 0;
                    screenShake = 15;
                    for (auto& en : enemies) {
                        if (en.active) {
                            en.active = false;
                            score += 10;
                            spawnParticles(particles, en.x, en.y, 255, 50, 100, 12);
                        }
                    }
                }
            }
        }

        // continous keyboard state for movement and shooting
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (!gameOver) {
            if (keys[SDL_SCANCODE_W] && playerY > 50) playerY -= playerSpeed;
            if (keys[SDL_SCANCODE_S] && playerY < SCREEN_HEIGHT - playerSize) playerY += playerSpeed;
            if (keys[SDL_SCANCODE_A] && playerX > 0) playerX -= playerSpeed;
            if (keys[SDL_SCANCODE_D] && playerX < SCREEN_WIDTH - playerSize) playerX += playerSpeed;

            if (shootCooldown > 0) shootCooldown--;
            float bvx = 0.0f, bvy = 0.0f;
            if (keys[SDL_SCANCODE_UP]) bvy = -10.0f;
            if (keys[SDL_SCANCODE_DOWN]) bvy = 10.0f;
            if (keys[SDL_SCANCODE_LEFT]) bvx = -10.0f;
            if (keys[SDL_SCANCODE_RIGHT]) bvx = 10.0f;

            if ((bvx != 0.0f || bvy != 0.0f) && shootCooldown == 0) {
                bullets.push_back({ playerX + playerSize / 2.0f - 4, playerY + playerSize / 2.0f - 4, bvx, bvy, true });
                shootCooldown = 8; // Fire rate
            }

            // enemy spawner, spawn rate decreases as score increases
            spawnTimer++;
            int spawnRate = std::max(15, 55 - (score / 50));
            if (spawnTimer >= spawnRate) {
                spawnTimer = 0;
                float ex, ey;
                int edge = rand() % 4;
                if (edge == 0) { ex = rand() % SCREEN_WIDTH; ey = 50.0f; }
                else if (edge == 1) { ex = rand() % SCREEN_WIDTH; ey = (float)SCREEN_HEIGHT; }
                else if (edge == 2) { ex = 0.0f; ey = 50.0f + rand() % (SCREEN_HEIGHT - 50); }
                else { ex = (float)SCREEN_WIDTH; ey = 50.0f + rand() % (SCREEN_HEIGHT - 50); }

                float spd = 1.5f + (rand() % 20) / 10.0f + (score / 500.0f);
                enemies.push_back({ ex, ey, spd, 20, true });
            }

            // UPDATE ENTITIES & COLLISIONS
            for (auto& b : bullets) {
                if (!b.active) continue;
                b.x += b.vx;
                b.y += b.vy;
                if (b.x < 0 || b.x > SCREEN_WIDTH || b.y < 50 || b.y > SCREEN_HEIGHT) b.active = false;
            }

            SDL_Rect playerRect = { (int)playerX, (int)playerY, playerSize, playerSize };

            for (auto& en : enemies) {
                if (!en.active) continue;

                // Vector math to chase player
                float dx = playerX - en.x;
                float dy = playerY - en.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 0) {
                    en.x += (dx / dist) * en.speed;
                    en.y += (dy / dist) * en.speed;
                }

                SDL_Rect enemyRect = { (int)en.x, (int)en.y, en.size, en.size };

                // bullet collision with enemy
                for (auto& b : bullets) {
                    if (!b.active) continue;
                    SDL_Rect bulletRect = { (int)b.x, (int)b.y, 8, 8 };
                    if (checkCollision(bulletRect, enemyRect)) {
                        b.active = false;
                        en.active = false;
                        score += 10;
                        if (score > highScore) highScore = score;
                        if (empCharge < 100) empCharge += 5;
                        spawnParticles(particles, en.x, en.y, 255, 60, 120, 8);

                        std::string title = "Cyber Grid Survivor | Score: " + std::to_string(score) +
                            " | Best: " + std::to_string(highScore) +
                            " | [WASD] Move [Arrows] Shoot [Space] EMP";
                        SDL_SetWindowTitle(window, title.c_str());
                        break;
                    }
                }

                // enemy collision with player
                if (en.active && checkCollision(playerRect, enemyRect)) {
                    en.active = false;
                    playerHP -= 20;
                    screenShake = 10;
                    spawnParticles(particles, playerX, playerY, 0, 255, 255, 15);
                    if (playerHP <= 0) {
                        playerHP = 0;
                        gameOver = true;
                        SDL_SetWindowTitle(window, "GAME OVER! Press [R] to Restart | Cyber Grid Survivor");
                    }
                }
            }
        }

        // Update particles
        for (auto& p : particles) {
            p.x += p.vx;
            p.y += p.vy;
            p.life--;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0; }), particles.end());

        // HARDWARE RENDERING
        int shakeX = (screenShake > 0) ? (rand() % 8 - 4) : 0;
        int shakeY = (screenShake > 0) ? (rand() % 8 - 4) : 0;
        if (screenShake > 0) screenShake--;

        // Background color
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        // Background Grid
        SDL_SetRenderDrawColor(renderer, 28, 30, 48, 255);
        for (int x = 0; x < SCREEN_WIDTH; x += 40) SDL_RenderDrawLine(renderer, x + shakeX, 50, x + shakeX, SCREEN_HEIGHT);
        for (int y = 50; y < SCREEN_HEIGHT; y += 40) SDL_RenderDrawLine(renderer, 0, y + shakeY, SCREEN_WIDTH, y + shakeY);

        // Particles
        for (const auto& p : particles) {
            SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, 255);
            SDL_Rect pRect = { (int)p.x + shakeX, (int)p.y + shakeY, 4, 4 };
            SDL_RenderFillRect(renderer, &pRect);
        }

        // Draw Player (Cyan)
        if (!gameOver) {
            SDL_SetRenderDrawColor(renderer, 0, 240, 255, 255);
            SDL_Rect pRect = { (int)playerX + shakeX, (int)playerY + shakeY, playerSize, playerSize };
            SDL_RenderFillRect(renderer, &pRect);
        }

        //  Bullets
        SDL_SetRenderDrawColor(renderer, 255, 230, 0, 255);
        for (const auto& b : bullets) {
            if (b.active) {
                SDL_Rect bRect = { (int)b.x + shakeX, (int)b.y + shakeY, 8, 8 };
                SDL_RenderFillRect(renderer, &bRect);
            }
        }

        // Enemies
        SDL_SetRenderDrawColor(renderer, 255, 45, 95, 255);
        for (const auto& en : enemies) {
            if (en.active) {
                SDL_Rect eRect = { (int)en.x + shakeX, (int)en.y + shakeY, en.size, en.size };
                SDL_RenderFillRect(renderer, &eRect);
            }
        }

        // Draw Top HUD Banner
        SDL_SetRenderDrawColor(renderer, 25, 28, 40, 255);
        SDL_Rect hudBg = { 0, 0, SCREEN_WIDTH, 50 };
        SDL_RenderFillRect(renderer, &hudBg);

        // Health Bar Green to red
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_Rect hpBack = { 20, 15, 160, 20 };
        SDL_RenderFillRect(renderer, &hpBack);

        SDL_SetRenderDrawColor(renderer, (100 - playerHP) * 2, playerHP * 2, 80, 255);
        SDL_Rect hpBar = { 20, 15, (int)(playerHP * 1.6f), 20 };
        SDL_RenderFillRect(renderer, &hpBar);

        // Energy Bar (Cyan/White flash when ready)
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_Rect empBack = { 200, 15, 160, 20 };
        SDL_RenderFillRect(renderer, &empBack);

        if (empCharge >= 100 && (SDL_GetTicks() / 150) % 2 == 0)
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        else
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, 255);
        SDL_Rect empBar = { 200, 15, (int)(empCharge * 1.6f), 20 };
        SDL_RenderFillRect(renderer, &empBar);

        // Score Progress Tier Bar 
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_Rect scoreProgressBack = { 380, 15, 160, 20 };
        SDL_RenderFillRect(renderer, &scoreProgressBack);

        int tierProgress = (score % 500); // Progress bar
        int scoreBarFillWidth = (int)((tierProgress / 500.0f) * 160);
        SDL_SetRenderDrawColor(renderer, 255, 190, 0, 255); // Red / Yellow
        SDL_Rect scoreProgressBar = { 380, 15, scoreBarFillWidth, 20 };
        SDL_RenderFillRect(renderer, &scoreProgressBar);

        // Digital Score
        renderNumber(renderer, score, 660, 15, 10, 255, 215, 0);

        // Game Over Overlay
        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 180, 20, 40, 255);
            SDL_Rect goBanner = { SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 40, 300, 80 };
            SDL_RenderFillRect(renderer, &goBanner);
        }

        SDL_RenderPresent(renderer);

        // Caps at 60 FPS 
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16) SDL_Delay(16 - frameTime);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}