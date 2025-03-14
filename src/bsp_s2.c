#include "bsp_s2.h"
#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_stages.h"
#include "bsp_tree.h"
#include "bsp_utils.h"
#include "raylib.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

BSP_Stage
S2_Init(IVector2 *polygon, usize numVertices, S2_Scene *scene)
{
    Region segmentsRegion = {
        .left = 0,
        .right = WIDTH / 2,
        .top = 0,
        .bottom = HEIGHT,
    };

    scene->segments = BuildSegments(polygon, numVertices, segmentsRegion, &scene->numSegments);
    BuildTree(scene);
    /* scene->tree = BuildBspTree(scene->segments, scene->numSegments, treeRegion); */
    /* UpdateTree(scene->tree); */

    /* scene->region = InitRegion(WIDTH, HEIGHT); */
    /* AddSegment(scene->region, scene->tree->root->segments[0], SplitLeft); */

    return S2_PENDING;
}

BSP_Stage
S2_Render(S2_Scene *scene)
{
    if (IsKeyPressed(KEY_LEFT))
        if (scene->tree->active->left) scene->tree->active = scene->tree->active->left;

    if (IsKeyPressed(KEY_RIGHT))
        if (scene->tree->active->right) scene->tree->active = scene->tree->active->right;

    if (IsKeyPressed(KEY_UP))
        if (scene->tree->active->parent) scene->tree->active = scene->tree->active->parent;

    BeginDrawing();
    ClearBackground(WHITE);
    DrawSegments(scene->segments, scene->numSegments);
    DrawBspTree(scene);
    DrawRegion(scene->nodeMeta[scene->activeIdx].region);
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

void
BuildTree(S2_Scene *scene)
{
    BspNode *root;
    { /* create actual bsp tree */
        Segment *segmentsCopy = (Segment *)malloc(scene->numSegments * sizeof(Segment));
        memcpy(segmentsCopy, scene->segments, scene->numSegments * sizeof(Segment));
        root = BuildBspNode(segmentsCopy, scene->numSegments, NULL);
    }

    { /* calculate size of tree */
        BspNode *node = MinNode(root);
        while (node)
        {
            scene->treeSize += 1;
            node = SuccNode(node);
        }
    }

    { /* add each node to array for easy indexing (also store node depth) */
        scene->nodeMeta = (BspNodeMeta *)malloc(scene->treeSize * sizeof(BspNodeMeta));
        BspNode *node = MinNode(root);
        for (usize i = 0; i < scene->treeSize; i++)
        {
            if (node == root)
            {
                scene->rootIdx = i;
                scene->activeIdx = i;
            }
            scene->nodeMeta[i].node = node;
            scene->nodeMeta[i].depth = 0;
            BspNode *tmp = node->parent;
            while (tmp)
            {
                scene->nodeMeta[i].depth += 1;
                tmp = tmp->parent;
            }
            node = SuccNode(node);
        }
    }

    { /* calculate height of tree */
        usize maxDepth = 0;
        for (usize i = 0; i < scene->treeSize; i++)
            maxDepth = max(maxDepth, scene->nodeMeta[i].depth);
        scene->treeHeight = maxDepth + 1;
    }

    { /* calculate node positions and radius for displaying BSP tree nodes to user */
        u32 width = WIDTH / 2, x0 = WIDTH / 2;
        u32 height = HEIGHT, y0 = 0;
        u32 shorter = min(width, height);
        scene->nodeRadius = (f32)shorter / max(scene->treeHeight * 2, scene->treeSize);

        BspNode *node = MinNode(root);
        for (usize i = 0; i < scene->treeSize; i++)
        {
            f32 x = ((f32)i / scene->treeSize) * width + x0;
            f32 y = scene->nodeRadius + node->depth * ((f32)height / scene->treeHeight) + y0;
            scene->nodeMeta[i].pos = (Vector2){ x, y };
        }
    }

    { /* find indexes of left/right/parent nodes for quick metadata navigation */
        for (usize i = 0; i < scene->treeSize; i++)
        {
            if (scene->nodeMeta[i].node->left)
            {
                for (usize j = 0; j < i; j++)
                    if (scene->nodeMeta[i].node->left == scene->nodeMeta[j].node)
                    {
                        scene->nodeMeta[i].left = j;
                        scene->nodeMeta[j].parent = i;
                    }
                assert(scene->nodeMeta[i].node->left == scene->nodeMeta[scene->nodeMeta[i].left].node);
                assert(scene->nodeMeta[scene->nodeMeta[i].left].node->parent == scene->nodeMeta[i].node);
            }
            if (scene->nodeMeta[i].node->right)
            {
                for (usize j = i + 1; j < scene->treeSize; j++)
                    if (scene->nodeMeta[i].node->right == scene->nodeMeta[j].node)
                    {
                        scene->nodeMeta[i].right = j;
                        scene->nodeMeta[j].parent = i;
                    }
                assert(scene->nodeMeta[i].node->right == scene->nodeMeta[scene->nodeMeta[i].right].node);
                assert(scene->nodeMeta[scene->nodeMeta[i].right].node->parent == scene->nodeMeta[i].node);
            }
        }
    }

    BuildTreeRegions(scene, scene->rootIdx);
}

void
BuildTreeRegions(S2_Scene *scene, usize idx)
{
    if (idx == scene->rootIdx) scene->nodeMeta[idx].region = BuildRegion(WIDTH / 2, HEIGHT, scene->nodeMeta[idx].node->segments[0]);
    else
    {
        BspNodeMeta parent = scene->nodeMeta[scene->nodeMeta[idx].parent];
        if (scene->nodeMeta[idx].node == parent.node->left)
            scene->nodeMeta[idx].region = NewRegion(parent.region, scene->nodeMeta[idx].node->segments, scene->nodeMeta[idx].node->numSegments, SplitLeft);
        if (scene->nodeMeta[idx].node == parent.node->right)
            scene->nodeMeta[idx].region = NewRegion(parent.region, scene->nodeMeta[idx].node->segments, scene->nodeMeta[idx].node->numSegments, SplitRight);
    }
    if (scene->nodeMeta[idx].node->left) BuildTreeRegions(scene, scene->nodeMeta[idx].left);
    if (scene->nodeMeta[idx].node->right) BuildTreeRegions(scene, scene->nodeMeta[idx].right);
}

void
DrawBspTree(S2_Scene *scene)
{
    for (usize i = 0; i < scene->treeSize; i++)
    {
        Vector2 node = scene->nodeMeta[i].pos;
        if (scene->nodeMeta[i].node->left)
        {
            Vector2 left = scene->nodeMeta[scene->nodeMeta[i].left].pos;
            DrawLineEx(node, left, 2.0f, BLACK);
        }
        if (scene->nodeMeta[i].node->right)
        {
            Vector2 right = scene->nodeMeta[scene->nodeMeta[i].right].pos;
            DrawLineEx(node, right, 2.0f, BLACK);
        }
        if (i == scene->activeIdx) DrawCircle(node.x, node.y, scene->nodeRadius, RED);
        else DrawCircle(node.x, node.y, scene->nodeRadius, BLUE);
    }
}
