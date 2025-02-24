#include "bsp_s1.h"
#include "bsp_stages.h"
#include "bsp_utils.h"
#include "bsp_vector.h"
#include "raylib.h"

BSP_Stage
S1_Init(S1_Scene *scene)
{
    S1_GridInit(scene);
    return S1_PENDING;
}

BSP_Stage
S1_Render(S1_Scene *scene)
{
    BSP_Stage nextStage = S1_PENDING;
    BeginDrawing();
    ClearBackground(RAYWHITE);

    S1_UpdateActiveCell(scene);
    S1_DrawCells(scene);
    S1_DrawPolygon(scene);

    DrawText(TextFormat("remaining vertices: %d", MAX_VERTICES - scene->numVertices), 10, 10, 20, BLACK);
    if (scene->numVertices == 0)
        S1_DrawMessage("select cells to create simple polygon (don't run out)", BLACK, BLUE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !S1_IntersectingPolygon(scene))
    {
        IVector2 newVertex = { scene->currentCell->j, scene->currentCell->i };

        if (scene->numVertices == MAX_VERTICES)
            nextStage = (IVector2DIsEqual(newVertex, scene->polygon[0])) ? S1_COMPLETED : S1_FAILED;
        else if (scene->numVertices >= 3 && IVector2DIsEqual(newVertex, scene->polygon[0]))
            nextStage = S1_COMPLETED;
        else
            scene->polygon[scene->numVertices++] = newVertex;
    }

    EndDrawing();
    return nextStage;
}

BSP_Stage
S1_RenderFailure(S1_Scene *scene)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    S1_DrawCells(scene);
    S1_DrawPolygon(scene);
    S1_DrawMessage("too many vertices: press 'R' if you would like to start over", BLACK, RED);

    if (IsKeyPressed(KEY_R))
    {
        scene->currentCell->active = false;
        scene->currentCell = &scene->grid[0][0];
        scene->numVertices = 0;
        return S1_PENDING;
    }

    EndDrawing();
    return S1_FAILED;
}

void
S1_GridInit(S1_Scene *scene)
{
    for (usize i = 0; i < ROWS; i++)
        for (usize j = 0; j < COLS; j++)
            scene->grid[i][j] = (Cell){ i, j, false };
    scene->currentCell = &scene->grid[0][0];
}

void
S1_DrawCells(S1_Scene *scene)
{
    for (usize i = 0; i < ROWS; i++)
    {
        for (usize j = 0; j < COLS; j++)
        {
            Cell cell = scene->grid[i][j];
            if (cell.active)
                DrawRectangle(cell.j * CELL_WIDTH, cell.i * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, LIGHTGRAY);
            else
                DrawRectangle(cell.j * CELL_WIDTH, cell.i * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, WHITE);

            DrawRectangleLines(cell.j * CELL_WIDTH, cell.i * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, LIGHTGRAY);
        }
    }
}

void
S1_UpdateActiveCell(S1_Scene *scene)
{
    Vector2 mouse = GetMousePosition();
    i32 i = mouse.y / CELL_HEIGHT;
    i32 j = mouse.x / CELL_WIDTH;

    if (i != scene->currentCell->i || j != scene->currentCell->j)
    {
        if (i >= 0 && i < ROWS && j >= 0 && j < COLS)
        {
            scene->currentCell->active = false;
            scene->currentCell = &scene->grid[i][j];
            scene->currentCell->active = true;
        }
    }
}

void
S1_DrawPolygon(S1_Scene *scene)
{
    if (scene->numVertices >= 2)
    {
        for (usize i = 0; i < scene->numVertices - 1; i++)
        {
            Vector2 p = { scene->polygon[i].x * CELL_WIDTH + CELL_WIDTH / 2.0f, scene->polygon[i].y * CELL_HEIGHT + CELL_HEIGHT / 2.0f };
            Vector2 q = { scene->polygon[i + 1].x * CELL_WIDTH + CELL_WIDTH / 2.0f, scene->polygon[i + 1].y * CELL_HEIGHT + CELL_HEIGHT / 2.0f };
            DrawLineEx(p, q, 2.0f, BLACK);
        }
    }
    if (scene->numVertices >= 1)
    {
        Vector2 p = { scene->polygon[scene->numVertices - 1].x * CELL_WIDTH + CELL_WIDTH / 2.0f,
                      scene->polygon[scene->numVertices - 1].y * CELL_HEIGHT + CELL_HEIGHT / 2.0f };
        Vector2 q = { scene->currentCell->j * CELL_WIDTH + CELL_WIDTH / 2.0f, scene->currentCell->i * CELL_HEIGHT + CELL_HEIGHT / 2.0f };

        if (S1_IntersectingPolygon(scene))
            DrawLineEx(p, q, 2.0f, RED);
        else
            DrawLineEx(p, q, 2.0f, LIGHTGRAY);
    }
}

bool
S1_IntersectingPolygon(S1_Scene *scene)
{
    /* can't intersect if there's no line to intersect */
    if (scene->numVertices <= 1)
        return false;

    /* line from last chosen square to the square containing our cursor */
    IVector2 p = scene->polygon[scene->numVertices - 1];
    IVector2 q = (IVector2){ scene->currentCell->j, scene->currentCell->i };

    for (usize i = 0; i < scene->numVertices - 1; i++)
    {
        IVector2 s = scene->polygon[i];
        IVector2 t = scene->polygon[i + 1];
        IVector2 pq = IVector2Subtract(q, p);
        IVector2 st = IVector2Subtract(t, s);

        /* make sure we don't make a line on top of the last one */
        if (i == scene->numVertices - 2)
            return (IVector2Determinant(pq, st) == 0 && IVector2DotProduct(pq, st) < 0);

        /* don't check the first line for intersections if we form a loop */
        if (i == 0 && IVector2DIsEqual(q, s))
            continue;

        IVector2 ps = IVector2Subtract(s, p);
        IVector2 pt = IVector2Subtract(t, p);
        IVector2 sp = IVector2Subtract(p, s);
        IVector2 sq = IVector2Subtract(q, s);

        /* edge cases are covered, so we can now just do an intersection test */
        if (sign(IVector2Determinant(pq, ps)) != sign(IVector2Determinant(pq, pt)) && sign(IVector2Determinant(st, sp)) != sign(IVector2Determinant(st, sq)))
            return true;
    }

    return false;
}

void
S1_DrawMessage(char *msg, Color fg, Color bg)
{
    DrawRectangle(0, 3 * (HEIGHT / 4), WIDTH, HEIGHT / 4, Fade(bg, 0.5));

    Vector2 textSize = MeasureTextEx(GetFontDefault(), msg, 20, 1.0f);
    u32 xPos = (WIDTH - textSize.x) / 2;
    u32 yPos = 3 * (HEIGHT / 4.0f) + (HEIGHT / 4.0f - textSize.y) / 2;

    DrawText(msg, xPos, yPos, 20, fg);
}
