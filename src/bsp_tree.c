#include "bsp_tree.h"
#include "bsp.h"
#include "f64_vector.h"
#include "raylib.h"
#include "segment.h"
#include "triangulation.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BspNode *
BuildBspTree(Segment *segments, usize len, BspNode *parent)
{
    BspNode *node = (BspNode *)malloc(sizeof(BspNode));
    node->left = NULL;
    node->right = NULL;
    node->parent = parent ? parent : NULL;

    if (len <= 1)
    {
        node->segments = segments;
        node->numSegments = len;
    }
    else
    {
        /* use free split as partitioning segment if one exists */
        usize splitIdx = 0;
        for (usize i = 0; i < len; i++)
            if (segments[i].splitLeft && segments[i].splitRight)
            {
                splitIdx = i;
                break;
            }

        /* first pass - find size needed for sub-trees */
        usize numBehind = 0, numInFront = 0, numInside = 0;
        for (usize i = 0; i < len; i++)
        {
            switch (SegmentSides(segments[splitIdx], segments[i]))
            {
            case SideInside:
                numInside += 1;
                break;
            case SideLeft:
                numInFront += 1;
                break;
            case SideRight:
                numBehind += 1;
                break;
            case SideBoth:
                numInFront += 1;
                numBehind += 1;
                break;
            }
        }

        assert(numInside >= 1);
        node->segments = (Segment *)malloc(numInside * sizeof(Segment));
        node->numSegments = numInside;

        Segment *segmentsBehind = NULL;
        Segment *segmentsInFront = NULL;
        if (numBehind > 0) segmentsBehind = (Segment *)malloc(numBehind * sizeof(Segment));
        if (numInFront > 0) segmentsInFront = (Segment *)malloc(numInFront * sizeof(Segment));
        usize behindIdx = 0, inFrontIdx = 0, insideIdx = 0;
        /* second pass - add segments to middle node and recurse into left/right children */
        for (usize i = 0; i < len; i++)
        {
            switch (SegmentSides(segments[splitIdx], segments[i]))
            {
            /* both endpoints inside split segment => add si to current node segment list */
            case SideInside:
                node->segments[insideIdx++] = segments[i];
                break;

            /* both endpoints in front of split segment => add si to right segment list */
            case SideLeft:
                segmentsInFront[inFrontIdx++] = segments[i];
                break;

            /* both endpoints behind split segment => add si to left segment list */
            case SideRight:
                segmentsBehind[behindIdx++] = segments[i];
                break;

            /*
             * current segmenst is bisected by splitting line =>
             *  - find intersection point of segment si with splitting line
             *  - insert split subsegments into respective left/right segment lists
             *  - update bools for each subsegment for "free split" check in recursive call
             */
            case SideBoth: {
                segmentsBehind[behindIdx] = segments[i];
                segmentsInFront[inFrontIdx] = segments[i];
                DVector2 intersection = SegmentIntersection(segments[splitIdx], segments[i]);

                if (SegmentSide(segments[splitIdx], segments[i].left) == SideRight)
                {
                    segmentsBehind[behindIdx].splitRight = true;
                    segmentsInFront[inFrontIdx].splitLeft = true;
                    segmentsBehind[behindIdx].right = intersection;
                    segmentsInFront[inFrontIdx].left = intersection;
                }
                else
                {
                    segmentsBehind[behindIdx].splitLeft = true;
                    segmentsInFront[inFrontIdx].splitRight = true;
                    segmentsBehind[behindIdx].left = intersection;
                    segmentsInFront[inFrontIdx].right = intersection;
                }

                behindIdx += 1;
                inFrontIdx += 1;
            }
            break;
            }
        }

        node->left = BuildBspTree(segmentsBehind, numBehind, node);
        node->right = BuildBspTree(segmentsInFront, numInFront, node);
        free(segments);
    }

    return node;
}

void
FreeBspTree(BspNode *node)
{
    if (node->segments) free(node->segments);
    if (node->left) FreeBspTree(node->left);
    if (node->right) FreeBspTree(node->right);
    free(node);
}

