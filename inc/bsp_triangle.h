#ifndef BSP_TRIANGLE_H_
#define BSP_TRIANGLE_H_

#include "dcel.h"

typedef struct Triangle {
    Vector2 v1;
    Vector2 v2;
    Vector2 v3;
} Triangle;

Triangle *MonotoneTriangulation(Segment *segments, usize numSegments);
void MonotoneTriangulateDCEL(DCEL *polygon);

#endif // BSP_TRIANGLE_H_
