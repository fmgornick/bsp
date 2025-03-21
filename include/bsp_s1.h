#ifndef BSP_S1_H_
#define BSP_S1_H_

#include "bsp_stages.h"
#include "bsp_utils.h"
#include "i32_vector.h"
#include "raylib.h"
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

typedef struct S1_Scene {
    /* 2D grid for building polygon */
    Cell grid[ROWS][COLS];
    Cell *currentCell;

    /* polygon constructed by user */
    IVector2 polygon[MAX_VERTICES];
    u32 numVertices;
} S1_Scene;

BSP_Stage S1_Init(S1_Scene *scene);
BSP_Stage S1_Render(S1_Scene *scene);
BSP_Stage S1_RenderFailure(S1_Scene *scene);
void S1_Free(S1_Scene *scene);

void S1_GridInit(S1_Scene *scene);
void S1_DrawCells(S1_Scene *scene);
void S1_DrawPolygon(S1_Scene *scene);
void S1_UpdateActiveCell(S1_Scene *scene);
bool S1_IntersectingPolygon(S1_Scene *scene);
void S1_DrawMessage(char *msg, Color fg, Color bg);

#endif // BSP_S1_H_