void
CopyBspTree(const BspTreeMeta *src, BspTreeMeta *dst)
{
    dst->height = src->height;
    dst->size = src->size;
    dst->meta = (BspNodeMeta *)malloc(dst->size * sizeof(BspNodeMeta));
    for (usize i = 0; i < dst->size; i++)
    {
        dst->meta[i].node = (BspNode *)malloc(sizeof(BspNode));
        dst->meta[i].node->numSegments = src->meta[i].node->numSegments;
        dst->meta[i].node->segments = (Segment *)malloc(dst->meta[i].node->numSegments * sizeof(Segment));
        dst->meta[i].region = (Region *)malloc(sizeof(Region));
        dst->meta[i].region->boundarySize = src->meta[i].region->boundarySize;
        dst->meta[i].region->triangulationSize = src->meta[i].region->triangulationSize;
        dst->meta[i].region->boundary = (Segment *)malloc(dst->meta[i].region->boundarySize * sizeof(Segment));
        dst->meta[i].region->triangulation = (Triangle *)malloc(dst->meta[i].region->triangulationSize * sizeof(Triangle));
        dst->meta[i].region->line = src->meta[i].region->line;
        dst->meta[i].region->hasLine = src->meta[i].region->hasLine;
        dst->meta[i].region->leftIdx = src->meta[i].region->leftIdx;
        dst->meta[i].region->rightIdx = src->meta[i].region->rightIdx;
        for (usize j = 0; j < dst->meta[i].region->boundarySize; j++)
            dst->meta[i].region->boundary[j] = src->meta[i].region->boundary[j];
        for (usize j = 0; j < dst->meta[i].region->triangulationSize; j++)
            dst->meta[i].region->triangulation[j] = src->meta[i].region->triangulation[j];
        dst->meta[i].depth = src->meta[i].depth;
        dst->meta[i].pos = src->meta[i].pos;
        dst->meta[i].left = src->meta[i].left;
        dst->meta[i].right = src->meta[i].right;
        dst->meta[i].parent = src->meta[i].parent;
        dst->meta[i].visible = src->meta[i].visible;
    }
    for (usize i = 0; i < dst->size; i++)
    {
        for (usize j = 0; j < dst->meta[i].node->numSegments; j++)
            dst->meta[i].node->segments[j] = src->meta[i].node->segments[j];
        if (dst->meta[i].left < dst->size) dst->meta[i].node->left = dst->meta[dst->meta[i].left].node;
        else dst->meta[i].node->left = NULL;
        if (dst->meta[i].right < dst->size) dst->meta[i].node->right = dst->meta[dst->meta[i].right].node;
        else dst->meta[i].node->right = NULL;
        if (dst->meta[i].parent < dst->size) dst->meta[i].node->parent = dst->meta[dst->meta[i].parent].node;
        else dst->meta[i].node->parent = NULL;
    }
    dst->rootIdx = src->rootIdx;
    dst->activeIdx = src->activeIdx;
    dst->root = dst->meta[dst->rootIdx].node;
    if (dst->activeIdx < dst->size)
    {
        dst->active = dst->meta[dst->activeIdx].node;
        dst->activeRegion = dst->meta[dst->activeIdx].region;
    }
    else
    {
        dst->active = NULL;
        dst->activeRegion = NULL;
    }
    dst->nodeRadius = src->nodeRadius;
    dst->bounds = src->bounds;
    dst->visibleSize = src->visibleSize;
    dst->visibleHeight = src->visibleHeight;
}

