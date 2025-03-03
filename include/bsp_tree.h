#ifndef BSP_TREE_H_
#define BSP_TREE_H_

#include "bsp_segment.h"
#include "bsp_utils.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct BspNode {
    struct BspNode *left;
    struct BspNode *right;
    struct BspNode *parent;

    Vector2 pos;
    f32 radius;
    u32 depth;

    Segment *segments;
    usize numSegments;
} BspNode;

typedef struct BspTree {
    BspNode *root;
    BspNode *active;
    usize size;
    usize height;

    Region region;
} BspTree;

BspNode *BuildBspNode(Segment *segments, usize len, BspTree *tree, BspNode *parent);
BspTree *BuildBspTree(Segment *segments, usize len, Region region);
void FreeBspTree(BspTree *tree);
void FreeBspNode(BspNode *node);

BspNode *MinNode(BspNode *root);
BspNode *MaxNode(BspNode *root);
BspNode *PrevNode(BspNode *node);
BspNode *SuccNode(BspNode *node);

void UpdateTree(BspTree *tree);
void DrawTree(BspTree *tree);
void DrawNode(BspNode *node);

#endif // BSP_TREE_H_
