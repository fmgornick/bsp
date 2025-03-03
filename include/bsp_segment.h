#ifndef BSP_SEGMENT_H_
#define BSP_SEGMENT_H_

#include "bsp_vector.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct Segment {
    IVector2 p1;
    IVector2 p2;
    Vector2 leftEndpoint;
    Vector2 rightEndpoint;
    bool splitLeft;
    bool splitRight;
} Segment;

typedef struct Hyperplane {
    i32 a;
    i32 b;
    i32 c;
} Hyperplane;

Segment *BuildSegments(IVector2 *polygon, usize numVertices, Region region, usize *size);

#endif // BSP_SEGMENT_H_
