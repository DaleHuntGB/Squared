#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>
#include <cmath>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TARGET_FPS 60
#define NUM_ENEMIES 5
#define ENEMY_SPAWN_RADIUS 700

class Projectile
{
private:
    Vector2 projectilePosition;
    Vector2 projectileDirection;
    int projectileSpeed;

public:
    Projectile(Vector2 startPosition, Vector2 direction, int speed) : projectilePosition(startPosition), projectileDirection(direction), projectileSpeed(speed)
    {
        float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
        projectileDirection.x /= magnitude;
        projectileDirection.y /= magnitude;
    }

    void MoveProjectile()
    {
        projectilePosition.x += projectileDirection.x * projectileSpeed;
        projectilePosition.y += projectileDirection.y * projectileSpeed;
    }

    void DrawProjectile()
    {
        DrawCircle(projectilePosition.x, projectilePosition.y, 10, BLACK);
    }

    Vector2 getPosition() { return projectilePosition; }
};

class Player
{
private:
    std::string playerName;
    int playerHealth;
    int playerSpeed;
    int playerScore = 0;
    Vector2 playerPosition;
    float burstCD = 0.0f;
    float cooldownSpeed = 1.0f;

    int moveUp;
    int moveDown;
    int moveLeft;
    int moveRight;
    int shoot;

    std::vector<Projectile> &projectiles;   

public:
    Player(const std::string &playerName, int playerHealth, int playerSpeed, Vector2 playerPosition, int moveUp, int moveDown, int moveLeft, int moveRight, int Shoot, std::vector<Projectile> &projectiles, int playerScore = 0) 
    : playerName(playerName), playerHealth(playerHealth), playerSpeed(playerSpeed), playerPosition(playerPosition), moveUp(moveUp), moveDown(moveDown), moveLeft(moveLeft), moveRight(moveRight), shoot(Shoot), projectiles(projectiles) {}

    void movePlayer()
    {
        if (IsKeyDown(moveUp))
        {
            playerPosition.y -= playerSpeed;
        }
        if (IsKeyDown(moveDown))
        {
            playerPosition.y += playerSpeed;
        }
        if (IsKeyDown(moveLeft))
        {
            playerPosition.x -= playerSpeed;
        }
        if (IsKeyDown(moveRight))
        {
            playerPosition.x += playerSpeed;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE))
        {
            Shoot();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            Burst();
        }
    }

    void DrawPlayer(Color playerColor)
    {
        DrawRectangle(playerPosition.x, playerPosition.y, 32, 32, playerColor);
        int textWidth = MeasureText(playerName.c_str(), 20);
        int textX = playerPosition.x + 20 - textWidth / 2;
        int textY = playerPosition.y - 20;
        DrawText(playerName.c_str(), textX, textY, 20, BLACK);
    }

    void Shoot()
    {
        Vector2 mousePosition = GetMousePosition();
        Vector2 direction = {mousePosition.x - playerPosition.x, mousePosition.y - playerPosition.y};
        Vector2 projectileStartPosition = {playerPosition.x + 25, playerPosition.y + 25};
        projectiles.push_back(Projectile(projectileStartPosition, direction, 10));
    }

    void Burst()
    {
        if (burstCD <= 0.0f)
        {
            for (int i = 0; i < 360; i += 10)
            {
                Vector2 direction = {cos(i * DEG2RAD), sin(i * DEG2RAD)};
                Vector2 projectileStartPosition = {playerPosition.x + 25, playerPosition.y + 25};
                projectiles.push_back(Projectile(projectileStartPosition, direction, 10));
            }
            burstCD = 30.0f;
        }
    }

    void UpdateBurstCD()
    {
        if (burstCD > 0.0f)
        {
            burstCD -= GetFrameTime();
            if (burstCD < 0.0f) burstCD = 0.0f;
        }
    }


    float getBurstCD() { return burstCD; }
    void setBurstCD(float value) { burstCD = value; }
    Vector2& getPlayerPosition() { return playerPosition; }
    std::string getPlayerName() { return playerName; }
    int updatePlayerScore(int amount) { return playerScore += amount; }
};

