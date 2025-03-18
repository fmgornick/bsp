#include "bsp_tree.h"
#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_utils.h"
#include "f64_vector.h"
#include "raylib.h"
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

BspTreeMeta *
BuildBspTreeMeta(Segment *segments, usize len, Region region)
{
    BspTreeMeta *tree = (BspTreeMeta *)malloc(sizeof(BspTreeMeta));

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
        tree->active = NULL;
        tree->activeRegion = NULL;
        tree->activeIdx = tree->size;
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

    { /* calculate node positions and radius for displaying BSP tree nodes to user */
        u32 width = (region.right - region.left), x0 = region.left;
        u32 height = (region.bottom - region.top), y0 = region.top;
        u32 shorter = min(width, height);
        tree->nodeRadius = (f32)shorter / max(tree->height * 2, tree->size + 1);

        for (usize i = 0; i < tree->size; i++)
        {
            f32 x = tree->nodeRadius + ((f32)i / (tree->size + 1)) * width + x0;
            f32 y = tree->nodeRadius + tree->meta[i].depth * ((f32)height / tree->height) + y0;
            tree->meta[i].pos = (Vector2){ x, y };
        }
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

    BuildTreeMetaRegions(tree, tree->rootIdx);
    return tree;
}

void
FreeBspTreeMeta(BspTreeMeta *tree)
{
    for (usize i = 0; i < tree->size; i++)
        FreeBspRegion(tree->meta[i].region);
    FreeBspTree(tree->root);
    free(tree->meta);
    free(tree);
}

void
BuildTreeMetaRegions(BspTreeMeta *tree, usize idx)
{
    BspNode *node = bspNode(tree, idx);
    if (node)
    {
        if (node == tree->root) tree->meta[idx].region = BuildBspRegion(WIDTH / 2, HEIGHT, node->segments[0]);
        else
        {
            BspNode *parent = bspNode(tree, idxParent(tree, idx));
            BspRegion *parentRegion = tree->meta[idxParent(tree, idx)].region;
            {
                if (node == parent->left) tree->meta[idx].region = NewBspRegion(parentRegion, node->segments, node->numSegments, SplitLeft);
                else if (node == parent->right) tree->meta[idx].region = NewBspRegion(parentRegion, node->segments, node->numSegments, SplitRight);
            }
        }
        BuildTreeMetaRegions(tree, idxLeft(tree, idx));
        BuildTreeMetaRegions(tree, idxRight(tree, idx));
    }
}

void
DrawBspTreeMeta(BspTreeMeta *tree)
{
    for (usize i = 0; i < tree->size; i++)
    {
        Vector2 node = tree->meta[i].pos;
        if (tree->meta[i].node->left)
        {
            Vector2 left = tree->meta[idxLeft(tree, i)].pos;
            DrawLineEx(node, left, 2.0f, BLACK);
        }
        if (tree->meta[i].node->right)
        {
            Vector2 right = tree->meta[idxRight(tree, i)].pos;
            DrawLineEx(node, right, 2.0f, BLACK);
        }
    }
    for (usize i = 0; i < tree->size; i++)
    {
        Vector2 node = tree->meta[i].pos;
        if (i == tree->activeIdx) DrawCircle(node.x, node.y, tree->nodeRadius, RED);
        else DrawCircle(node.x, node.y, tree->nodeRadius, BLUE);
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
    if (tree->active && tree->active->left)
    {
        usize idx = idxLeft(tree, tree->activeIdx);
        tree->active = tree->active->left;
        tree->activeRegion = tree->meta[idx].region;
        tree->activeIdx = idx;
    }
}

void
BspTreeMetaMoveRight(BspTreeMeta *tree)
{
    if (tree->active && tree->active->right)
    {
        usize idx = idxRight(tree, tree->activeIdx);
        tree->active = tree->active->right;
        tree->activeRegion = tree->meta[idx].region;
        tree->activeIdx = idx;
    }
}

void
BspTreeMetaMoveUp(BspTreeMeta *tree)
{
    if (tree->active && tree->active->parent)
    {
        usize idx = idxParent(tree, tree->activeIdx);
        tree->active = tree->active->parent;
        tree->activeRegion = tree->meta[idx].region;
        tree->activeIdx = idx;
    }
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
