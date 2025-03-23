#ifndef S2_H_
#define S2_H_

#include "bsp.h"
#include "bsp_tree.h"
#include "i32_vector.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct S2 {
    Segment *segments; /* line segments of polygon from S1 */
    usize numSegments; /* number of line segments in polygon from S1 */
    BspTreeMeta *tree; /* BSP tree with metadata for visual display */

    bool drawAllRegions;
} S2;

BspStage S2_Init(IVector2 *polygon, u32 numVertices, S2 *scene);
BspStage S2_Render(S2 *scene);
BspStage S2_RenderFailure(S2 *scene);
void S2_Free(S2 *scene);

void S2_DrawAllBspRegions(S2 *scene);
void S2_DrawMessage(char *msg, Color fg, Color bg);

#endif // S2_H_
