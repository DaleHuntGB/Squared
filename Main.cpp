#include <raylib.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define WINDOW_TITLE "Squared"
#define TARGET_FPS 60


class TextureManager
{
public:
    static inline Texture2D playerTexture;
    static inline Texture2D enemyTexture;

    static void LoadTextures()
    {
        playerTexture = LoadTexture("Resources/Assets/Player.png");
        enemyTexture = LoadTexture("Resources/Assets/Enemy.png");
    }
};

void SetupGameWindow()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    TextureManager::LoadTextures();
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
    Projectile(Vector2 position, Vector2 direction, int speed, int damage, int size) : projectilePosition(position), projectileDirection(direction), projectileSpeed(speed), projectileDamage(damage), projectileSize(size), isActive(true) {}

    void Update()
    {
        if (isActive)
        {
            projectilePosition.x += projectileDirection.x * projectileSpeed;
            projectilePosition.y += projectileDirection.y * projectileSpeed;
            if (projectilePosition.x < 0 || projectilePosition.x > SCREEN_WIDTH || projectilePosition.y < 0 || projectilePosition.y > SCREEN_HEIGHT) { isActive = false; }
        }
    }

    void Draw()
    {
        if (isActive) { DrawCircle(projectilePosition.x, projectilePosition.y, projectileSize, GREEN); }
    }

    static void Shoot(std::vector<Projectile>& projectileObjects, Vector2 startPosition, Vector2 targetPosition, int speed, int damage, int size)
    {
        Vector2 projectileDirection = { targetPosition.x - startPosition.x, targetPosition.y - startPosition.y };
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
};

class Entity
{
public:
    virtual void Draw() = 0;

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
    void Draw() override { 
        DrawTextureEx(TextureManager::playerTexture, entityPosition, 0, 1, WHITE); 
        }

    void Move(std::vector<Projectile>& projectileObjects)
    {
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) entityPosition.x += entitySpeed;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) entityPosition.x -= entitySpeed;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) entityPosition.y -= entitySpeed;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) entityPosition.y += entitySpeed;
        if (IsKeyPressed(KEY_SPACE)) 
        {
            Vector2 mousePosition = GetMousePosition();
            Projectile::Shoot(projectileObjects, entityPosition, mousePosition, 10, 20, 5);
        }

        if (IsKeyPressed(KEY_F))
        {
            std::cout << "Size: " << entitySize << std::endl;
        }

        if (entityPosition.x > SCREEN_WIDTH) entityPosition.x = 0;
        if (entityPosition.x < 0) entityPosition.x = SCREEN_WIDTH;
        if (entityPosition.y > SCREEN_HEIGHT) entityPosition.y = 0;
        if (entityPosition.y < 0) entityPosition.y = SCREEN_HEIGHT;
    }
};

class Enemy : public Entity
{
public:
    void Draw() override { DrawTextureEx(TextureManager::enemyTexture, entityPosition, 0, 1, WHITE); }
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
};

class FontManager
{
public:
    static inline Font displayFont;
    static inline Font scoreFont;
    static inline Font healthFont;
    static void UnloadFonts()
    {
        UnloadFont(displayFont);
        UnloadFont(scoreFont);
        UnloadFont(healthFont);
    }
};

class GameManager
{
public:
    static void StartGame()
    {
        ClearBackground(RAYWHITE);
        if (PC.GetHealth() <= 0)
        {
            isGameRunning = false;
            char* gameOverText = "GAME OVER!\nPress R to Restart\nQ to Quit";
            float textWidth = MeasureTextEx(FontManager::displayFont, gameOverText, 24, 0).x;
            float textHeight = MeasureTextEx(FontManager::displayFont, gameOverText, 24, 0).y;
            float textPosX = (SCREEN_WIDTH / 2) - textWidth / 2;
            float textPosY = SCREEN_HEIGHT / 2 - textHeight / 2;
            DrawTextEx(FontManager::displayFont, gameOverText, {textPosX, textPosY}, 24, 0, RED);
            HandleUserInput();
            return;
        }
        else
        {
            Update();
        }
    }

    static void ManageUnits()
    {
        for (auto& enemy : enemyUnits)
        {
            enemy.Draw();
            enemy.Move(PC.GetPosition());
        }

        for (auto& projectile : projectileObjects)
        {
            projectile.Update();
            projectile.Draw();
        }

        projectileObjects.erase(std::remove_if(projectileObjects.begin(), projectileObjects.end(), [](const Projectile& p) { return !p.IsActive(); }), projectileObjects.end());
    }

    static void Update()
    {
        PC.Draw();
        PC.Move(projectileObjects);
        ManageUnits();
        HandleCollision();
        DrawTextEx(FontManager::healthFont, ("Health: " + std::to_string(PC.GetHealth())).c_str(), {10, 10}, 24, 0, BLACK);
    }

    static void HandleCollision()
    {
        for (int i = 0; i < enemyUnits.size(); i++)
        {
            if (CheckCollisionRecs( {PC.GetPosition().x, PC.GetPosition().y, (float)PC.GetSize(), (float)PC.GetSize()}, {enemyUnits[i].GetPosition().x, enemyUnits[i].GetPosition().y, (float)enemyUnits[i].GetSize(), (float)enemyUnits[i].GetSize()}))
            {
                PC.SetHealth(PC.GetHealth() - enemyUnits[i].GetCollisionDamage());
                enemyUnits.erase(enemyUnits.begin() + i);
                i--;
            }
        }
    }

    static void Initialize()
    {
        isGameRunning = true;
        gameShouldClose = false;
        FontManager::displayFont = LoadFont("Resources/Fonts/DisplayFont.ttf");
        FontManager::scoreFont = LoadFont("Resources/Fonts/ScoreFont.otf");
        FontManager::healthFont = LoadFont("Resources/Fonts/ScoreFont.otf");
        PC.SetHealth(100);
        PC.SetSpeed(5);
        PC.SetPosition({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
        enemyUnits.clear();

        for (int i = 0; i < numEnemies; i++)
        {
            Enemy enemy;
            enemy.SetPosition({(float)GetRandomValue(0, SCREEN_WIDTH), (float)GetRandomValue(0, SCREEN_HEIGHT)});
            enemy.SetSpeed(3);
            enemyUnits.push_back(enemy);
        }
    }

    static void HandleUserInput()
    {
        if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) gameShouldClose = true;
        if (IsKeyPressed(KEY_R)) Restart();
    }

    static void Restart()
    {
        Initialize();
    }

    static bool GameShouldClose() { return gameShouldClose; }

private:
    static inline Player PC;
    static inline std::vector<Enemy> enemyUnits;
    static inline std::vector<Projectile> projectileObjects;
    static inline bool isGameRunning = true;
    static inline bool gameShouldClose = false;
    static inline int numEnemies = 5;
};

int main()
{
    SetupGameWindow();
    GameManager::Initialize();

    while (!WindowShouldClose() && !GameManager::GameShouldClose())
    {
        BeginDrawing();
        GameManager::StartGame();
        EndDrawing();
    }
    FontManager::UnloadFonts();
    CloseWindow();
    return 0;
}
