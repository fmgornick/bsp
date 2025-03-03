#ifndef BSP_S2_H_
#define BSP_S2_H_

#include "bsp_stages.h"
#include "bsp_tree.h"
#include "bsp_utils.h"
#include "bsp_vector.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct S2_Scene {
    Segment *segments;
    usize numSegments;
    BspTree *tree;
} S2_Scene;

BSP_Stage S2_Init(IVector2 *polygon, u32 numVertices, S2_Scene *scene);
BSP_Stage S2_Render(S2_Scene *scene);
BSP_Stage S2_RenderFailure(S2_Scene *scene);
void S2_DrawMessage(char *msg, Color fg, Color bg);

#endif // BSP_S2_H_
