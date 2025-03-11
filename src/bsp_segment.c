#include "bsp_segment.h"
#include "raylib.h"
#include "stdlib.h"
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
