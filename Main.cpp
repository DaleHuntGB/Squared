#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TARGET_FPS 60
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

    Vector2 getPosition() const { return projectilePosition; }
};

class Player
{
private:
    int playerHealth;
    int playerSpeed;
    int playerScore = 0;
    int playerDamage = 10;
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
    Player(int playerHealth, int playerSpeed, Vector2 playerPosition, int moveUp, int moveDown, int moveLeft, int moveRight, int Shoot, std::vector<Projectile> &projectiles, int playerScore = 0, int playerDamage = 10) 
    : playerHealth(playerHealth), playerSpeed(playerSpeed), playerPosition(playerPosition), moveUp(moveUp), moveDown(moveDown), moveLeft(moveLeft), moveRight(moveRight), shoot(Shoot), projectiles(projectiles), playerDamage(playerDamage) {}

    void movePlayer()
    {
        Vector2 direction = {0.0f, 0.0f};

        if (IsKeyDown(moveUp)) direction.y -= 1;
        if (IsKeyDown(moveDown)) direction.y += 1;
        if (IsKeyDown(moveLeft)) direction.x -= 1;
        if (IsKeyDown(moveRight)) direction.x += 1;

        float magnitude = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (magnitude > 0.0f)
        {
            direction.x /= magnitude;
            direction.y /= magnitude;
        }

        playerPosition.x += direction.x * playerSpeed;
        playerPosition.y += direction.y * playerSpeed;

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
        DrawCircle(playerPosition.x, playerPosition.y, 16, BLACK);
    }

    void Shoot()
    {
        Vector2 mousePosition = GetMousePosition();
        Vector2 direction = {mousePosition.x - playerPosition.x, mousePosition.y - playerPosition.y};
        Vector2 projectileStartPosition = {playerPosition.x, playerPosition.y};
        projectiles.push_back(Projectile(projectileStartPosition, direction, 10));
    }

    void Burst()
    {
        if (burstCD <= 0.0f)
        {
            for (int i = 0; i < 360; i += 10)
            {
                Vector2 direction = {cos(i * DEG2RAD), sin(i * DEG2RAD)};
                Vector2 projectileStartPosition = {playerPosition.x, playerPosition.y};
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

    float getBurstCD() const { return burstCD; }
    void setBurstCD(float value) { burstCD = value; }
    Vector2& getPlayerPosition() { return playerPosition; }
    int updatePlayerScore(int amount) { return playerScore += amount; }
    int getPlayerHealth() const { return playerHealth; }
    void decreaseHealth(int amount) { playerHealth -= amount; }
    int getPlayerDamage() const { return playerDamage; }
    void setPlayerDamage(int amount) { playerDamage = amount; }
    int getPlayerSpeed() const { return playerSpeed; }
    void setPlayerSpeed(int amount) { playerSpeed = amount; }
};

class Enemy
{
private:
    int enemyHealth;
    int enemySpeed;
    Vector2 enemyPosition;
public:
    Enemy(int enemyHealth, int enemySpeed, Vector2 enemyPosition) : enemyHealth(enemyHealth), enemySpeed(enemySpeed), enemyPosition(enemyPosition) {}

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

    Vector2 getEnemyPosition() const { return enemyPosition; }
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
    float waveTimer = 30.0f;
    int currentWave = 0;
    int numOfEnemies = 5;

public:
    void SpawnEnemies(Player& player, std::vector<Enemy>& enemies, int NUM_OF_WAVES)
    {
        waveTimer += GetFrameTime();
        if (waveTimer >= 30.0f && currentWave < NUM_OF_WAVES)
        {
            for (int i = 0; i < numOfEnemies; i++)
            {
                float angle = i * (360.0f / numOfEnemies) * DEG2RAD;
                Vector2 spawnPosition = {
                    player.getPlayerPosition().x + cos(angle) * ENEMY_SPAWN_RADIUS,
                    player.getPlayerPosition().y + sin(angle) * ENEMY_SPAWN_RADIUS
                };
                enemies.emplace_back(100, 2, spawnPosition);
            }

            currentWave++;
            numOfEnemies += 2;
            waveTimer = 0.0f;
            std::cout << "Wave " << currentWave << " of " << NUM_OF_WAVES << " has begun!" << std::endl;
            std::cout << "Enemy Count: " << numOfEnemies << std::endl;
        }
    }

    int getCurrentWave() { return currentWave; }
    float getWaveTimer() { return waveTimer; }
    int setNumOfEnemies(int amount) { return numOfEnemies = amount; }
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

    struct ActivePowerUp
    {
        int type;
        float remainingTime;
    };

    std::vector<PowerUp> positionalPowerUps;
    std::vector<ActivePowerUp> activePowerUps;

public:
    void SpawnPowerUp(int powerUpType)
    {
        PowerUp newPowerUp = {powerUpType, {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)}, true};
        positionalPowerUps.push_back(newPowerUp);
    }

    void DrawPowerUps()
    {
        for (const auto& powerUp : positionalPowerUps)
        {
            if (!powerUp.active) continue;

            Color color;
            switch (powerUp.type)
            {
            case 0: color = GREEN; break;
            case 1: color = YELLOW; break;
            case 2: color = RED; break;
            case 3: color = BLUE; break;
            default: color = GRAY; break;
            }

            DrawCircle(powerUp.position.x, powerUp.position.y, 10, color);
        }
    }

    void CheckPowerUpCollision(Player& player)
    {
        for (auto& powerUp : positionalPowerUps)
        {
            if (!powerUp.active) continue;

            if (CollisionManager::CheckCollision(player.getPlayerPosition(), powerUp.position, 16, 10))
            {
                powerUp.active = false;
                StartPowerUpTimer(player, powerUp.type);
            }
        }
    }

    void StartPowerUpTimer(Player& player, int powerUpType)
    {
        float duration = 10.0f;
        ApplyPowerUp(player, powerUpType);
        activePowerUps.push_back({powerUpType, duration});
    }

    void UpdatePowerUpTimers(Player& player)
    {
        for (size_t i = 0; i < activePowerUps.size();)
        {
            ActivePowerUp& powerUp = activePowerUps[i];
            powerUp.remainingTime -= GetFrameTime();

            if (powerUp.remainingTime <= 0.0f)
            {
                RevertPowerUp(player, powerUp.type);
                activePowerUps.erase(activePowerUps.begin() + i);
            }
            else
            {
                ++i;
            }
        }
    }

    void ApplyPowerUp(Player& player, int powerUpType)
    {
        switch (powerUpType)
        {
        case 0: break;
        case 1: player.setPlayerSpeed(player.getPlayerSpeed() + 2); break;
        case 2: player.setPlayerDamage(20); break;
        case 3: player.setBurstCD(0.0f); break;
        }
    }

    void RevertPowerUp(Player& player, int powerUpType)
    {
        switch (powerUpType)
        {
        case 1: player.setPlayerSpeed(player.getPlayerSpeed() - 2); break;
        case 2: player.setPlayerDamage(10); break;
        case 3: break;
        }
    }
};

int main()
{
    const char* windowTitle = "LoopLoop";
    std::vector<Projectile> projectiles;
    std::vector<Enemy> enemies;
    Player player(100, 5, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, KEY_W, KEY_S, KEY_A, KEY_D, MOUSE_BUTTON_LEFT, projectiles);
    GameManager gameManager;
    PowerUpManager powerUpManager;
    CollisionManager collisionManager;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowTitle);
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);

