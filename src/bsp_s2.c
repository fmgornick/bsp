#include "bsp_s2.h"
#include "bsp_stages.h"
#include "bsp_utils.h"
#include "raylib.h"
#include <limits.h>
#include <stdint.h>

BSP_Stage
S2_Init(IVector2 *polygon, uint32_t numVertices, S2_Scene *scene)
{
    /* we want to resize the polygon from stage one to fit our stage 2 split screen */
    uint32_t xMin = UINT_MAX, yMin = UINT_MAX, xMax = 0, yMax = 0;
    for (uint32_t i = 0; i < numVertices; i++)
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

    /* TODO: add padding so polygon doesn't touch the edge */
    double scale = MIN((double)WIDTH / (2 * (xMax - xMin)), (double)HEIGHT / (yMax - yMin));
    double xPadding = (((double)WIDTH / 2) - (xMax - xMin) * scale) / 2;
    double yPadding = (((double)HEIGHT) - (yMax - yMin) * scale) / 2;

    scene->numVertices = numVertices;
    for (uint32_t i = 0; i < numVertices; i++)
        scene->polygon[i] = (Vector2){ (scale * (polygon[i].x - xMin)) + xPadding, (scale * (polygon[i].y - yMin)) + yPadding };

    scene->polygonScreen = LoadRenderTexture(WIDTH / 2, HEIGHT);
    scene->treeScreen = LoadRenderTexture(WIDTH / 2, HEIGHT);

    scene->polygonPosition = (Vector2){ 0.0f, 0.0f };
    scene->treePosition = (Vector2){ WIDTH / 2.0f, 0.0f };

    /* need to build a flipped rectangle for the split view */
    scene->splitScreenRect = (Rectangle){ 0.0f, 0.0f, (float)WIDTH, (float)-HEIGHT };

    return S2_PENDING;
}

BSP_Stage
S2_Render(S2_Scene *scene)
{
    BeginTextureMode(scene->polygonScreen);
    ClearBackground(RAYWHITE);
    S2_DrawPolygon(scene);
    EndTextureMode();

    BeginTextureMode(scene->treeScreen);
    ClearBackground(RAYWHITE);
    EndTextureMode();

    BeginDrawing();
    DrawTextureRec(scene->polygonScreen.texture, scene->splitScreenRect, scene->polygonPosition, WHITE);
    DrawTextureRec(scene->treeScreen.texture, scene->splitScreenRect, scene->treePosition, WHITE);
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
    for (uint32_t i = 0; i < scene->numVertices; i++)
    {
        uint32_t j = (i + 1) % scene->numVertices;
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
    uint32_t xPos = (WIDTH - textSize.x) / 2;
    uint32_t yPos = 3 * (HEIGHT / 4.0f) + (HEIGHT / 4.0f - textSize.y) / 2;

    DrawText(msg, xPos, yPos, 20, fg);
}