BspTreeMeta *
BuildBspTreeMeta(Segment *segments, usize len, BoundingRegion region)
{
    BspTreeMeta *tree = (BspTreeMeta *)malloc(sizeof(BspTreeMeta));
    tree->bounds = region;

    { /* create actual bsp tree */
        Segment *segmentsCopy = (Segment *)malloc(len * sizeof(Segment));
        memcpy(segmentsCopy, segments, len * sizeof(Segment));
        tree->root = BuildBspTree(segmentsCopy, len, NULL);
    }

    { /* calculate size of tree */
        tree->size = 0;
        BspNode *node = MinNode(tree->root);
        while (node)
        {
            tree->size += 1;
            node = SuccNode(node);
        }
    }

    { /* add each node to array for easy indexing (also store node depth) */
        tree->meta = (BspNodeMeta *)malloc(tree->size * sizeof(BspNodeMeta));
        BspNode *node = MinNode(tree->root);
        for (usize i = 0; i < tree->size; i++)
        {
            tree->meta[i].node = node;
            if (node == tree->root)
            {
                tree->rootIdx = i;
                tree->meta[i].depth = 0;
            }
            else
            {
                tree->meta[i].depth = 0;
                tree->meta[i].visible = false;
                for (BspNode *tmp = node; (tmp->parent != NULL); tmp = tmp->parent)
                    tree->meta[i].depth += 1;
            }
            node = SuccNode(node);
        }
    }

    { /* calculate height of tree */
        usize maxDepth = 0;
        for (usize i = 0; i < tree->size; i++)
            maxDepth = max(maxDepth, tree->meta[i].depth);
        tree->height = maxDepth + 1;
    }

    { /* find indexes of left/right/parent nodes for quick metadata navigation */
        for (usize i = 0; i < tree->size; i++)
        {
            if (tree->meta[i].node->left)
            {
                for (usize j = 0; j < i; j++)
                    if (tree->meta[i].node->left == tree->meta[j].node)
                    {
                        tree->meta[i].left = j;
                        tree->meta[j].parent = i;
                    }
                assert(bspNode(tree, i)->left == bspNode(tree, idxLeft(tree, i)));
                assert(bspNode(tree, idxLeft(tree, i))->parent == bspNode(tree, i));
            }
            if (tree->meta[i].node->right)
            {
                for (usize j = i + 1; j < tree->size; j++)
                    if (tree->meta[i].node->right == tree->meta[j].node)
                    {
                        tree->meta[i].right = j;
                        tree->meta[j].parent = i;
                    }
                assert(bspNode(tree, i)->right == bspNode(tree, idxRight(tree, i)));
                assert(bspNode(tree, idxRight(tree, i))->parent == bspNode(tree, i));
            }
        }
    }

    {
        BuildTreeRegions(tree, tree->rootIdx);
        BspTreeMetaSetActive(tree, tree->rootIdx);
        tree->meta[tree->rootIdx].visible = true;
        tree->visibleSize = 1;
        tree->visibleHeight = 1;
        UpdateBspTreeMeta(tree);
    }

    return tree;
}

void
FreeBspTreeMeta(BspTreeMeta *tree)
{
    for (usize i = 0; i < tree->size; i++)
        FreeRegion(tree->meta[i].region);
    FreeBspTree(tree->root);
    free(tree->meta);
    free(tree);
}

void
BuildTreeRegions(BspTreeMeta *tree, usize idx)
{
    BspNode *node = bspNode(tree, idx);
    if (node)
    {
        if (node == tree->root) tree->meta[idx].region = BuildRegion(WIDTH / 2, HEIGHT, node->segments[0]);
        else
        {
            BspNode *parent = bspNode(tree, idxParent(tree, idx));
            Region *parentRegion = tree->meta[idxParent(tree, idx)].region;
            {
                if (node == parent->left) tree->meta[idx].region = NewRegion(parentRegion, node->segments, node->numSegments, SplitLeft);
                else if (node == parent->right) tree->meta[idx].region = NewRegion(parentRegion, node->segments, node->numSegments, SplitRight);
            }
        }
        BuildTreeRegions(tree, idxLeft(tree, idx));
        BuildTreeRegions(tree, idxRight(tree, idx));
    }
}

void
DrawBspTreeMeta(BspTreeMeta *tree)
{
    for (usize i = 0; i < tree->size; i++)
    {
        BspNodeMeta meta = tree->meta[i];
        if (meta.visible)
        {
            Vector2 node = tree->meta[i].pos;
            if (meta.node->left && tree->meta[meta.left].visible)
            {
                Vector2 left = tree->meta[idxLeft(tree, i)].pos;
                DrawLineEx(node, left, 2.0f, BLACK);
            }
            if (meta.node->right && tree->meta[meta.right].visible)
            {
                Vector2 right = tree->meta[idxRight(tree, i)].pos;
                DrawLineEx(node, right, 2.0f, BLACK);
            }
        }
    }
    for (usize i = 0; i < tree->size; i++)
    {
        if (tree->meta[i].visible)
        {
            Vector2 node = tree->meta[i].pos;
            if (i == tree->activeIdx) DrawCircle(node.x, node.y, tree->nodeRadius, RED);
            else DrawCircle(node.x, node.y, tree->nodeRadius, BLUE);
        }
    }
}

