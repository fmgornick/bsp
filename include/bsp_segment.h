#ifndef BSP_SEGMENT_H_
#define BSP_SEGMENT_H_

#include "f64_vector.h"
#include "i32_vector.h"
#include <stdbool.h>

typedef struct Segment {
    DVector2 left;
    DVector2 right;
    bool splitLeft;
    bool splitRight;
} Segment;

Segment *BuildSegments(IVector2 *polygon, usize numVertices, Region region, usize *size);
void DrawSegments(Segment *segments, usize len);

#endif // BSP_SEGMENT_H_
