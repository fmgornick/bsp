#include "bsp_tree.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>

Hyperplane
hyperplane(Segment s)
{
    return (Hyperplane){
        .a = s.p2.y - s.p1.y,
        .b = s.p1.x - s.p2.x,
        .c = (s.p2.x * s.p1.y) - (s.p1.x * s.p2.y),
    };
}

BspNode *
BuildBspNode(Segment *segments, usize len, BspTree *tree, BspNode *parent)
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

    tree->size += 1;
    if (node->depth == tree->height) tree->height += 1;

    if (len <= 1)
    {
        node->segments = segments;
        node->numSegments = len;
    }
    else
    {
        Segment splitSegment = segments[0];
        /* use free split as partitioning segment if one exists */
        for (int i = 0; i < len; i++)
        {
            if (segments[i].splitLeft && segments[i].splitRight)
            {
                splitSegment = segments[i];
                break;
            }
        }

        Hyperplane h = hyperplane(splitSegment);
        usize numLeft = 0, numRight = 0, numMiddle = 1;
        /* first pass - find size needed for sub-trees */
        for (usize i = 1; i < len; i++)
        {
            Segment si = segments[i];

            /* to check if segment lies exactly inside hyperplane we need perfect precision so we use integer endpoints
             * (these aren't the "true" endpoints, but will suffice for checking if segment si lies fully inside hyperplane h) */
            i32 p1Endpoint = h.a * si.p1.x + h.b * si.p1.y + h.c;
            i32 p2Endpoint = h.a * si.p2.x + h.b * si.p2.y + h.c;
            if (p1Endpoint == 0 && p2Endpoint == 0) numMiddle += 1;

            else
            {
                f32 leftEndpoint = h.a * si.leftEndpoint.x + h.b * si.leftEndpoint.y + h.c;
                f32 rightEndpoint = h.a * si.rightEndpoint.x + h.b * si.rightEndpoint.y + h.c;

                if (leftEndpoint < 0 && rightEndpoint < 0) numLeft += 1;
                else if (leftEndpoint > 0 && rightEndpoint > 0) numRight += 1;
                else
                {
                    numLeft += 1;
                    numRight += 1;
                }
            }
        }

        node->segments = (Segment *)malloc(numMiddle * sizeof(Segment));
        node->segments[0] = splitSegment;
        node->numSegments = numMiddle;

        Segment *sLeft, *sRight;
        if (numLeft > 0) sLeft = (Segment *)malloc(numLeft * sizeof(Segment));
        if (numRight > 0) sRight = (Segment *)malloc(numRight * sizeof(Segment));
        usize idxLeft = 0, idxRight = 0, idxMiddle = 1;
        /* second pass - add segments to middle node and recurse into left/right children */
        for (usize i = 1; i < len; i++)
        {
            Segment si = segments[i];

            i32 p1Endpoint = h.a * si.p1.x + h.b * si.p1.y + h.c;
            i32 p2Endpoint = h.a * si.p2.x + h.b * si.p2.y + h.c;
            if (p1Endpoint == 0 && p2Endpoint == 0) node->segments[idxMiddle++] = si;

            else
            {
                f32 leftEndpoint = h.a * si.leftEndpoint.x + h.b * si.leftEndpoint.y + h.c;
                f32 rightEndpoint = h.a * si.rightEndpoint.x + h.b * si.rightEndpoint.y + h.c;

                /* both endpoints on left side of hyperplane => add si to left segment list */
                if (leftEndpoint < 0 && rightEndpoint < 0) sLeft[idxLeft++] = si;

                /* both endpoints on right side of hyperplane => add si to right segment list */
                else if (leftEndpoint > 0 && rightEndpoint > 0) sRight[idxRight++] = si;

                /* current segmenst is split by h =>
                 *  - find intersection point of segment si with hyperplane h
                 *  - insert split subsegments into respective left/right segment lists
                 *  - update bools for each subsegment for "free split" check in recursive call */
                else
                {
                    sLeft[idxLeft] = segments[i];
                    sRight[idxRight] = segments[i];

                    /*
                     * a1 * x + b1 * y + c1 = 0
                     * a2 * x + b2 * y + c2 = 0
                     *
                     * => |a1 b1| |x|   |c1|
                     *    |a2 b2| |y| + |c2| = 0
                     *
                     * =>  |x|   |a1 b1|-1 |-c1|
                     *     |y| = |a2 b2|   |-c2|
                     * */
                    Hyperplane hi = hyperplane(si);
                    Vector2 intersection = {
                        .x = (f32)((h.b * hi.c) - (hi.b * h.c)) / ((h.a * hi.b) - (hi.a * h.b)),
                        .y = (f32)((hi.a * h.c) - (h.a * hi.c)) / ((h.a * hi.b) - (hi.a * h.b)),
                    };

                    sLeft[idxLeft].splitRight = true;
                    sRight[idxRight].splitLeft = true;
                    sLeft[idxLeft].rightEndpoint = intersection;
                    sRight[idxRight].leftEndpoint = intersection;

                    idxLeft += 1;
                    idxRight += 1;
                }
            }
        }

        node->left = BuildBspNode(sLeft, numLeft, tree, node);
        node->right = BuildBspNode(sRight, numRight, tree, node);
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

    BspNode *root = BuildBspNode(segmentsCopy, len, tree, NULL);
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
        f32 x = ((f32)i / tree->size) * width + radius + tree->region.left;
        f32 y = radius + node->depth * ((f32)height / tree->height) + tree->region.top;
        /* u32 y = radius * (2 * node->depth + 1); */

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
