/**
* Author: Eliot Kominer
* Assignment: Pong Variations
* Date due: 10/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// Professor Romero Cruz, i am so sorry for what you are about to experience

#include "CS3113/cs3113.h"
#include <math.h>

constexpr int SCREEN_WIDTH = 1600;
constexpr int SCREEN_HEIGHT = 900;
constexpr int FPS = 60;

constexpr float UFO_SPEED = 700.0f;
constexpr float ASTEROID_SPEED = 640.0f;

constexpr int WIN_SCORE = 5;
constexpr int MAX_ASTEROIDS = 3;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

constexpr int SCROLL_SPEED = 100;

constexpr float SUN_BASE_SIZE = 400.0f;
constexpr float SUN_PULSE_AMP = 16.0f;
constexpr float SUN_PULSE_FREQ = 3.0f;
constexpr float SUN_SPEED_X = 40.0f;
constexpr float SUN_SPEED_Y = 8.0f;
constexpr Color SUN_TINT = { 255, 255, 255, 50 };

constexpr float EARTH_SIZE = 60.0f;
constexpr float EARTH_ORBIT_RADIUS = 300.0f;
constexpr float EARTH_ORBIT_SPEED = 1.0f;
constexpr float ORBIT_MARGIN = EARTH_ORBIT_RADIUS + EARTH_SIZE / 2.0f;
constexpr Vector2 SUN_START = { SCREEN_WIDTH + ORBIT_MARGIN, 150.0f };

// hitbox size as seen on screen (the UFO is drawn sideways, see render)
constexpr Vector2 UFO_BASE_SIZE = { 110.0f, 260.0f };
constexpr Vector2 ASTEROID_BASE_SIZE = { 100.0f, 100.0f };
constexpr Vector2 WIN_MESSAGE_SIZE = { 400.0f, 160.0f };

// left UFO turns counterclockwise, right UFO turns clockwise
constexpr float UFO1_ANGLE = -90.0f;
constexpr float UFO2_ANGLE = 90.0f;

constexpr Vector2 UFO1_INIT_POSITION = { 120.0f, ORIGIN.y };
constexpr Vector2 UFO2_INIT_POSITION = { SCREEN_WIDTH - 120.0f, ORIGIN.y };

constexpr char BG_COLOUR[] = "#000000";
constexpr char BG_FP[] = "assets/game/starfield.png";
constexpr char UFO_FP[] = "assets/game/ufo.png";
constexpr char SUN_FP[] = "assets/game/sun.png";
constexpr char EARTH_FP[] = "assets/game/earth.png";
constexpr char ASTEROID_FP[] = "assets/game/asteroid.png";
constexpr char PLAYER1_WON_FP[] = "assets/game/player1won.png";
constexpr char PLAYER2_WON_FP[] = "assets/game/player2won.png";

AppStatus gAppStatus = RUNNING;

float gPreviousTicks = 0.0f;

float gBgScrollX = 0.0f;

float gSunSize = SUN_BASE_SIZE;
float gSunTime = 0.0f;
Vector2 gSunPosition = SUN_START;

float gEarthTime = 0.0f;
Vector2 gEarthPosition = SUN_START;

bool gGameOver = false;
bool gOnePlayerMode = false;
bool gPlayer1Won = false;
int gScoreUfo1 = 0;
int gScoreUfo2 = 0;
int gActiveCount = 1;

Vector2 gUfo1Position = UFO1_INIT_POSITION;
Vector2 gUfo1Movement = { 0.0f, 0.0f };
Vector2 gUfo1Scale = UFO_BASE_SIZE;

Vector2 gUfo2Position = UFO2_INIT_POSITION;
Vector2 gUfo2Movement = { 0.0f, 0.0f };
Vector2 gUfo2Scale = UFO_BASE_SIZE;

Vector2 gAsteroidPositions[MAX_ASTEROIDS] = { ORIGIN, ORIGIN, ORIGIN };
Vector2 gAsteroidMovements[MAX_ASTEROIDS] = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
Vector2 gAsteroidScale = ASTEROID_BASE_SIZE;
bool gAsteroidActive[MAX_ASTEROIDS] = { true, false, false };

Texture2D gBackgroundTexture;
Texture2D gUfoTexture;
Texture2D gSunTexture;
Texture2D gEarthTexture;
Texture2D gAsteroidTexture;
Texture2D gPlayer1WonTexture;
Texture2D gPlayer2WonTexture;

void initialise();
void processInput();
void update();
void render();
void shutdown();

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB);

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale, float angle = 0.0f, bool flipVertical = false,
                  Color tint = WHITE);

void setAsteroids(int count);
void resetAsteroid(int index, float directionX);
void boundaryCheck(Vector2 *position, const Vector2 *scale);

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;
    return false;
}

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale, float angle, bool flipVertical, Color tint)
{
    // a negative source height flips the texture vertically
    float sourceHeight = static_cast<float>(texture->height);
    if (flipVertical) sourceHeight = -sourceHeight;

    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        sourceHeight
    };

    Rectangle destinationArea = {
        position->x,
        position->y,
        scale->x,
        scale->y
    };

    Vector2 originOffset = {
        scale->x / 2.0f,
        scale->y / 2.0f
    };

    DrawTexturePro(
        *texture,
        textureArea, destinationArea, originOffset,
        angle, tint
    );
}

void resetAsteroid(int index, float directionX)
{
    float offsetsY[MAX_ASTEROIDS] = { -80.0f, 0.0f, 80.0f };
    float directionsY[MAX_ASTEROIDS] = { -0.6f, 0.0f, 0.6f };

    gAsteroidPositions[index] = { ORIGIN.x, ORIGIN.y + offsetsY[index] };
    float length = sqrt(directionX * directionX + directionsY[index] * directionsY[index]);
    gAsteroidMovements[index] = { directionX / length, directionsY[index] / length };
}

void setAsteroids(int count)
{
    gActiveCount = count;

    for (int i = 0; i < MAX_ASTEROIDS; i++)
    {
        gAsteroidActive[i] = (i < count);
    }

    for (int i = 0; i < count; i++)
    {
        resetAsteroid(i, 1.0f);
    }
}

void boundaryCheck(Vector2 *position, const Vector2 *scale)
{
    float halfHeight = scale->y / 2.0f;

    if (position->y - halfHeight < 0.0f) position->y = halfHeight;
    if (position->y + halfHeight > SCREEN_HEIGHT) position->y = SCREEN_HEIGHT - halfHeight;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Pong with Sebastian Romero Cruz!");

    gBackgroundTexture = LoadTexture(BG_FP);
    gUfoTexture = LoadTexture(UFO_FP);
    gSunTexture = LoadTexture(SUN_FP);
    gEarthTexture = LoadTexture(EARTH_FP);
    gAsteroidTexture = LoadTexture(ASTEROID_FP);
    gPlayer1WonTexture = LoadTexture(PLAYER1_WON_FP);
    gPlayer2WonTexture = LoadTexture(PLAYER2_WON_FP);

    setAsteroids(1);
    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    if (gGameOver) return;

    if (IsKeyPressed(KEY_ONE)) setAsteroids(1);
    if (IsKeyPressed(KEY_TWO)) setAsteroids(2);
    if (IsKeyPressed(KEY_THREE)) setAsteroids(3);

    if (IsKeyPressed(KEY_T))
    {
        gOnePlayerMode = !gOnePlayerMode;

        if (gOnePlayerMode) gUfo2Movement.y = 1.0f;
    }

    gUfo1Movement.y = 0.0f;
    if (IsKeyDown(KEY_W)) gUfo1Movement.y = -1.0f;
    if (IsKeyDown(KEY_S)) gUfo1Movement.y = 1.0f;

    // player 2's keys are ignored in one-player mode
    if (!gOnePlayerMode)
    {
        gUfo2Movement.y = 0.0f;
        if (IsKeyDown(KEY_UP)) gUfo2Movement.y = -1.0f;
        if (IsKeyDown(KEY_DOWN)) gUfo2Movement.y = 1.0f;
    }
}

void update()
{
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    gBgScrollX += SCROLL_SPEED * deltaTime;
    if (gBgScrollX >= SCREEN_WIDTH)
    {
        gBgScrollX -= SCREEN_WIDTH;
    }

    gSunTime += deltaTime;
    gSunPosition.x -= SUN_SPEED_X * deltaTime;
    gSunPosition.y += SUN_SPEED_Y * deltaTime;

    gSunSize = SUN_BASE_SIZE + SUN_PULSE_AMP * sin(gSunTime * SUN_PULSE_FREQ);

    if (gSunPosition.x < -ORBIT_MARGIN)
    {
        gSunPosition = SUN_START;
    }

    // earth's position is built from the sun's, so it keeps orbiting as the sun drifts
    gEarthTime += deltaTime;
    gEarthPosition.x = gSunPosition.x + EARTH_ORBIT_RADIUS * cos(gEarthTime * EARTH_ORBIT_SPEED);
    gEarthPosition.y = gSunPosition.y + EARTH_ORBIT_RADIUS * sin(gEarthTime * EARTH_ORBIT_SPEED);

    if (gGameOver) return;

    // one-player mode: UFO 2 patrols up and down on its own
    if (gOnePlayerMode)
    {
        float halfHeight = gUfo2Scale.y / 2.0f;

        if (gUfo2Position.y - halfHeight <= 0.0f) gUfo2Movement.y = 1.0f;
        else if (gUfo2Position.y + halfHeight >= SCREEN_HEIGHT) gUfo2Movement.y = -1.0f;
        else if (gUfo2Movement.y == 0.0f) gUfo2Movement.y = 1.0f;
    }

    gUfo1Position.y += UFO_SPEED * deltaTime * gUfo1Movement.y;
    gUfo2Position.y += UFO_SPEED * deltaTime * gUfo2Movement.y;

    boundaryCheck(&gUfo1Position, &gUfo1Scale);
    boundaryCheck(&gUfo2Position, &gUfo2Scale);

    for (int i = 0; i < MAX_ASTEROIDS; i++)
    {
        if (!gAsteroidActive[i]) continue;

        gAsteroidPositions[i].x += ASTEROID_SPEED * deltaTime * gAsteroidMovements[i].x;
        gAsteroidPositions[i].y += ASTEROID_SPEED * deltaTime * gAsteroidMovements[i].y;

        float halfAsteroidHeight = gAsteroidScale.y / 2.0f;
        float halfAsteroidWidth = gAsteroidScale.x / 2.0f;

        if (gAsteroidPositions[i].y + halfAsteroidHeight > SCREEN_HEIGHT)
        {
            gAsteroidPositions[i].y = SCREEN_HEIGHT - halfAsteroidHeight;
            gAsteroidMovements[i].y *= -1.0f;
        }

        if (gAsteroidPositions[i].y - halfAsteroidHeight < 0.0f)
        {
            gAsteroidPositions[i].y = halfAsteroidHeight;
            gAsteroidMovements[i].y *= -1.0f;
        }

        // fabs forces the direction so an asteroid can't get stuck inside a UFO
        if (isColliding(&gAsteroidPositions[i], &gAsteroidScale, &gUfo1Position, &gUfo1Scale))
        {
            gAsteroidMovements[i].x = fabs(gAsteroidMovements[i].x);
            gAsteroidPositions[i].x = gUfo1Position.x + (gUfo1Scale.x / 2.0f) + halfAsteroidWidth;
        }

        if (isColliding(&gAsteroidPositions[i], &gAsteroidScale, &gUfo2Position, &gUfo2Scale))
        {
            gAsteroidMovements[i].x = -fabs(gAsteroidMovements[i].x);
            gAsteroidPositions[i].x = gUfo2Position.x - (gUfo2Scale.x / 2.0f) - halfAsteroidWidth;
        }

        if (gAsteroidPositions[i].x - halfAsteroidWidth > SCREEN_WIDTH)
        {
            gScoreUfo1++;
            LOG("Player 1: " << gScoreUfo1 << "  Player 2: " << gScoreUfo2);
            resetAsteroid(i, -1.0f);
        }

        if (gAsteroidPositions[i].x + halfAsteroidWidth < 0.0f)
        {
            gScoreUfo2++;
            LOG("Player 1: " << gScoreUfo1 << "  Player 2: " << gScoreUfo2);
            resetAsteroid(i, 1.0f);
        }
    }

    if (gScoreUfo1 >= WIN_SCORE)
    {
        gGameOver = true;
        gPlayer1Won = true;
    }
    else if (gScoreUfo2 >= WIN_SCORE)
    {
        gGameOver = true;
        gPlayer1Won = false;
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    Rectangle bgTextureArea = {
        0.0f, 0.0f,
        static_cast<float>(gBackgroundTexture.width),
        static_cast<float>(gBackgroundTexture.height)
    };

    // two copies side by side make the background scroll seamlessly
    Rectangle bgA = {
        gBgScrollX, 0.0f,
        static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT)
    };

    Rectangle bgB = {
        gBgScrollX - SCREEN_WIDTH, 0.0f,
        static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT)
    };

    Vector2 bgOrigin = { 0.0f, 0.0f };

    DrawTexturePro(gBackgroundTexture, bgTextureArea, bgA, bgOrigin, 0.0f, WHITE);
    DrawTexturePro(gBackgroundTexture, bgTextureArea, bgB, bgOrigin, 0.0f, WHITE);

    float sunAspect = static_cast<float>(gSunTexture.height) / static_cast<float>(gSunTexture.width);
    Vector2 sunScale = { gSunSize, gSunSize * sunAspect };
    renderObject(&gSunTexture, &gSunPosition, &sunScale, 0.0f, false, SUN_TINT);

    float earthAspect = static_cast<float>(gEarthTexture.height) / static_cast<float>(gEarthTexture.width);
    Vector2 earthScale = { EARTH_SIZE, EARTH_SIZE * earthAspect };
    renderObject(&gEarthTexture, &gEarthPosition, &earthScale, 0.0f, false, SUN_TINT);

    // x and y are swapped because the UFO is rotated 90 degrees before it hits the screen
    Vector2 ufo1DrawScale = { gUfo1Scale.y, gUfo1Scale.x };
    Vector2 ufo2DrawScale = { gUfo2Scale.y, gUfo2Scale.x };
    renderObject(&gUfoTexture, &gUfo1Position, &ufo1DrawScale, UFO1_ANGLE, true);
    renderObject(&gUfoTexture, &gUfo2Position, &ufo2DrawScale, UFO2_ANGLE, true);

    if (!gGameOver)
    {
        for (int i = 0; i < MAX_ASTEROIDS; i++)
        {
            if (!gAsteroidActive[i]) continue;
            renderObject(&gAsteroidTexture, &gAsteroidPositions[i], &gAsteroidScale);
        }
    }
    else
    {
        if (gPlayer1Won) renderObject(&gPlayer1WonTexture, &ORIGIN, &WIN_MESSAGE_SIZE);
        else renderObject(&gPlayer2WonTexture, &ORIGIN, &WIN_MESSAGE_SIZE);
    }

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gUfoTexture);
    UnloadTexture(gSunTexture);
    UnloadTexture(gEarthTexture);
    UnloadTexture(gAsteroidTexture);
    UnloadTexture(gPlayer1WonTexture);
    UnloadTexture(gPlayer2WonTexture);
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}