#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"
#include "Game.h"
Game* Game::gameInstance = nullptr;
#include "TicksAndFPS.h"
#include "Player.h"
#include "Guard.h"
#include <iostream>
#include <string>
#include "math.h"
#include "PlayerController.h"


//triangle vertices
const Vector2 triVert0{ 0.0F, -15.F };
const Vector2 triVert1{ -10.0F, 10.F };
const Vector2 triVert2{ 10.0F, 10.F };

Game* Game::get()
{
    if (gameInstance == nullptr)
    {
        gameInstance = new Game();
    }
    return gameInstance;
}

void Game::close()
{
    if (gameInstance != nullptr)
    {
        gameInstance->closeRaylib();
        delete gameInstance;
    }
}

Game::Game()
{
    ticksAndFps = new TicksAndFPS(30);
    thePlayer = new Player((float)(screenWidth - 20 - (rand() % (screenWidth - 20))), (float)(rand() % (screenHeight - 40)) + 20.0F, (float)(rand() % 360));
    playerController = PlayerController::get();
    guards = new Guard[guardCount];

    for (int i = 0; i < guardCount; i++)
    {
        guards[i] = Guard((float)(screenWidth - 20 - (rand() % (screenWidth - 20))), (float)(rand() %  (screenHeight-40)) + 20.0F, (float)(rand() % 360));
    }
    buildLevelWalls();
    InitWindow(screenWidth, screenHeight, "Fredrick - AI for games");
}

Game::~Game()
{
    PlayerController::close();
    delete thePlayer;
    delete[] guards;
    delete ticksAndFps;
}

void Game::onFrame()
{
    if (closing = WindowShouldClose()) return;
    ticksAndFps->doOnTickUntillRealtimeSync(this);
    playerController->update();
    ticksAndFps->updateFPS();
}

void Game::onTick()
{
    for (int i = 0; i < guardCount; i++)
    {
        guards[i].onTick();
    }
    thePlayer->onTick();
}

void Game::drawScene()
{
    BeginDrawing();
    
    ClearBackground(LIGHTGRAY);
    
    DrawText("Use arrow keys to move", screenWidth - 260, 10, 20, BLACK);

    for (int i = 0; i < guardCount; i++)
    {
        Guard& theGuard = guards[i];
        Vector2 guardLerpPos = theGuard.getLerpPos();
        Vector2 triVert0Copy = triVert0;
        Vector2 triVert1Copy = triVert1;
        Vector2 triVert2Copy = triVert2;

        //rotation
        float radians =  Game::radians(theGuard.getLerpRotation());

        float newVert0X = triVert0Copy.x * std::cosf(radians) - triVert0Copy.y * std::sinf(radians);
        float newVert0Y = triVert0Copy.x * std::sinf(radians) + triVert0Copy.y * std::cosf(radians);

        float newVert1X = triVert1Copy.x * std::cosf(radians) - triVert1Copy.y * std::sinf(radians);
        float newVert1Y = triVert1Copy.x * std::sinf(radians) + triVert1Copy.y * std::cosf(radians);

        float newVert2X = triVert2Copy.x * std::cosf(radians) - triVert2Copy.y * std::sinf(radians);
        float newVert2Y = triVert2Copy.x * std::sinf(radians) + triVert2Copy.y * std::cosf(radians);
        
        triVert0Copy.x = newVert0X;
        triVert0Copy.y = newVert0Y;

        triVert1Copy.x = newVert1X;
        triVert1Copy.y = newVert1Y;

        triVert2Copy.x = newVert2X;
        triVert2Copy.y = newVert2Y;

        //translation
        triVert0Copy = Vector2Add(triVert0Copy, guardLerpPos);
        triVert1Copy = Vector2Add(triVert1Copy, guardLerpPos);
        triVert2Copy = Vector2Add(triVert2Copy, guardLerpPos);
        DrawTriangle(triVert0Copy, triVert1Copy, triVert2Copy, RED);
        if (drawDebug)
        {
            //DEBUG front vector line
            DrawLine(guardLerpPos.x, guardLerpPos.y, guardLerpPos.x + theGuard.getLerpFrontVec().x * 50, guardLerpPos.y + theGuard.getLerpFrontVec().y * 50, DARKGREEN);
            //DEBUG aabb box
            drawAABB(*theGuard.getAABB(), false);
        }
    }

    //do same for player

    Vector2 triVert0Copy = triVert0;
    Vector2 triVert1Copy = triVert1;
    Vector2 triVert2Copy = triVert2;

    //rotation
    float radians = Game::radians(thePlayer->getLerpRotation());

    float newVert0X = triVert0Copy.x * std::cosf(radians) - triVert0Copy.y * std::sinf(radians);
    float newVert0Y = triVert0Copy.x * std::sinf(radians) + triVert0Copy.y * std::cosf(radians);

    float newVert1X = triVert1Copy.x * std::cosf(radians) - triVert1Copy.y * std::sinf(radians);
    float newVert1Y = triVert1Copy.x * std::sinf(radians) + triVert1Copy.y * std::cosf(radians);

    float newVert2X = triVert2Copy.x * std::cosf(radians) - triVert2Copy.y * std::sinf(radians);
    float newVert2Y = triVert2Copy.x * std::sinf(radians) + triVert2Copy.y * std::cosf(radians);

    triVert0Copy.x = newVert0X;
    triVert0Copy.y = newVert0Y;

    triVert1Copy.x = newVert1X;
    triVert1Copy.y = newVert1Y;

    triVert2Copy.x = newVert2X;
    triVert2Copy.y = newVert2Y;

    //translation
    Vector2 playerLerpPos = thePlayer->getLerpPos();
    triVert0Copy = Vector2Add(triVert0Copy, playerLerpPos);
    triVert1Copy = Vector2Add(triVert1Copy, playerLerpPos);
    triVert2Copy = Vector2Add(triVert2Copy, playerLerpPos);

    DrawTriangle(triVert0Copy, triVert1Copy, triVert2Copy, DARKBLUE);
    if (drawDebug)
    {
        //DEBUG front vector line
        DrawLine(playerLerpPos.x, playerLerpPos.y, playerLerpPos.x + thePlayer->getLerpFrontVec().x * 50, playerLerpPos.y + thePlayer->getLerpFrontVec().y * 50, DARKGREEN);
        //DEBUG aabb box
        drawAABB(*thePlayer->getAABB(), false);
    }

    //drawing all walls
    for (std::vector<AABB>::iterator i = levelWallBoxes.begin(); i != levelWallBoxes.end(); i++)
    {
        drawAABB(*i, true);
    }
    DrawText((std::string("FPS: ") + std::to_string(ticksAndFps->getFps())).c_str(), 15, 10, 20, DARKGREEN);
    EndDrawing();
}

