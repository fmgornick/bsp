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

    f64 scale = min((f64)width / (xMax - xMin), (f64)height / (yMax - yMin)) * 0.9;
    f64 xPadding = (width - (xMax + xMin) * scale) / 2.0f + region.left;
    f64 yPadding = (height - (yMax + yMin) * scale) / 2.0f + region.top;

    for (usize i = 0; i < numSegments; i++)
    {
        usize j = (i + 1) % numSegments;
        IVector2 p1 = (IVector2){
            .x = scale * polygon[i].x + xPadding,
            .y = scale * polygon[i].y + yPadding,
        };
        IVector2 p2 = (IVector2){
            .x = scale * polygon[j].x + xPadding,
            .y = scale * polygon[j].y + yPadding,
        };
        segments[i] = (Segment){
            .p1 = p1,
            .p2 = p2,
            .leftEndpoint = (Vector2){ p1.x, p1.y },
            .rightEndpoint = (Vector2){ p2.x, p2.y },
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
        Segment s = segments[i];
        DrawLineEx((Vector2){ s.p1.x, s.p1.y }, (Vector2){ s.p2.x, s.p2.y }, 2.0f, BLACK);
    }
}
