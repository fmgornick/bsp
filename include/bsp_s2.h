#ifndef BSP_S2_H_
#define BSP_S2_H_

#include "bsp_stages.h"
#include "bsp_utils.h"
#include "bsp_vector.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct S2_Scene {
    // polygon constructed by user
    Vector2 polygon[MAX_VERTICES];
    uint32_t numVertices;
} S2_Scene;

void S2_DrawPolygon(S2_Scene *scene);
void S2_DrawMessage(char *msg, Color fg, Color bg);

BSP_Stage S2_Init(IVector2 *polygon, uint32_t numVertices, S2_Scene *scene);
BSP_Stage S2_Render(S2_Scene *scene);
BSP_Stage S2_RenderFailure(S2_Scene *scene);

#endif // BSP_S2_H_
