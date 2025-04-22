#include "s2.h"
#include "bsp.h"
#include "bsp_tree.h"
#include "f64_segment.h"
#include "raylib.h"
#include "region.h"
#include "triangulation.h"

/* ********** helpers ********** */
/* ***************************** */
void DrawAllBspRegions(S2 *scene);
void BspTreeStepForward(S2 *scene);
void BspTreeStepBack(S2 *scene);
void BspTreeFastForward(S2 *scene);
void BspTreeRewind(S2 *scene);
/* ***************************** */
/* ***************************** */

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
    scene->building = false;
    scene->buildTreeDt = 0.0f;
    scene->treeBuilt = false;
    scene->drawAllRegions = false;
    scene->initialized = true;

    return S2_PENDING;
}

BspStage
S2_Render(S2 *scene)
{
    if (!scene->treeBuilt)
    {
        if (IsKeyPressed(KEY_SPACE)) scene->building ^= true;
        if (IsKeyPressed(KEY_RIGHT)) BspTreeStepForward(scene);
        if (IsKeyPressed(KEY_LEFT)) BspTreeStepBack(scene);
        if (IsKeyPressed(KEY_C)) BspTreeFastForward(scene);
        if (IsKeyPressed(KEY_R)) BspTreeRewind(scene);
        if (scene->building)
        {
            scene->buildTreeDt += GetFrameTime();
            if (scene->buildTreeDt >= 0.3f)
            {
                BspTreeStepForward(scene);
                scene->buildTreeDt = 0.0f;
                scene->building = true;
            }
        }
        if (IsKeyPressed(KEY_ENTER))
        {
            BspTreeFastForward(scene);
            scene->treeBuilt = true;
            scene->drawAllRegions = true;
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER)) return S2_COMPLETED;
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
        if (IsKeyPressed(KEY_F)) scene->drawAllRegions ^= true;
    }

    BeginDrawing();
    ClearBackground(WHITE);
    DrawSegments(scene->segments, scene->numSegments);
    DrawBspTreeMeta(scene->tree);
    if (scene->drawAllRegions) DrawAllBspRegions(scene);
    else DrawRegion(scene->tree->activeRegion);
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
    scene->initialized = false;
}

void
DrawAllBspRegions(S2 *scene)
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
BspTreeStepForward(S2 *scene)
{
    scene->building = false;
    BspTreeMeta *tree = scene->tree;
    if (tree->activeIdx == tree->size - 1) return;
    if (tree->active->left && !tree->meta[idxLeft(tree, tree->activeIdx)].visible)
    {
        BspTreeMetaMoveLeft(tree);
        tree->meta[tree->activeIdx].visible = true;
        tree->visibleSize += 1;
        tree->visibleHeight = max(tree->meta[tree->activeIdx].depth + 1, tree->visibleHeight);
        UpdateBspTreeMeta(tree);
    }
    else if (tree->active->right && !tree->meta[idxRight(tree, tree->activeIdx)].visible)
    {
        BspTreeMetaMoveRight(tree);
        tree->meta[tree->activeIdx].visible = true;
        tree->visibleSize += 1;
        tree->visibleHeight = max(tree->meta[tree->activeIdx].depth + 1, tree->visibleHeight);
        UpdateBspTreeMeta(tree);
    }
    else if (tree->active->parent) BspTreeMetaMoveUp(tree);
}

void
BspTreeStepBack(S2 *scene)
{
    scene->building = false;
    BspTreeMeta *tree = scene->tree;
    if (tree->active == tree->root && !tree->meta[idxLeft(tree, tree->activeIdx)].visible) return;
    else if (tree->active->right && tree->meta[idxRight(tree, tree->activeIdx)].visible) BspTreeMetaMoveRight(tree);
    else if (tree->active->left && tree->meta[idxLeft(tree, tree->activeIdx)].visible) BspTreeMetaMoveLeft(tree);
    else
    {
        tree->meta[tree->activeIdx].visible = false;
        tree->visibleSize -= 1;
        bool blah = true;
        for (usize i = 0; i < tree->size; i++)
        {
            if (i == tree->activeIdx) continue;
            if (tree->meta[i].visible && tree->meta[i].depth >= tree->meta[tree->activeIdx].depth)
            {
                blah = false;
                break;
            }
        }
        if (blah) tree->visibleHeight -= 1;
        BspTreeMetaMoveUp(tree);
        UpdateBspTreeMeta(tree);
    }
}

void
BspTreeFastForward(S2 *scene)
{
    scene->building = false;
    BspTreeMeta *tree = scene->tree;
    for (usize i = 0; i < tree->size; i++)
        tree->meta[i].visible = true;
    tree->visibleSize = tree->size;
    tree->visibleHeight = tree->height;
    BspTreeMetaSetActive(tree, tree->size - 1);
    UpdateBspTreeMeta(tree);
}

void
BspTreeRewind(S2 *scene)
{
    scene->building = false;
    BspTreeMeta *tree = scene->tree;
    for (usize i = 0; i < tree->size; i++)
        tree->meta[i].visible = false;
    tree->visibleSize = 1;
    tree->visibleHeight = 1;
    tree->meta[tree->rootIdx].visible = true;
    BspTreeMetaSetActive(tree, tree->rootIdx);
    UpdateBspTreeMeta(tree);
}
