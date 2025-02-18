#include "bsp_utils.h"
#include "raylib.h"
#include "vector.h"
#include <stdbool.h>
#include <stdint.h>

#define ROWS 36
#define COLS 64
#define MVERTICES 10

uint32_t cellWidth = WIDTH / COLS;
uint32_t cellHeight = HEIGHT / ROWS;

typedef struct Cell {
    int32_t i;   // cell row
    int32_t j;   // cell column
    bool active; // true if mouse hovering over cell
    bool chosen; // true if user clicked on cell
} Cell;

Cell grid[ROWS][COLS];
Cell *currentCell = &grid[0][0];

// first and last vertex should be duplicates
IVector2 polygon[MVERTICES + 1];
uint32_t numVertices = 0;

void GridInit();
void DrawCell(Cell cell);
void DrawPolygon();
bool IntersectingPolygon();

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

        DrawPolygon();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IntersectingPolygon())
        {
            polygon[numVertices++] = (IVector2){ currentCell->j, currentCell->i };

            if (numVertices == MVERTICES)
            {
                polygon[numVertices++] = polygon[0];
                /* break; */
            }
        }

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

void
DrawPolygon()
{
    if (numVertices >= 2)
    {
        for (uint32_t i = 0; i < numVertices - 1; i++)
        {
            Vector2 p = { polygon[i].x * cellWidth + cellWidth / 2.0f, polygon[i].y * cellHeight + cellHeight / 2.0f };
            Vector2 q = { polygon[i + 1].x * cellWidth + cellWidth / 2.0f, polygon[i + 1].y * cellHeight + cellHeight / 2.0f };
            DrawLineEx(p, q, 2.0f, BLACK);
        }
    }
    if (numVertices >= 1)
    {
        Vector2 p = { polygon[numVertices - 1].x * cellWidth + cellWidth / 2.0f,
                      polygon[numVertices - 1].y * cellHeight + cellHeight / 2.0f };
        Vector2 q = { currentCell->j * cellWidth + cellWidth / 2.0f, currentCell->i * cellHeight + cellHeight / 2.0f };

        if (IntersectingPolygon())
            DrawLineEx(p, q, 2.0f, RED);
        else
            DrawLineEx(p, q, 2.0f, LIGHTGRAY);
    }
}

bool
IntersectingPolygon()
{
    if (numVertices <= 1)
        return false;

    IVector2 p = polygon[numVertices - 1];
    IVector2 q = (IVector2){ currentCell->j, currentCell->i };

    for (uint32_t i = 0; i < numVertices - 1; i++)
    {
        IVector2 s = polygon[i];
        IVector2 t = polygon[i + 1];
        IVector2 pq = IVector2Subtract(q, p);
        IVector2 st = IVector2Subtract(t, s);

        if (i == numVertices - 2)
            return (IVector2Determinant(pq, st) == 0 && IVector2DotProduct(pq, st) < 0);

        IVector2 ps = IVector2Subtract(s, p);
        IVector2 pt = IVector2Subtract(t, p);
        IVector2 sp = IVector2Subtract(p, s);
        IVector2 sq = IVector2Subtract(q, s);

        if (!SAME_SIGN(IVector2Determinant(pq, ps), IVector2Determinant(pq, pt))
            && !SAME_SIGN(IVector2Determinant(st, sp), IVector2Determinant(st, sq)))
            return true;
    }

    return false;
}
