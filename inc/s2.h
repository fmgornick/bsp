#ifndef S2_H_
#define S2_H_

#include "bsp.h"
#include "bsp_tree.h"
#include "i32_vector.h"
#include <stdbool.h>

typedef struct S2 {
    Segment *segments; /* line segments of polygon from S1 */
    usize numSegments; /* number of line segments in polygon from S1 */
    BspTreeMeta *tree; /* BSP tree with metadata for visual display */

    bool building;       /* (pt 1) true if automated tree build running */
    f32 buildTreeDt;     /* (pt 1) time from last frame (for automated building) */
    bool treeBuilt;      /* (pt 1) true if tree build demo portion over */
    bool drawAllRegions; /* (pt 2) true if we want to highlight all bsp regions */
    bool initialized;
} S2;

BspStage S2_Init(IVector2 *polygon, u32 numVertices, S2 *scene);
BspStage S2_Render(S2 *scene);
BspStage S2_RenderFailure(S2 *scene);
void S2_Free(S2 *scene);

#endif // S2_H_
