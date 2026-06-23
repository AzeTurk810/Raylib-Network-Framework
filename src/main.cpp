#pragma once
#include <cstdio>
#include <string>
#define RAYGUI_IMPLEMENTATION
#include "inc/globs.hpp"
#include <cstdlib>
#include <ctime>
#include <raygui.h>
#include <enet/enet.h>

Color BlackCellColor = BROWN;
Color WhiteCellColour = WHITE;

inline void Draw(void);
inline void ResetGame(void);
inline void DrawIndexs(void);
Cell grid[COLS][ROWS];

int main(int argc, char ** argv) {
    srand(time(0));

    InitWindow(screenWidth, screenHeight, "Raylib ChessGame");
    InitAudioDevice();
    SetTargetFPS(60);
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occured while intializing Enet\n");
        return EXIT_FAILURE;
    }
    ResetGame();
    /// Every Frame
    while (!WindowShouldClose()) {
        BeginDrawing();
        Draw();
        EndDrawing();
    }
    CloseAudioDevice();
    CloseWindow();
    enet_deinitialize();
    return 0;
}

inline void Draw(void) {
    ClearBackground(GREEN);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j].DrawCell();
        }
    }
    DrawIndexs();
}

inline void initGrid(void) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Color curColor = ((i + j) & 1) ? BlackCellColor : WhiteCellColour;
            grid[i][j] = (Cell) {.i = i, .j = j, .piece = "empty", .isFree = true, .color = curColor};
        }
    }
}

inline void DrawIndexs(void) {
    for (int i = 0; i < ROWS; i++) {
        std::string curchar = "a";
        curchar[0] += i;
        int posX = cellHeight * i + cellHeight / 2 + padding - 14;
        int posY = boardHeight + padding + 10;
        DrawText(curchar.c_str(),  posX,  posY,  28, RED);
    }
    for (int j = 0; j < COLS; j++) {
        std::string curchar = "8";
        curchar[0] -= j;
        int posX = padding - 14 - 10;
        int posY = cellWidth * j + cellWidth / 2 + padding - 14;
        DrawText(curchar.c_str(),  posX,  posY,  28, RED);
    }
}

inline void ResetGame(void) {
    initGrid();
}

