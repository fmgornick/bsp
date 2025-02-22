#include "bsp_s2.h"
#include "bsp_stages.h"
#include "bsp_utils.h"
#include "raylib.h"
#include <limits.h>

BSP_Stage
S2_Init(IVector2 *polygon, usize numVertices, S2_Scene *scene)
{
    /* we want to resize the polygon from stage one to fit our stage 2 split screen */
    u32 xMin = UINT_MAX, yMin = UINT_MAX, xMax = 0, yMax = 0;
    for (usize i = 0; i < numVertices; i++)
    {
        if (polygon[i].x < xMin)
            xMin = polygon[i].x;
        if (polygon[i].x > xMax)
            xMax = polygon[i].x;

        if (polygon[i].y < yMin)
            yMin = polygon[i].y;
        if (polygon[i].y > yMax)
            yMax = polygon[i].y;
    }

    f64 scale = MIN((f64)WIDTH / (2 * (xMax - xMin)), (f64)HEIGHT / (yMax - yMin)) * 0.9;
    f64 xPadding = (WIDTH / 2.0f - (xMax - xMin) * scale) / 2.0f;
    f64 yPadding = (HEIGHT - (yMax - yMin) * scale) / 2.0f;

    scene->numVertices = numVertices;
    for (usize i = 0; i < numVertices; i++)
        scene->polygon[i] = (Vector2){ scale * (polygon[i].x - xMin) + xPadding, scale * (polygon[i].y - yMin) + yPadding };

    return S2_PENDING;
}

BSP_Stage
S2_Render(S2_Scene *scene)
{
    BeginDrawing();
    ClearBackground(WHITE);
    S2_DrawPolygon(scene);
    EndDrawing();

    return S2_PENDING;
}

BSP_Stage
S2_RenderFailure(S2_Scene *scene)
{
    return S2_FAILED;
}

void
S2_DrawPolygon(S2_Scene *scene)
{
    for (usize i = 0; i < scene->numVertices; i++)
    {
        u32 j = (i + 1) % scene->numVertices;
        Vector2 p = { scene->polygon[i].x, scene->polygon[i].y };
        Vector2 q = { scene->polygon[j].x, scene->polygon[j].y };
        DrawLineEx(p, q, 2.0f, BLACK);
    }
}

void
S2_DrawMessage(char *msg, Color fg, Color bg)
{
    DrawRectangle(0, 3 * (HEIGHT / 4), WIDTH, HEIGHT / 4, Fade(bg, 0.5));

    Vector2 textSize = MeasureTextEx(GetFontDefault(), msg, 20, 1.0f);
    u32 xPos = (WIDTH - textSize.x) / 2;
    u32 yPos = 3 * (HEIGHT / 4.0f) + (HEIGHT / 4.0f - textSize.y) / 2;

    DrawText(msg, xPos, yPos, 20, fg);
}
