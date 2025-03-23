#include "s2.h"
#include "bsp.h"
#include "bsp_tree.h"
#include "raylib.h"
#include "region.h"
#include "segment.h"
#include "triangulation.h"

static usize numColors = 18;
static Color colors[18] = {
    YELLOW,  GOLD, ORANGE,   PINK,   RED,    MAROON,     GREEN, LIME,  DARKGREEN, //
    SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN, //
};

BspStage
S2_Init(IVector2 *polygon, usize numVertices, S2 *scene)
{
    BoundingRegion segmentsRegion = {
        .left = 0,
        .right = WIDTH / 2,
        .top = 0,
        .bottom = HEIGHT,
    };
    BoundingRegion treeRegion = {
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

BspStage
S2_Render(S2 *scene)
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
    else DrawRegion(scene->tree->activeRegion);

    /* { */
    /*     Vector2 v1 = { .x = 640.0f, .y = 100.0f }; */
    /*     Vector2 v2 = { .x = 320.0f, .y = 600.0f }; */
    /*     Vector2 v3 = { .x = 960.0f, .y = 600.0f }; */
    /*     DrawTriangle(v1, v2, v3, Fade(BLUE, 0.5f)); */
    /* } */

    EndDrawing();

    return S2_PENDING;
}

BspStage
S2_RenderFailure(S2 *scene)
{
    return S2_FAILED;
}

void
S2_Free(S2 *scene)
{
    FreeSegments(scene->segments);
    FreeBspTreeMeta(scene->tree);
}

void
S2_DrawAllBspRegions(S2 *scene)
{
    BspTreeMetaSetActive(scene->tree, scene->tree->size);
    for (usize i = 0; i < scene->tree->size; i++)
    {
        BspNodeMeta meta = scene->tree->meta[i];
        if (IsLeaf(meta.node))
        {
            for (usize j = 0; j < meta.region->triangulationSize; j++)
            {
                Triangle t = meta.region->triangulation[j];
                DrawTriangle(t.v1, t.v2, t.v3, Fade(colors[i % numColors], 0.5f));
            }
        }
    }
    for (usize i = 0; i < scene->numSegments; i++)
        DrawSegment(scene->segments[i], 5.0f, BLACK, false);
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
