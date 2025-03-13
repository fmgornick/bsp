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

typedef enum SegmentSide {
    SegmentLeft = 0b01,
    SegmentRight = 0b10,
    SegmentInside = 0b11,
} SegmentSide;

Segment *BuildSegments(IVector2 *polygon, usize numVertices, Region region, usize *size);
void DrawSegments(Segment *segments, usize len);

DVector2 SegmentIntersection(Segment s1, Segment s2);
bool PointInSegment(DVector2 pt, Segment s);
SegmentSide PointSegmentSide(DVector2 pt, Segment s);
bool SegmentsParallel(Segment s1, Segment s2);

#endif // BSP_SEGMENT_H_
