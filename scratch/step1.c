#include "bsp_utils.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdint.h>

bool CheckIntersecting (Vector2 p1, Vector2 p2, Vector2 q1, Vector2 q2);

int32_t
main ()
{
    uint32_t width = WIDTH;
    uint32_t height = HEIGHT;

    SetConfigFlags (FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow (width, height, "csci 8442 bsp demo");

    /* track vertices of polygon (we will use these for building our BSP tree) */
    Vector2 vertices[MAX_VERTICES] = { 0 };
    uint32_t num_vertices = 0;

    Rectangle startCell = { 0 };
    Rectangle lastSelected = { 0 };

    while (!WindowShouldClose ())
    {
        /* get new dimensions if user resizes */
        if (IsWindowResized () && !IsWindowFullscreen ())
        {
            width = GetScreenWidth ();
            height = GetScreenHeight ();
        }

        /* end step if user closes simple polygon loop */
        else if (num_vertices >= 3 && CheckCollisionPointRec (vertices[num_vertices - 1], startCell))
            break;

        /* automatically end step if sipmle polygon if no more vertices allowed */
        if (num_vertices == MAX_VERTICES)
            break;

        BeginDrawing ();
        ClearBackground (RAYWHITE);

        /* vertical grid lines */
        for (int i = 0; i < width / GRID_SIZE + 1; i++)
            DrawLineV ((Vector2){ (float)GRID_SIZE * i, 0 }, (Vector2){ (float)GRID_SIZE * i, (float)height }, LIGHTGRAY);

        /* horizontal grid lines */
        for (int i = 0; i < height / GRID_SIZE + 1; i++)
            DrawLineV ((Vector2){ 0, (float)GRID_SIZE * i }, (Vector2){ (float)width, (float)GRID_SIZE * i }, LIGHTGRAY);

        /* shade square of previously selected cell */
        if (lastSelected.width != 0.0)
            DrawRectangleRec (lastSelected, Fade (LIGHTGRAY, 0.3));

        /* shade cell containing current cursor position */
        Vector2 mouse = GetMousePosition ();
        Rectangle currentCell
            = { mouse.x - ((uint32_t)mouse.x % GRID_SIZE), mouse.y - ((uint32_t)mouse.y % GRID_SIZE), GRID_SIZE, GRID_SIZE };
        DrawRectangleRec (currentCell, LIGHTGRAY);

        /* outline of convex polygon made by user so far */
        bool intersecting = false;
        if (num_vertices >= 1)
        {
            Vector2 currentPoint = { currentCell.x + (float)GRID_SIZE / 2, currentCell.y + (float)GRID_SIZE / 2 };

            for (uint32_t i = 0; i < num_vertices - 1; i++)
            {
                if (!intersecting && num_vertices - i > 1)
                    intersecting |= CheckIntersecting (vertices[i], vertices[i + 1], vertices[num_vertices - 1], currentPoint);

                DrawLineEx (vertices[i], vertices[i + 1], 1.5, BLACK);
            }

            /* edge from last vertex to current cursor */
            if (intersecting)
                DrawLineEx (vertices[num_vertices - 1], currentPoint, 1.5, RED);
            else
                DrawLineEx (vertices[num_vertices - 1], currentPoint, 1.5, DARKGRAY);
        }

        /* if theres only one usable vertex left, automatically close polygon loop */
        if (num_vertices == MAX_VERTICES - 1)
            vertices[num_vertices++] = vertices[0];

        else if (IsMouseButtonPressed (MOUSE_BUTTON_LEFT) && num_vertices < MAX_VERTICES - 1)
        {
            /* track starting cell to know if we closed the loop in the future */
            if (num_vertices == 0)
                startCell = currentCell;

            lastSelected = currentCell;
            vertices[num_vertices++] = (Vector2){ lastSelected.x + (float)GRID_SIZE / 2, lastSelected.y + (float)GRID_SIZE / 2 };
        }

        EndDrawing ();
    }

    return 0;
}

bool
CheckIntersecting (Vector2 p1, Vector2 p2, Vector2 q1, Vector2 q2)
{
    Vector2 p = Vector2Subtract (p2, p1);
    Vector2 q = Vector2Subtract (q2, q1);
    Vector2 pq1 = Vector2Subtract (q1, p1);
    Vector2 pq2 = Vector2Subtract (q2, p1);

    return !SAME_SIGN (p.x * pq1.y - p.y * pq1.y, p.x * pq2.y - p.y * pq2.y)
           && !SAME_SIGN (q.x * pq1.y - q.y * pq1.y, q.x * pq2.y - q.y * pq2.y);
}