        player.movePlayer();
        player.UpdateBurstCD();
        player.DrawPlayer(BLACK);

        gameManager.SpawnEnemies(player, enemies, 5);
        for (Enemy& enemy : enemies)
        {
            enemy.MoveEnemy(player.getPlayerPosition());
            enemy.DrawEnemy(RED);
        }

        for (size_t i = 0; i < projectiles.size(); i++)
        {
            projectiles[i].MoveProjectile();
            projectiles[i].DrawProjectile();
        }

        for (size_t i = 0; i < projectiles.size(); i++)
        {
            for (size_t j = 0; j < enemies.size(); j++)
            {
                if (CollisionManager::CheckCollision(projectiles[i].getPosition(), enemies[j].getEnemyPosition(), 10, 16))
                {
                    enemies[j].decreaseHealth(player.getPlayerDamage());
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

        collisionManager.CheckBounds(player.getPlayerPosition(), SCREEN_WIDTH, SCREEN_HEIGHT);

        powerUpManager.DrawPowerUps();
        powerUpManager.CheckPowerUpCollision(player);
        powerUpManager.UpdatePowerUpTimers(player);

        std::string playerScoreText = "Score: " + std::to_string(player.updatePlayerScore(0));
        int playerScoreTextWidth = MeasureText(playerScoreText.c_str(), 20);
        Vector2 scorePosition = {SCREEN_WIDTH / 2 - playerScoreTextWidth / 2, 10};
        DrawText(playerScoreText.c_str(), scorePosition.x, scorePosition.y, 20, BLACK);

        std::string waveText = "Current Wave: " + std::to_string(gameManager.getCurrentWave());
        std::string waveTimerText = "Next Wave: " + std::to_string(static_cast<int>(ceil(gameManager.getWaveTimer())));
        int waveTextWidth = MeasureText(waveText.c_str(), 20);
        int waveTimerTextWidth = MeasureText(waveTimerText.c_str(), 20);
        Vector2 wavePosition = {SCREEN_WIDTH / 2 - waveTextWidth - 10, 40};
        Vector2 waveTimerPosition = {SCREEN_WIDTH / 2 + 10, 40};

        DrawText(waveText.c_str(), wavePosition.x, wavePosition.y, 20, BLACK);
        DrawText(waveTimerText.c_str(), waveTimerPosition.x, waveTimerPosition.y, 20, BLACK);

        if (player.getBurstCD() > 0.0f) { DrawText(("Burst CD: " + std::to_string(static_cast<int>(ceil(player.getBurstCD())))).c_str(), 10, 100, 20, RED); }
        else { DrawText("Burst: Ready!", 10, 100, 20, GREEN); }
        // Store Player Health as a variable so that we can measure the text
        // to help with centering / positioning.
        std::string playerHealthText = "Health: " + std::to_string(player.getPlayerHealth());
        int playerHealthTextWidth = MeasureText(playerHealthText.c_str(), 20);
        Vector2 textPosition = {SCREEN_WIDTH / 2 - playerHealthTextWidth / 2, SCREEN_HEIGHT - 30};
        DrawText(playerHealthText.c_str(), textPosition.x, textPosition.y, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
