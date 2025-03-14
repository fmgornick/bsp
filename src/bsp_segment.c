#include "bsp_segment.h"
#include "f64_vector.h"
#include "raylib.h"
#include "raymath.h"
#include "stdlib.h"
#include <assert.h>
#include <limits.h>

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
     * signedArea > 0 => segments ordered counter-clockwise
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
        /* make sure to order segments counter-clockwise, so normals face inward */
        usize segmentIdx = (signedArea > 0) ? i : numSegments - 1 - i;
        segments[segmentIdx] = (Segment){
            .left = (DVector2) {
                .x = (f64)scale * polygon[i].x + xPadding,
                .y = (f64)scale * polygon[i].y + yPadding,
            },
            .right = (DVector2){
                .x = (f64)scale * polygon[j].x + xPadding,
                .y = (f64)scale * polygon[j].y + yPadding,
            },
            .splitLeft = false,
            .splitRight = false,
        };
    }

    *size = numSegments;
    return segments;
}

void
DrawSegments(Segment *segments, usize len)
{
    for (usize i = 0; i < len; i++)
    {
        Segment si = segments[i];
        DrawLineEx((Vector2){ si.left.x, si.left.y }, (Vector2){ si.right.x, si.right.y }, 2.0f, BLACK);
    }
}

DVector2
SegmentIntersection(Segment s1, Segment s2)
{
    /* segments are parallel => assume they never intersect */
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
SegmentsParallel(Segment s1, Segment s2)
{
    return babs(DVector2Determinant(DVector2Subtract(s1.right, s1.left), DVector2Subtract(s2.right, s2.left))) < EPSILON;
}

bool
PointInSegment(DVector2 pt, Segment s)
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

SegmentSide
PointSegmentSide(DVector2 pt, Segment s)
{
    f64 dot = DVector2DotProduct(DVector2Subtract(s.right, s.left), DVector2Subtract(pt, s.left));
    if (babs(dot) < EPSILON) return SegmentInside;
    return dot < 0 ? SegmentRight : SegmentLeft;
}
