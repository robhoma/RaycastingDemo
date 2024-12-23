#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <string>


const int viewportWidth = 1280; //3D VIEWPORT
const int topdownWidth = 400;   //2D TOPDOWN VIEW

const int screenWidth = viewportWidth + topdownWidth;
const int screenHeight = 720;
const int worldWidth = 8;
const int worldHeight = 8;

const float moveSpeed = 0.02;
const float rotationSpeed = 0.02;

Vector2 player = { 5.0f, 5.0f };
Vector2 direction = { -1.0f, 0.0f };
Vector2 plane = { 0.0f, 0.66f };
Vector2 camera;
Vector2 rayDir;
Vector2 oldPos;
Vector2 oldDirection;
Vector2 oldPlane;
Vector2 step;
int side;
int lineHeight;
int drawStartPos;
int drawEndPos;
double cameraX;
double rayAngle;
double perpendicularDistance;
Vector2 rayDirection = { 0.0, 0.0 };
Vector2 mapPos;
Vector2 deltaDistance;
Vector2 sideDistance;
Vector2 steps;
bool DDA_hit = false;
int DDA_side = 0;
Color wallColor;
bool DDA_tileFound = false;

Vector2 grid = { worldWidth, worldHeight };


Color EMPTYSPACE = { 192, 192, 192, 225 };
Color WALL = { 102, 178, 255, 255 };
Color COLUMN = { 51, 0, 25, 255};

int world[worldWidth][worldHeight] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,2,2,2,2,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1}
};