void Game::drawAABB(AABB box, bool wall)
{
    if (wall)
    {
        DrawRectangle(box.minBounds.x, box.minBounds.y, box.maxBounds.x - box.minBounds.x, box.maxBounds.y - box.minBounds.y, BLACK);
    }
    else
    {
        DrawRectangleLines(box.minBounds.x, box.minBounds.y, box.maxBounds.x - box.minBounds.x, box.maxBounds.y - box.minBounds.y, MAGENTA);
    }
}

void Game::tryToMoveEntity(EntityLiving* ent, Vector2& entVel)
{
    AABB* entBox = ent->getAABB();
    
	/*test if entity bounding box aligns with a wall, if so, correct velocity so
	  it will not go through the wall in the way.*/

	//correct y velocity
    for (std::vector<AABB>::iterator wall = levelWallBoxes.begin(); wall != levelWallBoxes.end(); wall++)
    {
        if (AABB::overlappingX(*entBox, *wall))
        {
            float yDist;//distance between boxes in y direction, depending on position and velocity

            //if ent is moving towards positive y and entity box is "below" wall box
			if (entVel.y > 0.0F && entBox->maxBounds.y <= wall->minBounds.y)
			{
                yDist = wall->minBounds.y - entBox->maxBounds.y;
                if (yDist < entVel.y)
                {
                    entVel.y = yDist;
                }
			}

            //if ent is moving towards negative y and entity box is "above" wall box
            if (entVel.y < 0.0F && entBox->minBounds.y >= wall->maxBounds.y)
            {
                yDist = wall->maxBounds.y - entBox->minBounds.y;//creating negative dist for comparing with negative velocity so there is no need to use abs() func
                if (yDist > entVel.y)
                {
                    entVel.y = yDist;
                }
            }
        }

    }

    entBox->offset(Vector2{0, entVel.y});//move bounding box accordingly with new y velocity

    //correct x velocity
    for (std::vector<AABB>::iterator wall = levelWallBoxes.begin(); wall != levelWallBoxes.end(); wall++)
    {
        if (AABB::overlappingY(*entBox, *wall))
        {
            float xDist;//distance between boxes in x direction, depending on position and velocity

             //if ent is moving towards positive x and entity box is "to left" of wall box
            if (entVel.x > 0.0F && entBox->maxBounds.x <= wall->minBounds.x)
            {
                xDist = wall->minBounds.x - entBox->maxBounds.x;
                if (xDist < entVel.x)
                {
                    entVel.x = xDist;
                }
            }

            //if ent is moving towards negative x and entity box is "to right" of wall box
            if (entVel.x < 0.0F && entBox->minBounds.x >= wall->maxBounds.x)
            {
                xDist = wall->maxBounds.x - entBox->minBounds.x;//creating negative dist for comparing with negative velocity so there is no need to use abs() func
                if (xDist > entVel.x)
                {
                    entVel.x = xDist;
                }
            }
        }
    }

    entBox->offset(Vector2{ entVel.x, 0 });//move bounding box accordingly with new x velocity     
    
    ent->setPos(entBox->getCenter());//set entity position to align with its new bounding box position
}

float Game::lerp(float start, float dest)
{
    return start + (dest - start) * ticksAndFps->getPercentageToNextTick();
}

void Game::toggleBooleanOnButtonPress(bool button, bool& booleanToToggle)
{
    if (button)
    {
        booleanToToggle = !booleanToToggle;
    }
}

void Game::closeRaylib()
{
    CloseWindow();        // Close window and OpenGL context
}

void Game::buildLevelWalls()
{
    addWall(-10, 0, 10, screenHeight);//left screen bounds wall
    addWall(screenWidth - 10, 0, screenWidth + 10, screenHeight);//right screen bounds wall
    addWall(10, screenHeight - 10, screenWidth - 10, screenHeight + 10);//bottom screen bounds wall
    addWall(10, -10, screenWidth - 10, 10);//top screen bounds wall
    addWall(100, 100, 200, 200);
    addWall(200, 200, 300, 300);
}

void Game::addWall(float minX, float minY, float maxX, float maxY)
{
    levelWallBoxes.push_back(AABB(minX, minY, maxX, maxY));
}

float Game::radians(float degrees)
{
    return degrees * ((float)(3.141592653589F) / 180.0F);
}
