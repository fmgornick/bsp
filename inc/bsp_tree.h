#ifndef BSP_TREE_H_
#define BSP_TREE_H_

#include "bsp.h"
#include "raylib.h"
#include "region.h"
#include "segment.h"
#include <stdbool.h>

typedef struct BspNode {
    struct BspNode *left;   /* pointer to left child (if exists) */
    struct BspNode *right;  /* pointer to right child (if exists) */
    struct BspNode *parent; /* pointer to parent (if exists) */
    Segment *segments;      /* segment(s) corresponding to BSP node */
    usize numSegments;      /* number of segments for node (usually 1) */
} BspNode;

typedef struct BspNodeMeta {
    BspNode *node;  /* associated node in BSP tree */
    Region *region; /* visual region associated with node */
    usize depth;    /* depth in tree (root=0) */
    Vector2 pos;    /* position of node for visualization */
    usize left;     /* index of left child (if exists) */
    usize right;    /* index of right child (if exists) */
    usize parent;   /* index of parent (if exists) */
    bool visible;
} BspNodeMeta;

typedef struct BspTreeMeta {
    usize height;         /* height of tree */
    usize size;           /* size of tree */
    BspNodeMeta *meta;    /* array of node metadata (sorted left -> right) */
    BspNode *root;        /* pointer to root node of tree */
    BspNode *active;      /* pointer to active node in tree */
    usize rootIdx;        /* index of root node metadata in array */
    usize activeIdx;      /* index of active node metadata in array */
    Region *activeRegion; /* current active region in BSP tree */
    f32 nodeRadius;       /* radius of BSP node for displaying (dependent on tree/region size) */
    BoundingRegion bounds;
    usize visibleSize;
    usize visibleHeight;
} BspTreeMeta;

BspNode *BuildBspTree(Segment *segments, usize len, BspNode *parent);
void FreeBspTree(BspNode *node);
void CopyBspTree(const BspTreeMeta *src, BspTreeMeta *dst);

BspTreeMeta *BuildBspTreeMeta(Segment *segments, usize len, BoundingRegion region);
void FreeBspTreeMeta(BspTreeMeta *tree);
void BuildTreeRegions(BspTreeMeta *tree, usize idx);
void DrawBspTreeMeta(BspTreeMeta *tree);
void UpdateBspTreeMeta(BspTreeMeta *tree);

void BspTreeMetaSetActive(BspTreeMeta *tree, usize i);
void BspTreeMetaMoveLeft(BspTreeMeta *tree);
void BspTreeMetaMoveRight(BspTreeMeta *tree);
void BspTreeMetaMoveUp(BspTreeMeta *tree);

BspNode *MinNode(BspNode *root);
BspNode *MaxNode(BspNode *root);
BspNode *PrevNode(BspNode *node);
BspNode *SuccNode(BspNode *node);
bool IsLeaf(BspNode *node);

BspNode *bspNode(BspTreeMeta *tree, usize idx);
usize idxLeft(BspTreeMeta *tree, usize idx);
usize idxRight(BspTreeMeta *tree, usize idx);
usize idxParent(BspTreeMeta *tree, usize idx);

#endif // BSP_TREE_H_
