#include "bsp_utils.h"
#include "bsp_vector.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Segment {
    Vector2 leftEndpoint;
    Vector2 rightEndpoint;
    IVector2 p1;
    IVector2 p2;
    bool splitLeft;
    bool splitRight;
} Segment;

/* represents infinitely long line segment (2D hyperplane) */
typedef struct Hyperplane {
    /* ax + by + c = 0 */
    i32 a, b, c;
} Hyperplane;

typedef struct BspNode {
    struct BspNode *left;
    struct BspNode *right;
    Segment *segments;
    usize len;
} BspNode;

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
BuildBspNode(Segment *segments, usize len)
{
    BspNode *root = (BspNode *)malloc(sizeof(BspNode));
    if (len <= 1)
    {
        root->segments = segments;
        root->len = len;
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
            Segment s = segments[i];
            i32 leftEndpoint = h.a * s.p1.x + h.b * s.p1.y + h.c;
            i32 rightEndpoint = h.a * s.p2.x + h.b * s.p2.y + h.c;

            if (leftEndpoint < 0 && rightEndpoint < 0)
                numLeft += 1;
            else if (leftEndpoint > 0 && rightEndpoint > 0)
                numRight += 1;
            else if (leftEndpoint == 0 && rightEndpoint == 0)
                numMiddle += 1;
            else /* current segmenst split by h */
            {
                numLeft += 1;
                numRight += 1;
            }
        }

        root->segments = (Segment *)malloc(numMiddle * sizeof(Segment));
        root->segments[0] = splitSegment;
        root->len = numMiddle;

        Segment *sLeft = (Segment *)malloc(numLeft * sizeof(Segment));
        Segment *sRight = (Segment *)malloc(numRight * sizeof(Segment));
        usize idxLeft = 0, idxRight = 0, idxMiddle = 1;
        /* second pass - add segments to middle node and recurse into left/right children */
        for (usize i = 1; i < len; i++)
        {
            Segment si = segments[i];

            /* to check if segment lies exactly inside hyperplane we need perfect precision so we use integer endpoints
             * (these aren't the "true" endpoints, but will suffice for checking if segment si lies fully inside hyperplane h) */
            i32 p1Endpoint = h.a * si.p1.x + h.b * si.p1.y + h.c;
            i32 p2Endpoint = h.a * si.p2.x + h.b * si.p2.y + h.c;
            if (p1Endpoint == 0 && p2Endpoint == 0)
                root->segments[idxMiddle++] = si;

            f32 leftEndpoint = h.a * si.leftEndpoint.x + h.b * si.leftEndpoint.y + h.c;
            f32 rightEndpoint = h.a * si.rightEndpoint.x + h.b * si.rightEndpoint.y + h.c;

            /* both endpoints on left side of hyperplane => add si to left segment list */
            if (leftEndpoint < 0 && rightEndpoint < 0)
                sLeft[idxLeft++] = si;

            /* both endpoints on right side of hyperplane => add si to right segment list */
            else if (leftEndpoint > 0 && rightEndpoint > 0)
                sRight[idxRight++] = si;

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

        root->left = BuildBspNode(sLeft, numLeft);
        root->right = BuildBspNode(sRight, numRight);
        free(segments);
    }

    return root;
}

BspNode *
BuildBspTree(Segment *segments, usize len)
{
    Segment *S = (Segment *)malloc(len * sizeof(Segment));
    memcpy(S, segments, len);
    return BuildBspNode(S, len * sizeof(Segment));
}
