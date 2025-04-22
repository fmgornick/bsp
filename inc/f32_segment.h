#ifndef F32_SEGMENT_H_
#define F32_SEGMENT_H_

#include "bsp.h"
#include "f64_segment.h"
#include <stdbool.h>

#define F32_EPSILON 0.0001f

typedef struct FSegment {
    Vector2 origin;
    Vector2 dest;
} FSegment;

typedef enum Side {
    FSideLeft,
    FSideRight,
    FSideInside,
    FSideBoth,
} Side;

FSegment *BuildFSegments(const DSegment *dSegments, usize numSegments, BoundingRegion region, usize *size);
f32 FSegmentsDotProduct(FSegment s1, FSegment s2);
f32 FSegmentPointDeterminant(FSegment s, Vector2 pt);
Side FSegmentSide(FSegment s, Vector2 pt);
Side FSegmentSides(FSegment s1, FSegment s2);
Vector2 FSegmentIntersection(FSegment s1, FSegment s2);
bool FSegmentContainsPoint(FSegment s, Vector2 pt);
bool FSegmentsParallel(FSegment s1, FSegment s2);
bool FSegmentsIntersect(FSegment s1, FSegment s2);

#endif // F32_SEGMENT_H_
