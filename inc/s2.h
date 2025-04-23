#ifndef S2_H_
#define S2_H_

#include "bsp.h"
#include "bsp_tree.h"
#include "i32_vector.h"
#include <stdbool.h>

typedef struct S2 {
    DSegment *segments; /* line segments of polygon from S1 */
    usize numSegments;  /* number of line segments in polygon from S1 */
    BspTreeMeta *tree;  /* BSP tree with metadata for visual display */
    Vector2 helpButton;

    bool building;       /* (pt 1) true if automated tree build running */
    f32 buildTreeDt;     /* (pt 1) time from last frame (for automated building) */
    bool treeBuilt;      /* (pt 1) true if tree build demo portion over */
    bool drawAllRegions; /* (pt 2) true if we want to highlight all bsp regions */
    bool initialized;
    bool helpMenu;
    bool buildMessage;
    bool navMessage;
} S2;

/* clang-format off */
static const char S2_PT1_HELP_MENU[7][128] = {
    "- toggle help menu : H",
    "- quit demonstration : Q",
    "- restart from beginning : R",
    "- step forward through BSP build : RIGHT",
    "- step backwards through BSP build : LEFT",
    "- play/pause BSP build at a fixed rate : SPACE",
    "- skip to the end to reveal full tree : RETURN",
};

static const char S2_PT2_HELP_MENU[9][128] = {
    "- toggle help menu : H",
    "- quit demonstration : Q",
    "- restart from beginning : R",
    "- navigate to specific BSP node : CLICK",
    "- navigate to left child : LEFT",
    "- navigate to right child : RIGHT",
    "- navigate to parent : UP",
    "- toggle BSP region mosaic : F",
    "- continue to 3D rendering portion : RETURN",
};
/* clang-format on */

BspStage S2_Init(IVector2 *polygon, u32 numVertices, S2 *scene);
BspStage S2_Render(S2 *scene);
BspStage S2_RenderFailure(S2 *scene);
void S2_Free(S2 *scene);

#endif // S2_H_
