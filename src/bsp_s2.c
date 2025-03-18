#include "bsp_s2.h"
#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_stages.h"
#include "bsp_tree.h"
#include "bsp_utils.h"
#include "raylib.h"

BSP_Stage
S2_Init(IVector2 *polygon, usize numVertices, S2_Scene *scene)
{
    Region segmentsRegion = {
        .left = 0,
        .right = WIDTH / 2,
        .top = 0,
        .bottom = HEIGHT,
    };
    Region treeRegion = {
        .left = WIDTH / 2,
        .right = WIDTH,
        .top = 0,
        .bottom = HEIGHT,
    };
    scene->segments = BuildSegments(polygon, numVertices, segmentsRegion, &scene->numSegments);
    scene->tree = BuildBspTreeMeta(scene->segments, scene->numSegments, treeRegion);
    scene->drawAllRegions = false;

    return S2_PENDING;
}

BSP_Stage
S2_Render(S2_Scene *scene)
{
    if (IsKeyPressed(KEY_LEFT)) BspTreeMetaMoveLeft(scene->tree);
    if (IsKeyPressed(KEY_RIGHT)) BspTreeMetaMoveRight(scene->tree);
    if (IsKeyPressed(KEY_UP)) BspTreeMetaMoveUp(scene->tree);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 pos = GetMousePosition();
        f32 radius = scene->tree->nodeRadius;
        usize i = 0;
        for (; i < scene->tree->size; i++)
            if (CheckCollisionPointCircle(pos, scene->tree->meta[i].pos, radius)) break;
        BspTreeMetaSetActive(scene->tree, i);
        scene->drawAllRegions = false;
    }
    if (IsKeyPressed(KEY_F)) scene->drawAllRegions = true ^ scene->drawAllRegions;

    BeginDrawing();
    ClearBackground(WHITE);
    DrawSegments(scene->segments, scene->numSegments);
    DrawBspTreeMeta(scene->tree);
    if (scene->drawAllRegions) S2_DrawAllBspRegions(scene);
    else DrawBspRegion(scene->tree->activeRegion);
    EndDrawing();

    return S2_PENDING;
}

BSP_Stage
S2_RenderFailure(S2_Scene *scene)
{
    return S2_FAILED;
}

void
S2_Free(S2_Scene *scene)
{
    FreeSegments(scene->segments);
    FreeBspTreeMeta(scene->tree);
}

void
S2_DrawAllBspRegions(S2_Scene *scene)
{
    BspTreeMetaSetActive(scene->tree, scene->tree->size);
    for (usize i = 0; i < scene->tree->size; i++)
    {
        BspNodeMeta meta = scene->tree->meta[i];
        if (IsLeaf(meta.node))
        {
            for (usize i = 0; i < meta.region->boundarySize; i++)
                DrawSegment(meta.region->boundary[i], 5.0f, BLACK, false);
            DrawSegment(meta.region->line, 5.0f, BLACK, false);
        }
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
