#ifndef BSP_SEGMENT_H_
#define BSP_SEGMENT_H_

#include "f64_vector.h"
#include "i32_vector.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct Segment {
    DVector2 left;
    DVector2 right;
    bool splitLeft;
    bool splitRight;
} Segment;

typedef enum Side {
    SideLeft,
    SideRight,
    SideInside,
    SideBoth,
} Side;

Segment *BuildSegments(IVector2 *polygon, usize numVertices, Region region, usize *size);
void FreeSegments(Segment *segments);
void DrawSegment(Segment segment, f32 thick, Color color, bool hasNormal);
void DrawSegments(Segment *segments, usize len);

f64 SegmentsDotProduct(Segment s1, Segment s2);
f64 SegmentPointDeterminant(Segment s, DVector2 pt);
Side SegmentSide(Segment s, DVector2 pt);
Side SegmentSides(Segment s1, Segment s2);
DVector2 SegmentIntersection(Segment s1, Segment s2);
bool SegmentContainsPoint(Segment s, DVector2 pt);
bool SegmentsParallel(Segment s1, Segment s2);

#endif // BSP_SEGMENT_H_
