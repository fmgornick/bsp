#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_utils.h"
#include "f64_vector.h"
#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

BspRegion *
BuildBspRegion(usize width, usize height, Segment initialLine)
{
    BspRegion *region = (BspRegion *)malloc(sizeof(BspRegion));

    { /*
       * create counter-clockwise boundary around provided region
       */
        region->boundarySize = 4;
        region->boundary = (Segment *)malloc(region->boundarySize * sizeof(Segment));
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
    }

    { /*
       * create new active segment from newLine
       * (also make sure it only intersects 2 boundary lines)
       */
        DVector2 intersections[2];
        usize indexes[2];
        usize numIntersections = 0;
        for (usize i = 0; i < region->boundarySize; i++)
        {
            if (!SegmentsParallel(initialLine, region->boundary[i]))
            {
                DVector2 intersection = SegmentIntersection(initialLine, region->boundary[i]);
                if (PointInSegment(intersection, region->boundary[i]))
                {
                    intersections[numIntersections] = intersection;
                    indexes[numIntersections] = i;
                    numIntersections += 1;
                }
            }
        }
        assert(numIntersections == 2);
        region->line.left = intersections[0];
        region->leftIdx = indexes[0];
        region->line.right = intersections[1];
        region->rightIdx = indexes[1];
    }

    return region;
}

BspRegion *
NewBspRegion(const BspRegion *oldRegion, const Segment *segments, usize numSegments, SplitDirection dir)
{
    BspRegion *newRegion = (BspRegion *)malloc(sizeof(BspRegion));

    Segment line;
    usize leftIdx, rightIdx;
    { /*
       * new active segment is on the right => flip active segment orientation
       * (maintain counter-clockwise ordering)
       */
        if (dir == SplitRight)
        {
            line = (Segment){
                .left = oldRegion->line.right,
                .right = oldRegion->line.left,
            };
            leftIdx = oldRegion->rightIdx;
            rightIdx = oldRegion->leftIdx;
        }
        else
        {
            line = oldRegion->line;
            leftIdx = oldRegion->leftIdx;
            rightIdx = oldRegion->rightIdx;
        }
    }

    { /*
       * create new boundary built of current active segment
       *   - old segment becomes first in new boundary
       *   - next segment in boundary is the split segment on the right end of the old active segment
       *   - last segment in boundary is the split segment on the left end of the old active segment
       *   - all remaining boundary points on "dir" side of segment are added between the split segemtns
       */
        newRegion->boundarySize = oldRegion->boundarySize - mod(rightIdx - leftIdx, oldRegion->boundarySize) + 2;
        newRegion->boundary = (Segment *)malloc(newRegion->boundarySize * sizeof(Segment));
        newRegion->boundary[0] = line;
        newRegion->boundary[1] = (Segment){
            .left = line.right,
            .right = oldRegion->boundary[rightIdx].right,
        };
        newRegion->boundary[newRegion->boundarySize - 1] = (Segment){
            .left = oldRegion->boundary[leftIdx].left,
            .right = line.left,
        };
        usize newBoundaryIdx = 2;
        usize oldBoundaryIdx = (rightIdx + 1) % oldRegion->boundarySize;
        while (oldBoundaryIdx != leftIdx)
        {
            assert(newBoundaryIdx < newRegion->boundarySize - 1);
            newRegion->boundary[newBoundaryIdx] = oldRegion->boundary[oldBoundaryIdx];
            newBoundaryIdx += 1;
            oldBoundaryIdx = (oldBoundaryIdx + 1) % oldRegion->boundarySize;
        }
    }

    if (numSegments > 0)
    { /*
       * create new active segment from newLine (if it exists)
       * (also make sure it only intersects 2 boundary lines)
       */
        DVector2 intersections[5];
        usize indexes[5];
        usize numIntersections = 0;
        for (usize i = 0; i < newRegion->boundarySize; i++)
        {
            if (!SegmentsParallel(segments[0], newRegion->boundary[i]))
            {
                DVector2 intersection = SegmentIntersection(segments[0], newRegion->boundary[i]);
                if (PointInSegment(intersection, newRegion->boundary[i]))
                {
                    intersections[numIntersections] = intersection;
                    indexes[numIntersections] = i;
                    numIntersections += 1;
                }
            }
        }
        /* assert(numIntersections == 2); */
        if (numIntersections == 2)
        {
            newRegion->line.left = intersections[0];
            newRegion->leftIdx = indexes[0];
            newRegion->line.right = intersections[1];
            newRegion->rightIdx = indexes[1];
        }
    }

    return newRegion;
}

void
FreeBspRegion(BspRegion *region)
{
    free(region->boundary);
    free(region);
}

void
DrawBspRegion(BspRegion *region)
{
    for (usize i = 0; i < region->boundarySize; i++)
    {
        Segment s = region->boundary[i];
        DrawLineEx((Vector2){ s.left.x, s.left.y }, (Vector2){ s.right.x, s.right.y }, 5.0f, BLUE);
    }
    DrawLineEx((Vector2){ region->line.left.x, region->line.left.y }, (Vector2){ region->line.right.x, region->line.right.y }, 5.0f, RED);
}