class Enemy
{
    private:
    int enemyHealth;
    int enemySpeed;
    Vector2 enemyPosition;
    public:
    Enemy(int enemyHealth, int enemySpeed, Vector2 enemyPosition) : enemyHealth(enemyHealth), enemySpeed(enemySpeed), enemyPosition(enemyPosition) {}
    Enemy(Vector2 enemyPosition, int enemyHealth) : enemyHealth(enemyHealth), enemySpeed(enemySpeed), enemyPosition(enemyPosition) {}

    void MoveEnemy(Vector2 playerPosition)
    {
        Vector2 direction = {playerPosition.x - enemyPosition.x, playerPosition.y - enemyPosition.y};
        float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
        direction.x /= magnitude;
        direction.y /= magnitude;
        enemyPosition.x += direction.x * enemySpeed;
        enemyPosition.y += direction.y * enemySpeed;
    }

    void DrawEnemy(Color enemyColor)
    {
        DrawRectangle(enemyPosition.x, enemyPosition.y, 32, 32, enemyColor);
        DrawRectangle(enemyPosition.x, enemyPosition.y - 10, 32, 5, RED);
        DrawRectangle(enemyPosition.x, enemyPosition.y - 10, 32 * (enemyHealth / 100.0f), 5, GREEN);
    }

    Vector2 getEnemyPosition() { return enemyPosition; }
    int getEnemyHealth() { return enemyHealth; }
    void decreaseHealth(int amount)
    {
        enemyHealth -= amount;
        if (enemyHealth < 0)
            enemyHealth = 0;
    }

    bool isDead() const { return enemyHealth <= 0; }
};

class CollisionManager
{
    public:
    static bool CheckCollision(Vector2 position1, Vector2 position2, float radius1, float radius2)
    {
        float distance = sqrt(pow(position2.x - position1.x, 2) + pow(position2.y - position1.y, 2));
        return distance < radius1 + radius2;
    }

    static void CheckBounds(Vector2 &position, int width, int height)
    {
        if (position.x < 0) position.x = width;
        if (position.x > width) position.x = 0;
        if (position.y < 0) position.y = height;
        if (position.y > height) position.y = 0;
    }
};

class GameManager
{
private:
    float waveTimer = 30.0f; // Timer to track wave spawning
    int currentWave = 0;

public:
    void SpawnEnemies(Player& player, std::vector<Enemy>& enemies, int NUM_OF_WAVES)
    {
        waveTimer += GetFrameTime();
        if (waveTimer >= 30.0f && currentWave < NUM_OF_WAVES)
        {
            for (int i = 0; i < NUM_ENEMIES; i++)
            {
                float angle = i * (360.0f / NUM_ENEMIES) * DEG2RAD;
                Vector2 spawnPosition = {
                    player.getPlayerPosition().x + cos(angle) * ENEMY_SPAWN_RADIUS,
                    player.getPlayerPosition().y + sin(angle) * ENEMY_SPAWN_RADIUS
                };
                enemies.emplace_back(100, 2, spawnPosition);
            }

            currentWave++;
            waveTimer = 0.0f;
            std::cout << "Wave " << currentWave << " of " << NUM_OF_WAVES << " has begun!" << std::endl;
        }
    }

    int getCurrentWave() { return currentWave; }
    float getWaveTimer() { return waveTimer; }
};

class PowerUpManager
{
private:
    struct PowerUp
    {
        int type;
        Vector2 position;
        bool active;
    };

