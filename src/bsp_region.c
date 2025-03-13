#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_utils.h"
#include "f64_vector.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>

BspRegion *
InitRegion(usize width, usize height)
{
    BspRegion *region = (BspRegion *)malloc(sizeof(BspRegion));
    region->boundary[0] = (Segment){
        .left = (DVector2){ 0, 0 },
        .right = (DVector2){ 0, height },
    };
    region->boundary[1] = (Segment){
        .left = (DVector2){ 0, height },
        .right = (DVector2){ width, height },
    };
    region->boundary[2] = (Segment){
        .left = (DVector2){ width, height },
        .right = (DVector2){ width, 0 },
    };
    region->boundary[3] = (Segment){
        .left = (DVector2){ width, 0 },
        .right = (DVector2){ 0, 0 },
    };
    region->boundarySize = 4;
    region->active = false;
    return region;
}

void
AddSegment(BspRegion *region, Segment s, SplitDirection dir)
{
    if (!region->active)
    {
        UpdateRegionSegment(s, region);
        region->active = true;
    }
    else
    {
        /* if new active segment is on the right, then flip the current active segment to maintain cc ordering */
        if (dir == SplitRight)
        {
            DVector2 tmpIntersection = region->segment.left;
            usize tmpIdx = region->leftIntersectIdx;
            region->segment.left = region->segment.right;
            region->leftIntersectIdx = region->rightIntersectIdx;
            region->segment.right = tmpIntersection;
            region->rightIntersectIdx = tmpIdx;
        }

        /* new boundary starts at old active segment */
        Segment newBoundary[MAX_VERTICES + 4];
        usize newBoundarySize = 0;
        newBoundary[newBoundarySize++] = region->segment;

        /* next segment in boundary is the split segment on the right end of the old active segment */
        newBoundary[newBoundarySize++] = (Segment){
            .left = region->segment.right,
            .right = region->boundary[region->rightIntersectIdx].right,
        };

        /* all remaining boundary points before the last one are NOT intersected by the old active segment */
        for (usize i = (region->rightIntersectIdx + 1) % region->boundarySize; i != region->leftIntersectIdx; i = (i + 1) % region->boundarySize)
            newBoundary[newBoundarySize++] = region->boundary[i];

        /* last segment in boundary is the split segment on the left end of the old active segment */
        newBoundary[newBoundarySize++] = (Segment){
            .left = region->boundary[region->leftIntersectIdx].left,
            .right = region->segment.left,
        };

        region->boundarySize = newBoundarySize;
        for (usize i = 0; i < newBoundarySize; i++)
            region->boundary[i] = newBoundary[i];
        UpdateRegionSegment(s, region);
    }
}

void
UpdateRegionSegment(Segment s, BspRegion *region)
{
    DVector2 intersection;
    bool first = true;
    for (usize i = 0; i < region->boundarySize; i++)
    {
        if (!SegmentsParallel(s, region->boundary[i]))
        {
            intersection = SegmentIntersection(s, region->boundary[i]);
            if (PointInSegment(intersection, region->boundary[i]))
            {
                if (first)
                {
                    region->segment.left = intersection;
                    region->leftIntersectIdx = i;
                    first = false;
                }
                else
                {
                    region->segment.right = intersection;
                    region->rightIntersectIdx = i;
                    break;
                }
            }
        }
    }
}

void
DrawRegion(BspRegion *region)
{
    for (usize i = 0; i < region->boundarySize; i++)
    {
        Segment s = region->boundary[i];
        DrawLineEx((Vector2){ s.left.x, s.left.y }, (Vector2){ s.right.x, s.right.y }, 5.0f, BLUE);
    }
    DrawLineEx((Vector2){ region->segment.left.x, region->segment.left.y }, (Vector2){ region->segment.right.x, region->segment.right.y }, 5.0f, RED);
}
