#ifndef BSP_TRIANGLE_H_
#define BSP_TRIANGLE_H_

#include "bsp_segment.h"
#include "dcel.h"
#include "f64_vector.h"

typedef struct Triangle {
    DVector2 x;
    DVector2 y;
    DVector2 z;
} Triangle;

Triangle *MonotoneTriangulation(Segment *edges, usize numEdges);
void MonotoneTriangulate(DCEL *polygon);

#endif // BSP_TRIANGLE_H_
