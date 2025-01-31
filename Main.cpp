#include <raylib.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define WINDOW_TITLE "Squared"
#define TARGET_FPS 60

class FontManager
{
public:
    Font displayFont;
    Font scoreFont;
    Font uiFont;

    void LoadFonts()
    {
        displayFont = LoadFont("Resources/Fonts/DisplayFont.ttf");
        scoreFont = LoadFont("Resources/Fonts/ScoreFont.otf");
        uiFont = LoadFont("Resources/Fonts/ScoreFont.otf");
    }

    void UnloadFonts()
    {
        UnloadFont(displayFont);
        UnloadFont(scoreFont);
        UnloadFont(uiFont);
    }
};

class TextureManager
{
public:
    Texture2D playerTexture;
    Texture2D enemyTexture;
    Texture2D projectileTexture;
    Texture2D healthTexture;
    Texture2D powerUpTexture;
    Texture2D lifeTexture;

    void LoadTextures()
    {
        playerTexture = LoadTexture("Resources/Assets/Player.png");
        enemyTexture = LoadTexture("Resources/Assets/Enemy.png");
        projectileTexture = LoadTexture("Resources/Assets/Projectile.png");
        healthTexture = LoadTexture("Resources/Assets/Health.png");
        powerUpTexture = LoadTexture("Resources/Assets/PowerUp.png");
        lifeTexture = LoadTexture("Resources/Assets/Life.png");
    }

    void UnloadTextures()
    {
        UnloadTexture(playerTexture);
        UnloadTexture(enemyTexture);
        UnloadTexture(projectileTexture);
        UnloadTexture(healthTexture);
        UnloadTexture(powerUpTexture);
        UnloadTexture(lifeTexture);
    }
};

void SetupGameWindow(TextureManager& TM, FontManager& FM)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    FM.LoadFonts();
    TM.LoadTextures();
}

class Projectile
{
protected:
    Vector2 projectilePosition;
    Vector2 projectileDirection;
    int projectileSpeed;
    int projectileDamage;
    int projectileSize;
    bool isActive;

public:
    Projectile(Vector2 position, Vector2 direction, int speed, int damage, int size)
        : projectilePosition(position), projectileDirection(direction), projectileSpeed(speed), projectileDamage(damage), projectileSize(size), isActive(true) {}

    void Update()
    {
        if (isActive)
        {
            projectilePosition.x += projectileDirection.x * projectileSpeed;
            projectilePosition.y += projectileDirection.y * projectileSpeed;
            if (projectilePosition.x < 0 || projectilePosition.x > SCREEN_WIDTH || projectilePosition.y < 0 || projectilePosition.y > SCREEN_HEIGHT)
            {
                isActive = false;
            }
        }
    }

    void Draw(TextureManager& TM)
    {
        if (isActive)
        {
            DrawTextureEx(TM.projectileTexture, projectilePosition, 0, 1, WHITE);
        }
    }


    static void Shoot(std::vector<Projectile>& projectileObjects, Vector2 startPosition, Vector2 targetPosition, int speed, int damage, int size)
    {
        Vector2 projectileDirection = {targetPosition.x - startPosition.x, targetPosition.y - startPosition.y};
        float projectileMagnitude = sqrt(projectileDirection.x * projectileDirection.x + projectileDirection.y * projectileDirection.y);

        if (projectileMagnitude > 0)
        {
            projectileDirection.x /= projectileMagnitude;
            projectileDirection.y /= projectileMagnitude;
        }

        projectileObjects.emplace_back(startPosition, projectileDirection, speed, damage, size);
    }

    bool IsActive() const { return isActive; }
    Vector2 GetPosition() const { return projectilePosition; }
    int GetDamage() const { return projectileDamage; }
    int GetSize() const { return projectileSize; }
    int GetSpeed() const { return projectileSpeed; }
    int SetSpeed(int speed) { return projectileSpeed = speed; }
};

class Entity
{
public:
    virtual void Draw(TextureManager& TM) = 0;

    int GetHealth() { return entityHealth; }
    void SetHealth(int health) { entityHealth = health; }

    int GetSpeed() { return entitySpeed; }
    void SetSpeed(int speed) { entitySpeed = speed; }

    int GetDamage() { return entityDamage; }
    void SetDamage(int damage) { entityDamage = damage; }

    int GetCollisionDamage() { return entityCollisionDamage; }
    void SetCollisionDamage(int damage) { entityCollisionDamage = damage; }

    int GetSize() { return entitySize; }
    void SetSize(int size) { entitySize = size; }

