#ifndef F64_SEGMENT_H_
#define F64_SEGMENT_H_

#include "f64_vector.h"
#include "i32_vector.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct DSegment {
    DVector2 left;
    DVector2 right;
    bool splitLeft;
    bool splitRight;
} DSegment;

typedef enum DSide {
    DSideLeft,
    DSideRight,
    DSideInside,
    DSideBoth,
} DSide;

DSegment *BuildSegments(IVector2 *polygon, usize numVertices, BoundingRegion region, usize *size);
void FreeSegments(DSegment *segments);
void DrawSegment(DSegment segment, f32 thick, Color color, bool hasNormal);
void DrawSegments(DSegment *segments, usize len);

f64 DSegmentsDotProduct(DSegment s1, DSegment s2);
f64 DSegmentPointDeterminant(DSegment s, DVector2 pt);
DSide DSegmentSide(DSegment s, DVector2 pt);
DSide DSegmentSides(DSegment s1, DSegment s2);
DVector2 DSegmentIntersection(DSegment s1, DSegment s2);
bool DSegmentContainsPoint(DSegment s, DVector2 pt);
bool DSegmentsParallel(DSegment s1, DSegment s2);
bool DSegentsIntersect(DSegment s1, DSegment s2);

#endif // F64_SEGMENT_H_