    std::vector<PowerUp> activePowerUps;

public:
    void SpawnPowerUp(int powerUpType)
    {
        PowerUp newPowerUp = {powerUpType, {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)}, true};
        activePowerUps.push_back(newPowerUp);
    }

    void DrawPowerUps()
    {
        for (const auto& powerUp : activePowerUps)
        {
            if (!powerUp.active) continue;

            switch (powerUp.type)
            {
            case 0: // Health
                DrawCircle(powerUp.position.x, powerUp.position.y, 10, GREEN);
                std::cout << "Health PowerUp Spawned!" << std::endl;
                break;
            case 1: // Speed
                DrawCircle(powerUp.position.x, powerUp.position.y, 10, YELLOW);
                std::cout << "Speed PowerUp Spawned!" << std::endl;
                break;
            case 2: // Damage
                DrawCircle(powerUp.position.x, powerUp.position.y, 10, RED);
                std::cout << "Damage PowerUp Spawned!" << std::endl;
                break;
            case 3: // Burst
                DrawCircle(powerUp.position.x, powerUp.position.y, 10, BLUE);
                std::cout << "Burst PowerUp Spawned!" << std::endl;
                break;
            }
        }
    }

    void CheckPowerUpCollision(Player& player)
    {
        for (auto& powerUp : activePowerUps)
        {
            if (!powerUp.active) continue;

            if (CollisionManager::CheckCollision(player.getPlayerPosition(), powerUp.position, 16, 10))
            {
                powerUp.active = false;
                ApplyPowerUp(player, powerUp.type);
            }
        }
    }

    void ApplyPowerUp(Player& player, int powerUpType)
    {
        switch (powerUpType)
        {
        case 0: // Health
            std::cout << "Health PowerUp Collected!" << std::endl;
            break;
        case 1: // Speed
            std::cout << "Speed PowerUp Collected!" << std::endl;
            break;
        case 2: // Damage
            std::cout << "Damage PowerUp Collected!" << std::endl;
            break;
        case 3: // Burst
            std::cout << "Burst PowerUp Collected!" << std::endl;
            player.setBurstCD(0.0f);
            break;
        }
    }
};

int main()
{
    const char* windowTitle = "LoopLoop";
    std::vector<Projectile> projectiles;
    std::vector<Enemy> enemies;
    Player player("Unhalted", 100, 5, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, KEY_W, KEY_S, KEY_A, KEY_D, MOUSE_BUTTON_LEFT, projectiles);
    GameManager gameManager;
    PowerUpManager powerUpManager;
    CollisionManager collisionManager;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowTitle);
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);

        // Player actions
        player.movePlayer();
        player.UpdateBurstCD();
        player.DrawPlayer(BLACK);

        // Manage waves and enemies
        gameManager.SpawnEnemies(player, enemies, 5);
        for (Enemy& enemy : enemies)
        {
            enemy.MoveEnemy(player.getPlayerPosition());
            enemy.DrawEnemy(RED);
        }

        // Handle projectiles
        for (size_t i = 0; i < projectiles.size(); i++)
        {
            projectiles[i].MoveProjectile();
            projectiles[i].DrawProjectile();
        }

        // Handle Collisions
        for (size_t i = 0; i < projectiles.size(); i++)
        {
            for (size_t j = 0; j < enemies.size(); j++)
            {
                if (collisionManager.CheckCollision(projectiles[i].getPosition(), enemies[j].getEnemyPosition(), 10, 16))
                {
                    enemies[j].decreaseHealth(10);
                    projectiles.erase(projectiles.begin() + i);
                    if (enemies[j].isDead())
                    {
                        enemies.erase(enemies.begin() + j);
                        player.updatePlayerScore(10);
                        powerUpManager.SpawnPowerUp(GetRandomValue(0, 3));
                    }
                }
            }
        }

        // Handle power-ups
        powerUpManager.DrawPowerUps();
        powerUpManager.CheckPowerUpCollision(player);

        // Display HUD
        DrawText(("Score: " + std::to_string(player.updatePlayerScore(0))).c_str(), 10, 10, 20, BLACK);
        DrawText(("Wave: " + std::to_string(gameManager.getCurrentWave())).c_str(), 10, 40, 20, BLACK);
        DrawText(("Wave Timer: " + std::to_string(static_cast<int>(ceil(gameManager.getWaveTimer())))).c_str(), 10, 70, 20, BLACK);
        if (player.getBurstCD() > 0.0f) { DrawText(("Burst CD: " + std::to_string(static_cast<int>(ceil(player.getBurstCD())))).c_str(), 10, 100, 20, RED); }
        else { DrawText("BurstCD: Ready!", 10, 100, 20, GREEN); }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
