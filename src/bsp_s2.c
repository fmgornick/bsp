#include "bsp_s2.h"
#include "bsp_segment.h"
#include "bsp_stages.h"
#include "bsp_tree.h"
#include "bsp_utils.h"
#include "raylib.h"
#include <stdlib.h>

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
    scene->tree = BuildBspTree(scene->segments, scene->numSegments, treeRegion);

    return S2_PENDING;
}

BSP_Stage
S2_Render(S2_Scene *scene)
{
    BeginDrawing();
    ClearBackground(WHITE);
    DrawTree(scene->tree);
    DrawSegments(scene->segments, scene->numSegments);
    EndDrawing();
    return S2_PENDING;
}

BSP_Stage
S2_RenderFailure(S2_Scene *scene)
{
    return S2_FAILED;
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

void
S2_Free(S2_Scene *scene)
{
    free(scene->segments);
    FreeBspTree(scene->tree);
}
