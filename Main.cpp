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
    int playerScore;
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

int main()
{
    const char *windowTitle = "LoopLoop";
    std::vector<Projectile> projectiles;
    std::vector<Enemy> enemies;
    static Player player("Unhalted", 100, 5, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, KEY_W, KEY_S, KEY_A, KEY_D, MOUSE_BUTTON_LEFT, projectiles);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowTitle);
    SetTargetFPS(TARGET_FPS);

    for (int i = 0; i < NUM_ENEMIES; i++)
    {
        float angle = i * (360.0f / NUM_ENEMIES) * DEG2RAD;
        Vector2 spawnPosition = {
            player.getPlayerPosition().x + cos(angle) * ENEMY_SPAWN_RADIUS,
            player.getPlayerPosition().y + sin(angle) * ENEMY_SPAWN_RADIUS
        };
        enemies.emplace_back(100, 2, spawnPosition);
    }
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);
        player.DrawPlayer(BLACK);
        player.movePlayer();
        player.UpdateBurstCD();
        CollisionManager::CheckBounds(player.getPlayerPosition(), SCREEN_WIDTH, SCREEN_HEIGHT);
        std::string burstText = "Burst CD: " + std::to_string(static_cast<int>(ceil(player.getBurstCD())));
        if (player.getBurstCD() <= 0.0f) { burstText = "Burst Ready!"; }
        DrawText(burstText.c_str(), 10, 10, 20, BLACK);
        DrawText(("Score: " + std::to_string(player.updatePlayerScore(0))).c_str(), 10, 40, 20, BLACK);
        for (size_t i = 0; i < projectiles.size(); i++)
        {
            projectiles[i].MoveProjectile();
            projectiles[i].DrawProjectile();
        }

        for (Enemy &enemy : enemies)
        {
            enemy.MoveEnemy(player.getPlayerPosition());
            enemy.DrawEnemy(RED);
        }

        for (size_t i = 0; i < projectiles.size(); i++)
        {
            for (size_t j = 0; j < enemies.size(); j++)
            {
                if (CollisionManager::CheckCollision(projectiles[i].getPosition(), enemies[j].getEnemyPosition(), 10, 16)) // Check Collision Between Projectile & Enemy.
                {
                    enemies[j].decreaseHealth(10);
                    // TODO: Check Damage Based on Projectile Type
                    projectiles.erase(projectiles.begin() + i);
                    i--;
                    if (enemies[j].isDead())
                    {
                        enemies.erase(enemies.begin() + j);
                        j--;
                        player.updatePlayerScore(10);
                    }
                    break;
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