void
UpdateBspTreeMeta(BspTreeMeta *tree)
{
    u32 width = (tree->bounds.right - tree->bounds.left), x0 = tree->bounds.left;
    u32 height = (tree->bounds.bottom - tree->bounds.top), y0 = tree->bounds.top;
    u32 shorter = min(width, height);
    tree->nodeRadius = (f32)shorter / max(tree->visibleHeight * 2, tree->visibleSize + 1);
    usize visibleIdx = 0;
    for (usize i = 0; i < tree->size; i++)
    {
        if (tree->meta[i].visible)
        {
            f32 x = tree->nodeRadius + ((f32)visibleIdx / (tree->visibleSize + 1)) * width + x0;
            f32 y = tree->nodeRadius + tree->meta[i].depth * ((f32)height / tree->visibleHeight) + y0;
            tree->meta[i].pos = (Vector2){ x, y };
            visibleIdx += 1;
        }
    }
}

void
BspTreeMetaSetActive(BspTreeMeta *tree, usize i)
{
    if (i >= 0 && i < tree->size)
    {
        tree->active = tree->meta[i].node;
        tree->activeRegion = tree->meta[i].region;
        tree->activeIdx = i;
    }
    else
    {
        tree->active = NULL;
        tree->activeRegion = NULL;
        tree->activeIdx = tree->size;
    }
}

void
BspTreeMetaMoveLeft(BspTreeMeta *tree)
{
    if (tree->active && tree->active->left) BspTreeMetaSetActive(tree, idxLeft(tree, tree->activeIdx));
}

void
BspTreeMetaMoveRight(BspTreeMeta *tree)
{
    if (tree->active && tree->active->right) BspTreeMetaSetActive(tree, idxRight(tree, tree->activeIdx));
}

void
BspTreeMetaMoveUp(BspTreeMeta *tree)
{
    if (tree->active && tree->active->parent) BspTreeMetaSetActive(tree, idxParent(tree, tree->activeIdx));
}

BspNode *
MinNode(BspNode *node)
{
    if (!node) return NULL;
    else if (!node->left) return node;
    else return MinNode(node->left);
}

BspNode *
MaxNode(BspNode *node)
{
    if (!node) return NULL;
    else if (!node->right) return node;
    else return MaxNode(node->right);
}

BspNode *
PrevNode(BspNode *node)
{
    if (node->left) return MaxNode(node->left);
    BspNode *tmp = node;
    while (tmp->parent && tmp == tmp->parent->left)
        tmp = tmp->parent;
    return tmp->parent;
}

BspNode *
SuccNode(BspNode *node)
{
    if (node->right) return MinNode(node->right);
    BspNode *tmp = node;
    while (tmp->parent && tmp == tmp->parent->right)
        tmp = tmp->parent;
    return tmp->parent;
}

bool
IsLeaf(BspNode *node)
{
    return (!node->left) && (!node->right);
}

BspNode *
bspNode(BspTreeMeta *tree, usize idx)
{
    if (idx < 0 || idx >= tree->size) return NULL;
    return tree->meta[idx].node;
}

usize
idxLeft(BspTreeMeta *tree, usize idx)
{
    if (idx < 0 || idx >= tree->size) return tree->size;
    else if (!bspNode(tree, idx)->left) return tree->size;
    else return tree->meta[idx].left;
}

usize
idxRight(BspTreeMeta *tree, usize idx)
{
    if (idx < 0 || idx >= tree->size) return tree->size;
    else if (!bspNode(tree, idx)->right) return tree->size;
    else return tree->meta[idx].right;
}

usize
idxParent(BspTreeMeta *tree, usize idx)
{
    if (idx < 0 || idx >= tree->size) return tree->size;
    else if (!bspNode(tree, idx)->parent) return tree->size;
    else return tree->meta[idx].parent;
}
