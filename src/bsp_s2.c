#include "bsp_s2.h"
#include "bsp_stages.h"
#include "bsp_utils.h"
#include "raylib.h"
#include <stdint.h>

BSP_Stage
S2_Init(IVector2 *polygon, uint32_t numVertices, S2_Scene *scene)
{
    scene->numVertices = numVertices;
    for (uint32_t i = 0; i < numVertices; i++)
        scene->polygon[i] = (Vector2){ polygon[i].x, polygon[i].y };

    scene->polygonScreen = LoadRenderTexture(WIDTH / 2, HEIGHT);
    scene->treeScreen = LoadRenderTexture(WIDTH / 2, HEIGHT);

    scene->polygonPosition = (Vector2){ 0.0f, 0.0f };
    scene->treePosition = (Vector2){ WIDTH / 2.0f, 0.0f };
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
    for (uint32_t i = 0; i < scene->numVertices - 1; i++)
    {
        Vector2 p = { scene->polygon[i].x, scene->polygon[i].y };
        Vector2 q = { scene->polygon[i + 1].x, scene->polygon[i + 1].y };
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
