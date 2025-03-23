#include "segment.h"
#include "f64_vector.h"
#include "i32_vector.h"
#include "raylib.h"
#include "raymath.h"
#include "stdlib.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>

Segment *
BuildSegments(IVector2 *polygon, usize numVertices, Region region, usize *size)
{
    u32 width = region.right - region.left;
    u32 height = region.bottom - region.top;

    usize numSegments = numVertices;
    Segment *segments = (Segment *)malloc(numSegments * sizeof(Segment));

    /* we want to resize the polygon from stage one to fit our stage 2 split screen */
    u32 xMin = UINT_MAX, yMin = UINT_MAX, xMax = 0, yMax = 0;
    for (usize i = 0; i < numVertices; i++)
    {
        if (polygon[i].x < xMin) xMin = polygon[i].x;
        if (polygon[i].x > xMax) xMax = polygon[i].x;
        if (polygon[i].y < yMin) yMin = polygon[i].y;
        if (polygon[i].y > yMax) yMax = polygon[i].y;
    }

    /*
     * signedArea > 0 => segments ordered counter-clockwise
     * signedArea < 0 => segments ordered clockwise
     */
    f32 signedArea = 0.0f;
    for (usize i = 0; i < numSegments; i++)
    {
        usize j = (i + 1) % numSegments;
        signedArea += (polygon[i].x * polygon[j].y) - (polygon[j].x * polygon[i].y);
    }
    signedArea /= 2.0f;

    f64 scale = min((f64)width / (xMax - xMin), (f64)height / (yMax - yMin)) * 0.9;
    f64 xPadding = (width - (xMax + xMin) * scale) / 2.0f + region.left;
    f64 yPadding = (height - (yMax + yMin) * scale) / 2.0f + region.top;

    for (usize i = 0; i < numSegments; i++)
    {
        usize j = (i + 1) % numSegments;
        usize leftIdx, rightIdx, segmentIdx;
        if (signedArea >= 0.0f)
        {
            leftIdx = i;
            rightIdx = j;
            segmentIdx = i;
        }
        else
        {
            leftIdx = j;
            rightIdx = i;
            segmentIdx = numSegments - 1 - i;
        }
        segments[segmentIdx] = (Segment){
            .left = (DVector2) {
                .x = (f64)scale * polygon[leftIdx].x + xPadding,
                .y = (f64)scale * polygon[leftIdx].y + yPadding,
            },
            .right = (DVector2){
                .x = (f64)scale * polygon[rightIdx].x + xPadding,
                .y = (f64)scale * polygon[rightIdx].y + yPadding,
            },
            .splitLeft = false,
            .splitRight = false,
        };
    }

    *size = numSegments;
    return segments;
}

void
FreeSegments(Segment *segments)
{
    free(segments);
}

void
DrawSegment(Segment segment, f32 thick, Color color, bool hasNormal)
{
    Vector2 left = { segment.left.x, segment.left.y };
    Vector2 right = { segment.right.x, segment.right.y };
    DrawLineEx(left, right, thick, color);
    if (hasNormal)
    {
        Vector2 half = Vector2Add(left, Vector2Scale(Vector2Subtract(right, left), 0.5f));
        Vector2 unit = Vector2Normalize(Vector2Subtract(right, left));
        Vector2 perp = { unit.y, -unit.x };
        DrawLineEx(half, Vector2Add(half, Vector2Scale(perp, 10.0f)), 5.0f, PURPLE);
    }
}

void
DrawSegments(Segment *segments, usize len)
{
    for (usize i = 0; i < len; i++)
        DrawSegment(segments[i], 3.0f, BLACK, false);
}

f64
SegmentsDotProduct(Segment u, Segment v)
{
    DVector2 du = DVector2Subtract(u.right, u.left);
    DVector2 dv = DVector2Subtract(v.right, v.left);
    return DVector2DotProduct(du, dv);
}

f64
SegmentPointDeterminant(Segment s, DVector2 pt)
{
    DVector2 u = DVector2Subtract(s.right, s.left);
    DVector2 v = DVector2Subtract(pt, s.left);
    return DVector2DotProduct(u, v);
}

Side
SegmentSide(Segment s, DVector2 pt)
{
    DVector2 pq = DVector2Subtract(s.right, s.left);
    DVector2 pr = DVector2Subtract(pt, s.left);
    f64 det = DVector2Determinant(pq, pr);
    if (babs(det) < EPSILON) return SideInside;
    else if (det >= 0.0) return SideLeft;
    else return SideRight;
}

Side
SegmentSides(Segment u, Segment v)
{
    DVector2 pq = DVector2Subtract(u.right, u.left);
    DVector2 pr = DVector2Subtract(v.left, u.left);
    DVector2 ps = DVector2Subtract(v.right, u.left);
    f64 leftSide = DVector2Determinant(pq, pr);
    f64 rightSide = DVector2Determinant(pq, ps);
    if (babs(leftSide) < EPSILON && babs(rightSide) < EPSILON) return SideInside;
    else if (leftSide + EPSILON > 0 && rightSide + EPSILON > 0) return SideLeft;
    else if (leftSide - EPSILON < 0 && rightSide - EPSILON < 0) return SideRight;
    else return SideBoth;
}

DVector2
SegmentIntersection(Segment s1, Segment s2)
{
    assert(!SegmentsParallel(s1, s2));
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
    f64 a1 = s1.right.y - s1.left.y;
    f64 b1 = s1.left.x - s1.right.x;
    f64 c1 = (s1.right.x * s1.left.y) - (s1.left.x * s1.right.y);
    f64 a2 = s2.right.y - s2.left.y;
    f64 b2 = s2.left.x - s2.right.x;
    f64 c2 = (s2.right.x * s2.left.y) - (s2.left.x * s2.right.y);
    return (DVector2){
        .x = (f64)((b1 * c2) - (b2 * c1)) / ((a1 * b2) - (b1 * a2)),
        .y = (f64)((a2 * c1) - (a1 * c2)) / ((a1 * b2) - (b1 * a2)),
    };
}

bool
SegmentContainsPoint(Segment s, DVector2 pt)
{
    f64 a = s.right.y - s.left.y;
    f64 b = s.left.x - s.right.x;
    f64 c = (s.right.x * s.left.y) - (s.left.x * s.right.y);
    if (babs(a * pt.x + b * pt.y + c) > EPSILON) return false;
    if (pt.x < (min(s.left.x, s.right.x) - EPSILON)) return false;
    if (pt.x > (max(s.left.x, s.right.x) + EPSILON)) return false;
    if (pt.y < (min(s.left.y, s.right.y) - EPSILON)) return false;
    if (pt.y > (max(s.left.y, s.right.y) + EPSILON)) return false;
    return true;
}

bool
SegmentsParallel(Segment s1, Segment s2)
{
    DVector2 pq = DVector2Subtract(s1.right, s1.left);
    DVector2 rs = DVector2Subtract(s2.right, s2.left);
    return babs(DVector2Determinant(pq, rs)) < EPSILON;
}