    Vector2 GetPosition() { return entityPosition; }
    void SetPosition(Vector2 position) { entityPosition = position; }

protected:
    int entityHealth = 100;
    int entitySpeed = 5;
    int entityDamage = 10;
    int entityCollisionDamage = 25;
    int entitySize = 32;
    Vector2 entityPosition = {0, 0};
};

class Player : public Entity
{
public:
    void Draw(TextureManager& TM) override
    {
        DrawTextureEx(TM.playerTexture, entityPosition, 0, 1, WHITE);
    }

    void Move(std::vector<Projectile>& projectileObjects)
    {
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            entityPosition.x += entitySpeed;
        }
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            entityPosition.x -= entitySpeed;
        }
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            entityPosition.y -= entitySpeed;
        }
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            entityPosition.y += entitySpeed;
        }
        if (IsKeyPressed(KEY_SPACE))
        {
            Vector2 mousePosition = GetMousePosition();
            Projectile::Shoot(projectileObjects, entityPosition, mousePosition, 10, 20, 5);
        }

        if (entityPosition.x > SCREEN_WIDTH) entityPosition.x = 0;
        if (entityPosition.x < 0) entityPosition.x = SCREEN_WIDTH;
        if (entityPosition.y > SCREEN_HEIGHT) entityPosition.y = 0;
        if (entityPosition.y < 0) entityPosition.y = SCREEN_HEIGHT;
    }
    int SetPlayerLives(int lives) { return playerLives = lives; }
    int GetPlayerLives() { return playerLives; }
private:
    int playerLives = 3;
};

class Enemy : public Entity
{
public:
    void Draw(TextureManager& TM) override
    {
        DrawTextureEx(TM.enemyTexture, entityPosition, 0, 1, WHITE);
        DrawRectangle(entityPosition.x, entityPosition.y - 10, entitySize, 5, RED);
        DrawRectangle(entityPosition.x, entityPosition.y - 10, entitySize * (entityHealth / 100.0f), 5, GREEN);
    }

    void Move(Vector2 playerPosition)
    {
        Vector2 direction = {playerPosition.x - entityPosition.x, playerPosition.y - entityPosition.y};
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0)
        {
            direction.x /= distance;
            direction.y /= distance;
            entityPosition.x += direction.x * entitySpeed;
            entityPosition.y += direction.y * entitySpeed;
        }
    }

    void ShootAtPlayer(std::vector<Projectile>& projectileObjects, Vector2 playerPosition)
    {
        Vector2 direction = {playerPosition.x - entityPosition.x, playerPosition.y - entityPosition.y};
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0 && canShoot)
        {
            Projectile::Shoot(projectileObjects, entityPosition, playerPosition, 5, 10, 5);
            canShoot = false;
        }
        else
        {
            shootCooldown -= GetFrameTime();
            if (shootCooldown <= 0)
            {
                canShoot = true;
                shootCooldown = 1;
            }
        }
    }
private:
    float shootCooldown = 0;
    bool canShoot = false;
};

class GameManager
{
public:
    // Level Stats: {Level, Enemies To Spawn, Enemy Speed}
    std::vector<std::tuple<int, int, int>> levelStats = {
        {1, 4, 2},
        {2, 7, 3},
        {3, 10, 4},
        {4, 12, 5},
        {5, 15, 6}
    };

    void SpawnEnemies()
    {
        int enemySpeed = std::get<2>(levelStats[gameLevel - 1]);
        if (gameLevel < 1 || gameLevel > levelStats.size()) { std::cout << "No Enemy For Level: " << gameLevel << std::endl; return; }
        int enemiesToSpawn = std::get<1>(levelStats[gameLevel - 1]);
        std::cout << "Game Level: " << gameLevel << " | Enemies Killed: " << enemiesKilled << std::endl;
        for (int i = 0; i < enemiesToSpawn; i++)
        {
            if (enemyUnits.size() >= enemiesToSpawn) break;
            Enemy enemy;
            enemy.SetPosition((Vector2){ GetRandomValue(-200, SCREEN_WIDTH + 200), GetRandomValue(-200, SCREEN_HEIGHT + 200) });
            enemy.SetSpeed(enemySpeed);
            enemyUnits.push_back(enemy);
            std::cout << "Spawned: " << i + 1 << " Enemies" << std::endl;
        }
}

