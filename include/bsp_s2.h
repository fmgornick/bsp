#ifndef BSP_S2_H_
#define BSP_S2_H_

#include "bsp_region.h"
#include "bsp_stages.h"
#include "bsp_tree.h"
#include "bsp_utils.h"
#include "i32_vector.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct BspNodeMeta {
    BspNode *node;     /* associated node in BSP tree */
    BspRegion *region; /* visual region associated with node */
    usize depth;       /* depth in tree (root=0) */
    Vector2 pos;       /* position of node for visualization */
    usize left;        /* index of left child (if exists) */
    usize right;       /* index of right child (if exists) */
    usize parent;      /* index of parent (if exists) */
} BspNodeMeta;

typedef struct S2_Scene {
    Segment *segments; /* line segments of polygon from S1 */
    usize numSegments; /* number of line segments in polygon from S1 */
    BspTree *tree;
    Region treeRegion;

    usize rootIdx;
    usize activeIdx;
    usize treeSize;
    usize treeHeight;
    f32 nodeRadius;
    BspNodeMeta *nodeMeta;
} S2_Scene;

typedef struct BlahTree {
    usize size;
    BspNodeMeta *nodes;
    BspNode *root;
    usize height;
    f32 nodeRadius;
    Region region;
} BlahTree;

BSP_Stage S2_Init(IVector2 *polygon, u32 numVertices, S2_Scene *scene);
BSP_Stage S2_Render(S2_Scene *scene);
BSP_Stage S2_RenderFailure(S2_Scene *scene);
void S2_DrawMessage(char *msg, Color fg, Color bg);

void BuildTree(S2_Scene *scene);
void BuildTreeRegions(S2_Scene *scene, usize idx);
void DrawBspTree(S2_Scene *scene);

#endif // BSP_S2_H_
