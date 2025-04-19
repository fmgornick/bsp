#include "f32_segment.h"
#include "bsp.h"
#include "raymath.h"
#include <assert.h>

f32
FSegmentsDotProduct(FSegment u, FSegment v)
{
    Vector2 du = Vector2Subtract(u.dest, u.origin);
    Vector2 dv = Vector2Subtract(v.dest, v.origin);
    return Vector2DotProduct(du, dv);
}

f32
FSegmentPointDeterminant(FSegment s, Vector2 pt)
{
    Vector2 u = Vector2Subtract(s.dest, s.origin);
    Vector2 v = Vector2Subtract(pt, s.origin);
    return Vector2DotProduct(u, v);
}

Side
FSegmentSide(FSegment s, Vector2 pt)
{
    Vector2 pq = Vector2Subtract(s.dest, s.origin);
    Vector2 pr = Vector2Subtract(pt, s.origin);
    f32 det = Vector2Determinant(pq, pr);
    if (babs(det) < BSP_EPSILON) return FSideInside;
    else if (det >= 0.0) return FSideLeft;
    else return FSideRight;
}

Side
FSegmentSides(FSegment u, FSegment v)
{
    Vector2 pq = Vector2Subtract(u.dest, u.origin);
    Vector2 pr = Vector2Subtract(v.origin, u.origin);
    Vector2 ps = Vector2Subtract(v.dest, u.origin);
    f32 leftSide = Vector2Determinant(pq, pr);
    f32 rightSide = Vector2Determinant(pq, ps);
    if (babs(leftSide) < BSP_EPSILON && babs(rightSide) < BSP_EPSILON) return FSideInside;
    else if (leftSide + BSP_EPSILON > 0 && rightSide + BSP_EPSILON > 0) return FSideLeft;
    else if (leftSide - BSP_EPSILON < 0 && rightSide - BSP_EPSILON < 0) return FSideRight;
    else return FSideBoth;
}

Vector2
FSegmentIntersection(FSegment s1, FSegment s2)
{
    assert(!FSegmentsParallel(s1, s2));
    /*
     * a1 * x + b1 * y + c1 = 0
     * a2 * x + b2 * y + c2 = 0
     *
     * => |a1 b1| |x|   |c1|
     *    |a2 b2| |y| + |c2| = 0
     *
     * =>  |x|   |a1 b1|-1 |-c1|
     *     |y| = |a2 b2|   |-c2|
     */
    f32 a1 = s1.dest.y - s1.origin.y;
    f32 b1 = s1.origin.x - s1.dest.x;
    f32 c1 = (s1.dest.x * s1.origin.y) - (s1.origin.x * s1.dest.y);
    f32 a2 = s2.dest.y - s2.origin.y;
    f32 b2 = s2.origin.x - s2.dest.x;
    f32 c2 = (s2.dest.x * s2.origin.y) - (s2.origin.x * s2.dest.y);
    return (Vector2){
        .x = (f32)((b1 * c2) - (b2 * c1)) / ((a1 * b2) - (b1 * a2)),
        .y = (f32)((a2 * c1) - (a1 * c2)) / ((a1 * b2) - (b1 * a2)),
    };
}

bool
FSegmentContainsPoint(FSegment s, Vector2 pt)
{
    f32 a = s.dest.y - s.origin.y;
    f32 b = s.origin.x - s.dest.x;
    f32 c = (s.dest.x * s.origin.y) - (s.origin.x * s.dest.y);
    if (babs(a * pt.x + b * pt.y + c) > BSP_EPSILON) return false;
    if (pt.x < (min(s.origin.x, s.dest.x) - BSP_EPSILON)) return false;
    if (pt.x > (max(s.origin.x, s.dest.x) + BSP_EPSILON)) return false;
    if (pt.y < (min(s.origin.y, s.dest.y) - BSP_EPSILON)) return false;
    if (pt.y > (max(s.origin.y, s.dest.y) + BSP_EPSILON)) return false;
    return true;
}

bool
FSegmentsParallel(FSegment s1, FSegment s2)
{
    Vector2 pq = Vector2Subtract(s1.dest, s1.origin);
    Vector2 rs = Vector2Subtract(s2.dest, s2.origin);
    return babs((pq.x * rs.y - pq.y * rs.x)) < BSP_EPSILON;
}

bool
FSegmentsIntersect(FSegment s1, FSegment s2)
{
    return (FSegmentSide(s1, s2.origin) != FSegmentSide(s1, s2.dest)) && (FSegmentSide(s2, s1.origin) != FSegmentSide(s2, s1.dest));
}
