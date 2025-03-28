#ifndef S1_H_
#define S1_H_

#include "bsp.h"
#include "i32_vector.h"
#include <stdbool.h>

#define ROWS 36
#define COLS 64

#define CELL_WIDTH 20  /* dependent on (WIDTH / COLS) */
#define CELL_HEIGHT 20 /* dependent on (HEIGHT / ROWS) */

typedef struct Cell {
    i32 i;       /* cell row */
    i32 j;       /* cell column */
    bool active; /* true if mouse hovering over cell */
} Cell;

typedef struct S1 {
    /* 2D grid for building polygon */
    Cell grid[ROWS][COLS];
    Cell *currentCell;

    /* polygon constructed by user */
    IVector2 polygon[MAX_VERTICES];
    u32 numVertices;
} S1;

BspStage S1_Init(S1 *scene);
BspStage S1_Render(S1 *scene);
BspStage S1_RenderFailure(S1 *scene);
void S1_Free(S1 *scene);

// void GridInit(S1 *scene);
// void DrawCells(S1 *scene);
// void DrawPolygon(S1 *scene);
// void UpdateActiveCell(S1 *scene);
// bool IntersectingPolygon(S1 *scene);
// void DrawMessage(char *msg, Color fg, Color bg);

#endif // S1_H_
