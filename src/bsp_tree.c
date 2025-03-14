#include "bsp_tree.h"
#include "bsp_segment.h"
#include "bsp_utils.h"
#include "f64_vector.h"
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <string.h>

BspNode *
BuildBspNode(Segment *segments, usize len, BspNode *parent)
{
    BspNode *node = (BspNode *)malloc(sizeof(BspNode));
    if (parent)
    {
        node->parent = parent;
        node->depth = parent->depth + 1;
    }
    else
    {
        node->parent = NULL;
        node->depth = 0;
    }

    /* tree->size += 1; */
    /* if (node->depth == tree->height) tree->height += 1; */

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

        node->segments = (Segment *)malloc(numInside * sizeof(Segment));
        node->segments[0] = split;
        node->numSegments = numInside;

        Segment *segmentsBehind, *segmentsInFront;
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

                /* current segmenst is bisected by splitting line =>
                 *  - find intersection point of segment si with splitting line
                 *  - insert split subsegments into respective left/right segment lists
                 *  - update bools for each subsegment for "free split" check in recursive call */
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
        node->right = BuildBspNode(segmentsBehind, numBehind, node);
        node->left = BuildBspNode(segmentsInFront, numInFront, node);
        free(segments);
    }

    return node;
}

BspTree *
BuildBspTree(Segment *segments, usize len, Region region)
{
    BspTree *tree = (BspTree *)malloc(sizeof(BspTree));
    tree->size = 0;
    tree->height = 0;
    tree->region = region;

    Segment *segmentsCopy = (Segment *)malloc(len * sizeof(Segment));
    memcpy(segmentsCopy, segments, len * sizeof(Segment));

    BspNode *root = BuildBspNode(segmentsCopy, len, NULL);
    tree->root = root;
    tree->active = root;

    UpdateTree(tree);
    return tree;
}

void
FreeBspTree(BspTree *tree)
{
    FreeBspNode(tree->root);
    free(tree);
}

void
FreeBspNode(BspNode *node)
{
    if (node->left) FreeBspNode(node->left);
    if (node->right) FreeBspNode(node->right);

    free(node->segments);
    free(node);
}

void
UpdateTree(BspTree *tree)
{
    u32 width = tree->region.right - tree->region.left;
    u32 height = tree->region.bottom - tree->region.top;
    u32 shorter = min(width, height);
    f32 radius = (f32)shorter / max(tree->height * 2, tree->size);

    BspNode *node = MinNode(tree->root);
    u32 i = 0;
    while (node)
    {
        f32 x = ((f32)i / tree->size) * width + tree->region.left;
        f32 y = radius + node->depth * ((f32)height / tree->height) + tree->region.top;

        node->pos = (Vector2){ x, y };
        node->radius = radius;
        node = SuccNode(node);
        i++;
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

void
DrawTree(BspTree *tree)
{
    DrawNode(tree->root);
    DrawCircle(tree->active->pos.x, tree->active->pos.y, tree->active->radius, RED);

    /* draw segments corresponding to active node */
    /* BspNode *node = tree->active; */
    /* while (node) */
    /* { */
    /*     for (usize i = 0; i < node->numSegments; i++) */
    /*     { */
    /*         Segment s = node->segments[i]; */
    /*         Vector2 left = (Vector2){ s.left.x, s.left.y }; */
    /*         Vector2 right = (Vector2){ s.right.x, s.right.y }; */
    /*         if (tree->active == node) DrawLineEx(left, right, 4.0f, RED); */
    /*         else if (tree->active == node->left || tree->active == node->right) DrawLineEx(left, right, 4.0f, DARKPURPLE); */
    /*         else DrawLineEx(left, right, 4.0f, BLUE); */
    /*     } */
    /*     node = node->parent; */
    /* } */
}

void
DrawNode(BspNode *node)
{
    if (node->left)
    {
        DrawLineEx(node->pos, node->left->pos, 1.0f, BLACK);
        DrawNode(node->left);
    }
    if (node->right)
    {
        DrawLineEx(node->pos, node->right->pos, 1.0f, BLACK);
        DrawNode(node->right);
    }
    DrawCircle(node->pos.x, node->pos.y, node->radius, BLUE);
}