    void Initialize(TextureManager& TM, FontManager& FM)
    {
        this->TM = &TM;
        this->FM = &FM;

        PC.SetHealth(100);
        PC.SetSpeed(5);
        PC.SetPosition({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
        PC.SetPlayerLives(3);
        gameTimer = 0;

        isGameRunning = true;

        enemyUnits.clear();
        SpawnEnemies();
    }

    void Update()
    {
        ClearBackground(RAYWHITE);

        HandlePlayerInput();

        if (isGameRunning)
        {
            HandlePlayer();
            HandleEnemies();
            HandleProjectiles();
            HandleCollision();

            if (!isGamePaused)
            {
                gameTimer += GetFrameTime();

                if (fmod(gameTimer, 3.0f) < 0.01f)
                {
                    // Do Something
                }
                if (fmod(gameTimer, waveTimer) < 0.01f)
                {
                    std::cout << "Game Timer: " << gameTimer << " | Wave Timer: " << waveTimer << std::endl;
                    SpawnEnemies();
                }
            }
        }
        else
        {
            GameIsOver();
        }

        DisplayUI();
    }

    bool GameShouldClose() const { return gameShouldClose; }

private:
    void HandlePlayer()
    {
        PC.Draw(*TM);
        PC.Move(playerProjectileObjects);
    }

    void HandlePlayerInput()
    {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q))
        {
            gameShouldClose = true;
        }
        if (IsKeyPressed(KEY_R))
        {
            Initialize(*TM, *FM);
        }
        if (isGameRunning && IsKeyPressed(KEY_P))
        {
            std::cout << "Game Paused" << std::endl;
            isGamePaused = !isGamePaused;

            for (auto& enemy : enemyUnits)
            {
                enemy.SetSpeed(isGamePaused ? 0 : 3);
            }

            for (auto& projectile : playerProjectileObjects)
            {
                projectile.SetSpeed(isGamePaused ? 0 : 10);
            }

            for (auto& projectile : enemyProjectileObjects)
            {
                projectile.SetSpeed(isGamePaused ? 0 : 5);
            }

            PC.SetSpeed(isGamePaused ? 0 : 5);

            gameTimer = isGamePaused ? gameTimer : gameTimer;
        }
    }

    void HandleEnemies()
    {
        for (auto& enemy : enemyUnits)
        {
            enemy.Draw(*TM);
            enemy.Move(PC.GetPosition());
            enemy.ShootAtPlayer(enemyProjectileObjects, PC.GetPosition());
        }
    }

    void HandleProjectiles()
    {
        for (auto& projectile : playerProjectileObjects)
        {
            projectile.Update();
            projectile.Draw(*TM);
        }
        playerProjectileObjects.erase( std::remove_if(playerProjectileObjects.begin(), playerProjectileObjects.end(), [](const Projectile& p) { return !p.IsActive(); }),
        playerProjectileObjects.end());

        for (auto& projectile : enemyProjectileObjects)
        {
            projectile.Update();
            projectile.Draw(*TM);
        }
        enemyProjectileObjects.erase( std::remove_if(enemyProjectileObjects.begin(), enemyProjectileObjects.end(), [](const Projectile& p) { return !p.IsActive(); }),
        enemyProjectileObjects.end());
    }

    void HandleCollision()
    {
        for (size_t i = 0; i < enemyUnits.size(); ++i)
        {
            if (CheckCollisionRecs( {PC.GetPosition().x, PC.GetPosition().y, (float)PC.GetSize(), (float)PC.GetSize()}, {enemyUnits[i].GetPosition().x, enemyUnits[i].GetPosition().y, (float)enemyUnits[i].GetSize(), (float)enemyUnits[i].GetSize()}))
            {
                PC.SetHealth(PC.GetHealth() - enemyUnits[i].GetCollisionDamage());
                enemyUnits.erase(enemyUnits.begin() + i);
                --i;
                if (PC.GetHealth() <= 0)
                {
                    std::cout << "Player Health: " << PC.GetHealth() << std::endl;
                    PC.SetPlayerLives(PC.GetPlayerLives() - 1);
                    std::cout << "Player Lives: " << PC.GetPlayerLives() << std::endl;
                    PC.SetHealth(100);
                    if (PC.GetPlayerLives() <= 0)
                    {
                        isGameRunning = false;
                    }
                }
                continue;
            }
            // Player Projectile Collision
            for (size_t j = 0; j < playerProjectileObjects.size(); ++j)
            {
                if (CheckCollisionRecs( {playerProjectileObjects[j].GetPosition().x, playerProjectileObjects[j].GetPosition().y, (float)playerProjectileObjects[j].GetSize(), (float)playerProjectileObjects[j].GetSize()}, {enemyUnits[i].GetPosition().x, enemyUnits[i].GetPosition().y, (float)enemyUnits[i].GetSize(), (float)enemyUnits[i].GetSize()}))
                {
                    enemyUnits[i].SetHealth(enemyUnits[i].GetHealth() - playerProjectileObjects[j].GetDamage());
                    playerProjectileObjects.erase(playerProjectileObjects.begin() + j);

                    if (enemyUnits[i].GetHealth() <= 0)
                    {
                        enemyUnits.erase(enemyUnits.begin() + i);
                        enemiesKilled++;
                        if (enemiesKilled == std::get<2>(levelStats[gameLevel - 1]))
                        {
                            gameLevel++;
                            enemiesKilled = 0;
                            PC.SetHealth(100);
                            PC.SetPlayerLives(3);
                        }
                        --i;
                    }
                    break;
                }
            }
            // Enemy Projectile Collision
            for (size_t j = 0; j < enemyProjectileObjects.size(); ++j)
            {
                if (CheckCollisionRecs( {enemyProjectileObjects[j].GetPosition().x, enemyProjectileObjects[j].GetPosition().y, (float)enemyProjectileObjects[j].GetSize(), (float)enemyProjectileObjects[j].GetSize()}, {PC.GetPosition().x, PC.GetPosition().y, (float)PC.GetSize(), (float)PC.GetSize()}))
                {
                    PC.SetHealth(PC.GetHealth() - enemyProjectileObjects[j].GetDamage());
                    enemyProjectileObjects.erase(enemyProjectileObjects.begin() + j);
                    if (PC.GetHealth() <= 0)
                    {
                        std::cout << "Player Health: " << PC.GetHealth() << std::endl;
                        PC.SetPlayerLives(PC.GetPlayerLives() - 1);
                        std::cout << "Player Lives: " << PC.GetPlayerLives() << std::endl;
                        PC.SetHealth(100);
                        if (PC.GetPlayerLives() <= 0)
                        {
                            isGameRunning = false;
                        }
                    }
                    break;
                }
            }
        }
    }

