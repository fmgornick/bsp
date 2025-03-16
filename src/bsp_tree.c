#include "bsp_tree.h"
#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_utils.h"
#include "f64_vector.h"
#include "raymath.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

BspNode *
BuildBspTree(Segment *segments, usize len, BspNode *parent)
{
    BspNode *node = (BspNode *)malloc(sizeof(BspNode));
    if (parent) node->parent = parent;
    else node->parent = NULL;

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
            if (segments[i].splitLeft && segments[i].splitRight) splitIdx = i;

        Segment split = segments[splitIdx];
        DVector2 splitVec = DVector2Subtract(split.right, split.left);

        /* first pass - find size needed for sub-trees */
        usize numBehind = 0, numInFront = 0, numInside = 1;
        for (usize i = 1; i < len; i++)
        {
            Segment si = segments[i];
            f64 leftSide = DVector2Determinant(splitVec, DVector2Subtract(si.left, split.left));
            f64 rightSide = DVector2Determinant(splitVec, DVector2Subtract(si.right, split.left));

            if (babs(leftSide) < EPSILON && babs(rightSide) < EPSILON) numInside += 1;
            else if (leftSide - EPSILON < 0 && rightSide - EPSILON < 0) numBehind += 1;
            else if (leftSide + EPSILON > 0 && rightSide + EPSILON > 0) numInFront += 1;
            else
            {
                numBehind += 1;
                numInFront += 1;
            }
        }

        assert(numInside >= 1);
        node->segments = (Segment *)malloc(numInside * sizeof(Segment));
        node->segments[0] = split;
        node->numSegments = numInside;

        Segment *segmentsBehind = NULL;
        Segment *segmentsInFront = NULL;
        if (numBehind > 0) segmentsBehind = (Segment *)malloc(numBehind * sizeof(Segment));
        if (numInFront > 0) segmentsInFront = (Segment *)malloc(numInFront * sizeof(Segment));
        usize behindIdx = 0, inFrontIdx = 0, insideIdx = 1;
        /* second pass - add segments to middle node and recurse into left/right children */
        for (usize i = 1; i < len; i++)
        {
            Segment si = segments[i];
            f64 leftSide = DVector2Determinant(splitVec, DVector2Subtract(si.left, split.left));
            f64 rightSide = DVector2Determinant(splitVec, DVector2Subtract(si.right, split.left));

            /* both endpoints inside split segment => add si to current node segment list */
            if (babs(leftSide) < EPSILON && babs(rightSide) < EPSILON) node->segments[insideIdx++] = si;

            else
            {
                /* both endpoints behind split segment => add si to left segment list */
                if (leftSide - EPSILON < 0 && rightSide - EPSILON < 0) segmentsBehind[behindIdx++] = si;

                /* both endpoints in front of split segment => add si to right segment list */
                else if (leftSide + EPSILON > 0 && rightSide + EPSILON > 0) segmentsInFront[inFrontIdx++] = si;

                /*
                 * current segmenst is bisected by splitting line =>
                 *  - find intersection point of segment si with splitting line
                 *  - insert split subsegments into respective left/right segment lists
                 *  - update bools for each subsegment for "free split" check in recursive call
                 */
                else
                {
                    segmentsBehind[behindIdx] = segments[i];
                    segmentsInFront[inFrontIdx] = segments[i];
                    DVector2 intersection = SegmentIntersection(split, si);

                    if (leftSide < rightSide)
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
            }
        }

        /* TODO: FIGURE OUT WHY TF IT'S NOT SUPPOSED TO BE THE OTHER WAY AROUND */
        node->right = BuildBspTree(segmentsBehind, numBehind, node);
        node->left = BuildBspTree(segmentsInFront, numInFront, node);
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

    BspNode *root;
    { /* create actual bsp tree */
        Segment *segmentsCopy = (Segment *)malloc(len * sizeof(Segment));
        memcpy(segmentsCopy, segments, len * sizeof(Segment));
        root = BuildBspTree(segmentsCopy, len, NULL);
        tree->root = root;
        tree->active = root;
    }

    { /* calculate size of tree */
        BspNode *node = MinNode(root);
        while (node)
        {
            tree->size += 1;
            node = SuccNode(node);
        }
    }

    { /* add each node to array for easy indexing (also store node depth) */
        tree->meta = (BspNodeMeta *)malloc(tree->size * sizeof(BspNodeMeta));
        BspNode *node = MinNode(root);
        for (usize i = 0; i < tree->size; i++)
        {
            if (node == root)
            {
                tree->rootIdx = i;
                tree->activeIdx = i;
            }
            tree->meta[i].node = node;
            tree->meta[i].depth = 0;
            BspNode *tmp = node->parent;
            while (tmp)
            {
                tree->meta[i].depth += 1;
                tmp = tmp->parent;
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
    tree->activeRegion = tree->meta[tree->rootIdx].region;
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
    if (idx == tree->rootIdx) tree->meta[idx].region = BuildBspRegion(WIDTH / 2, HEIGHT, tree->meta[idx].node->segments[0]);
    else
    {
        BspNodeMeta parent = tree->meta[idxParent(tree, idx)];
        BspNode *node = bspNode(tree, idx);
        if (node == parent.node->left) tree->meta[idx].region = NewBspRegion(parent.region, node->segments, node->numSegments, SplitLeft);
        if (node == parent.node->right) tree->meta[idx].region = NewBspRegion(parent.region, node->segments, node->numSegments, SplitRight);
    }
    if (bspNode(tree, idx)->left) BuildTreeMetaRegions(tree, idxLeft(tree, idx));
    if (bspNode(tree, idx)->right) BuildTreeMetaRegions(tree, idxRight(tree, idx));
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
BspTreeMetaMoveLeft(BspTreeMeta *tree)
{
    if (tree->active->left)
    {
        tree->active = tree->active->left;
        tree->activeIdx = idxLeft(tree, tree->activeIdx);
        tree->activeRegion = tree->meta[tree->activeIdx].region;
    }
}

void
BspTreeMetaMoveRight(BspTreeMeta *tree)
{
    if (tree->active->right)
    {
        tree->active = tree->active->right;
        tree->activeIdx = idxRight(tree, tree->activeIdx);
        tree->activeRegion = tree->meta[tree->activeIdx].region;
    }
}

void
BspTreeMetaMoveUp(BspTreeMeta *tree)
{
    if (tree->active->parent)
    {
        tree->active = tree->active->parent;
        tree->activeIdx = idxParent(tree, tree->activeIdx);
        tree->activeRegion = tree->meta[tree->activeIdx].region;
    }
}

BspNode *
MinNode(BspNode *r)
{
    BspNode *node = r;
    while (node->left)
        node = node->left;
    return node;
}

BspNode *
MaxNode(BspNode *r)
{
    BspNode *node = r;
    while (node->right)
        node = node->right;
    return node;
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

BspNode *
bspNode(BspTreeMeta *tree, usize idx)
{
    return tree->meta[idx].node;
}

usize
idxLeft(BspTreeMeta *tree, usize idx)
{
    return tree->meta[idx].left;
}

usize
idxRight(BspTreeMeta *tree, usize idx)
{
    return tree->meta[idx].right;
}

usize
idxParent(BspTreeMeta *tree, usize idx)
{
    return tree->meta[idx].parent;
}
