#include "bsp_utils.h"
#include "raylib.h"
#include <stdint.h>

#define ROWS 36
#define COLS 64

uint32_t cellWidth = WIDTH / COLS;
uint32_t cellHeight = HEIGHT / ROWS;

typedef struct Cell {
    uint32_t i;  // cell row
    uint32_t j;  // cell column
    bool active; // true if mouse hovering over cell
    bool chosen; // true if user clicked on cell
} Cell;

Cell grid[ROWS][COLS];
Cell *currentCell = &grid[0][0];

void GridInit();
void DrawCell(Cell cell);

int32_t
main(int32_t argc, char *argv[])
{
    GridInit();
    InitWindow(WIDTH, HEIGHT, "blah");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Vector2 mouse = GetMousePosition();
        uint32_t i = mouse.y / cellHeight;
        uint32_t j = mouse.x / cellWidth;

        if (i != currentCell->i || j != currentCell->j)
        {
            currentCell->active = false;
            currentCell = &grid[i][j];
            currentCell->active = true;
        }

        for (uint32_t i = 0; i < ROWS; i++)
            for (uint32_t j = 0; j < COLS; j++)
                DrawCell(grid[i][j]);

        EndDrawing();
    }
}

void
GridInit()
{
    for (uint32_t i = 0; i < ROWS; i++)
    {
        for (uint32_t j = 0; j < COLS; j++)
        {
            grid[i][j] = (Cell){
                .i = i,
                .j = j,
                .active = false,
                .chosen = false,
            };
        }
    }
}

void
DrawCell(Cell cell)
{
    if (cell.active)
        DrawRectangle(cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight, LIGHTGRAY);
    else
        DrawRectangle(cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight, WHITE);

    DrawRectangleLines(cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight, LIGHTGRAY);
}