    void DisplayUI()
    {
        std::string healthText = "Health: " + std::to_string(PC.GetHealth());
        std::string playerLivesText = "Lives: " + std::to_string(PC.GetPlayerLives());
        std::string gameTimerText = "Game Time: " + std::to_string((int)gameTimer);
        std::string activeEnemiesText = "Active Enemies: " + std::to_string(enemyUnits.size());
        std::string gameLevelText = "Level: " + std::to_string(gameLevel);
        std::string enemiesKilledText = "Enemies Killed: " + std::to_string(enemiesKilled);
        if (isGameRunning)
        {
            DrawTextEx(FM->uiFont, healthText.c_str(), {10, 10}, 24, 0, BLACK);
            DrawTextEx(FM->uiFont, playerLivesText.c_str(), {10, 40}, 24, 0, BLACK);
            DrawTextEx(FM->uiFont, gameTimerText.c_str(), {10, 70}, 24, 0, BLACK);
            DrawTextEx(FM->uiFont, activeEnemiesText.c_str(), {10, 100}, 24, 0, BLACK);
            DrawTextEx(FM->uiFont, gameLevelText.c_str(), {10, 130}, 24, 0, BLACK);
            DrawTextEx(FM->uiFont, enemiesKilledText.c_str(), {10, 160}, 24, 0, BLACK);
        }
        else
        {
            GameIsOver();
        }
    }

    void GameIsOver()
    {
        HandlePlayerInput(); // Handle Player Input to Restart or Quit
        char* GameOverMessage = "You died! Press R to Restart or Q to Quit."; // Char* for DrawTextEx Only :)
        float GameOverMessageWidth = MeasureTextEx(FM->displayFont, GameOverMessage, 24, 0).x;
        DrawTextEx(FM->displayFont, GameOverMessage, {(SCREEN_WIDTH - GameOverMessageWidth) / 2, SCREEN_HEIGHT / 2}, 24, 0, BLACK);
        // TODO: Remove Enemies & Projectiles
    }

    void SetGameLevel(int level) { gameLevel = level; }
    int GetGameLevel() { return gameLevel; }

    TextureManager* TM;
    FontManager* FM;

    Player PC;
    std::vector<Enemy> enemyUnits;
    std::vector<Projectile> playerProjectileObjects;
    std::vector<Projectile> enemyProjectileObjects;

    float gameTimer = 0;
    float waveTimer = 5;
    bool isGameRunning = true;
    bool isGamePaused = false;
    bool gameShouldClose = false;

    int numEnemies = 5;
    int gameLevel = 1;
    int enemiesKilled = 0;
};

void CleanUp(FontManager& FM, TextureManager& TM)
{
    FM.UnloadFonts();
    TM.UnloadTextures();
    CloseWindow();
}

int main()
{
    GameManager GM;
    FontManager FM;
    TextureManager TM;
    SetupGameWindow(TM, FM);
    GM.Initialize(TM, FM);
    while (!WindowShouldClose() && !GM.GameShouldClose())
    {
        BeginDrawing();
        GM.Update();
        EndDrawing();
    }
    CleanUp(FM, TM);
    return 0;
}