// Main Loop
int main(void)
{
    InitWindow(screenWidth, screenHeight, "Raycasting Demo");

    SetTargetFPS(120);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        //----------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------
        // 2D Viewport
        // ---------------------------------------------------------------------------------
        // 

        int squareSpacing = topdownWidth / worldWidth;
        Color squareColor;

        for (int gridX = 0; gridX < worldWidth; gridX++) {
            for (int gridY = 0; gridY < worldHeight; gridY++) {
                // Square math
                int squareStartX = viewportWidth + (squareSpacing * gridX);
                int squareStartY = screenHeight - topdownWidth + (squareSpacing * gridY);

                // Color based on world content
                if (world[gridX][gridY] == 0) {
                    squareColor = EMPTYSPACE;
                }
                if (world[gridX][gridY] == 1) {
                    squareColor = WALL;
                }
                if (world[gridX][gridY] == 2) {
                    squareColor = COLUMN;
                }
                if (world[gridX][gridY] == 3) {
                    squareColor = BLUE;
                }

                DrawRectangle(squareStartX, squareStartY, squareSpacing, squareSpacing, squareColor);
            }
        }


        // Coordinates are within the realm of the 2D viewport
        Vector2 viewportPos = { squareSpacing * player.x + viewportWidth, squareSpacing * player.y + (screenHeight - topdownWidth) };

        // Player on 2D Viewport
        DrawCircle(viewportPos.x, viewportPos.y, 5, RED);


        //----------------------------------------------------------------------------------
        // 3D DDA Algorithm
        // ---------------------------------------------------------------------------------
        // 3D Viewport Algorithm

        for (int i = 0; i < viewportWidth; i++) {

            camera.x = ((2.0f * i) / double(screenWidth)) - 1;
            rayDir.x = direction.x + plane.x * camera.x;
            rayDir.y = direction.y + plane.y * camera.x;

            Vector2 mapPos = { static_cast<int>(player.x), static_cast<int>(player.y) };

            Vector2 deltaDistance = { abs(1.0f / rayDir.x), abs(1.0f / rayDir.y) };

            if (rayDir.x < 0) {
                step.x = -1;
                sideDistance.x = (player.x - mapPos.x) * deltaDistance.x;
            }
            else {
                step.x = 1;
                sideDistance.x = (mapPos.x + 1.0f - player.x) * deltaDistance.x;
            }

            if (rayDir.y < 0) {
                step.y = -1;
                sideDistance.y = (player.y - mapPos.y) * deltaDistance.y;
            }
            else {
                step.y = 1;
                sideDistance.y = (mapPos.y + 1.0f - player.y) * deltaDistance.y;
            }

            bool DDA_hit = false;

            // DDA Algorithm
            while (!DDA_hit) {
                if (sideDistance.x < sideDistance.y) {
                    sideDistance.x += deltaDistance.x;
                    mapPos.x += step.x;
                    side = 0;
                }
                else {
                    sideDistance.y += deltaDistance.y;
                    mapPos.y += step.y;
                    side = 1;
                }

                if (world[static_cast<int>(mapPos.x)][static_cast<int>(mapPos.y)] > 0) {
                    DDA_hit = true;
                }
            }

            if (side == 0) {
                perpendicularDistance = sideDistance.x - deltaDistance.x;
            }
            else {
                perpendicularDistance = sideDistance.y - deltaDistance.y;
            }

            // Ensure perpWallDistance is not zero
            if (perpendicularDistance == 0) {
                perpendicularDistance = 1; // Small non-zero value to prevent division by zero
            }


            int lineHeight = int(screenHeight / perpendicularDistance);

            int drawStart = (-lineHeight / 2) + (screenHeight / 2);
            if (drawStart <= 0) {
                drawStart = 0;
            }
            int drawEnd = (lineHeight / 2) + (screenHeight / 2);
            if (drawEnd >= screenHeight) {
                drawEnd = screenHeight - 1;
            }

            switch (world[static_cast<int>(mapPos.x)][static_cast<int>(mapPos.y)])
            {
            case 1:  wallColor = WALL;  break; //wall
            case 2:  wallColor = COLUMN;  break; //column

            case 3:  wallColor = BLUE;   break; //blue
            case 4:  wallColor = RAYWHITE;  break; //white
            default: wallColor = DARKBLUE; break; //yellow
            }

            if (side == 1) {
                ColorBrightness(wallColor, -1.0f);
            }
            else {
                ColorBrightness(wallColor, 1.0f);
            }


            DrawLine(i, drawStart, i, drawEnd, wallColor);
        }


        //----------------------------------------------------------------------------------
        // User Input
        // ---------------------------------------------------------------------------------
        // 
        oldPos = player; // Required for collisions

        if (IsKeyDown(KEY_W)) {
            if (world[int(player.x + direction.x * moveSpeed)][(int)player.y] == false) {
                player.x += direction.x * moveSpeed;
            }
            if (world[(int)player.x][int(player.y + direction.y * moveSpeed)] == false) {
                player.y += direction.y * moveSpeed;
            }
        }
        if (IsKeyDown(KEY_S)) {
            if (world[int(player.x - direction.x * moveSpeed)][(int)player.y] == false) {
                player.x -= direction.x * moveSpeed;
            }
            if (world[(int)player.x][int(player.y - direction.y * moveSpeed)] == false) {
                player.y -= direction.y * moveSpeed;
            }
        }
        if (IsKeyDown(KEY_A)) {
            oldDirection.x = direction.x;
            oldPlane.x = plane.x;

            direction.x = direction.x * cos(rotationSpeed) - direction.y * sin(rotationSpeed);
            direction.y = oldDirection.x * sin(rotationSpeed) + direction.y * cos(rotationSpeed);
            plane.x = plane.x * cos(rotationSpeed) - plane.y * sin(rotationSpeed);
            plane.y = oldPlane.x * sin(rotationSpeed) + plane.y * cos(rotationSpeed);

        }
        if (IsKeyDown(KEY_D)) {
            oldDirection.x = direction.x;
            oldPlane.x = plane.x;

            direction.x = direction.x * cos(-rotationSpeed) - direction.y * sin(-rotationSpeed);
            direction.y = oldDirection.x * sin(-rotationSpeed) + direction.y * cos(-rotationSpeed);
            plane.x = plane.x * cos(-rotationSpeed) - plane.y * sin(-rotationSpeed);
            plane.y = oldPlane.x * sin(-rotationSpeed) + plane.y * cos(-rotationSpeed);
        }

        // Out of Bounds and Wall Collision
        Vector2 playerTile = { static_cast<int>(std::floor(player.x)), static_cast<int>(std::floor(player.y)) };
        if (player.x < 0 || player.x >= worldWidth || player.y < 0 || player.y >= worldHeight) {
            player = oldPos;
        }
        if (world[(int)playerTile.x][(int)playerTile.y] != 0) {
            player = oldPos;
        }

        //----------------------------------------------------------------------------------
        // Misc.
        // ---------------------------------------------------------------------------------
        // 
        //FPS Counter
        int currentFPS = GetFPS();
        DrawText(std::to_string(currentFPS).c_str(), (screenWidth - 30), 0, 20, YELLOW);

        //Viewport & Topdown divider
        DrawLine(viewportWidth, 0, viewportWidth, screenHeight, RAYWHITE);
        DrawLine(viewportWidth, screenHeight - topdownWidth, screenWidth, screenHeight - topdownWidth, RAYWHITE);

        //Player Pos. Debug Text
        int xPadding = 5;
        DrawText((std::string("x-Pos: ") + std::to_string(player.x)).c_str(), viewportWidth + xPadding, 0, 20, YELLOW);
        DrawText((std::string("y-Pos: ") + std::to_string(player.y)).c_str(), viewportWidth + xPadding, 20, 20, YELLOW);
        DrawText((std::string("x-Dir: ") + std::to_string(direction.x)).c_str(), viewportWidth + xPadding, 40, 20, YELLOW);
        DrawText((std::string("y-Dir: ") + std::to_string(direction.y)).c_str(), viewportWidth + xPadding, 60, 20, YELLOW);

        //Clearing screen
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}