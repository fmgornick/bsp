#include "bsp_region.h"
#include "bsp_segment.h"
#include "bsp_utils.h"
#include "f64_vector.h"
#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BspRegion *
InitRegion(usize width, usize height)
{
    BspRegion *region = (BspRegion *)malloc(sizeof(BspRegion));
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
    region->hasLine = false;
    return region;
}

BspRegion *
BuildRegion(usize width, usize height, Segment initialLine)
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
NewRegion(const BspRegion *oldRegion, const Segment *segments, usize numSegments, SplitDirection dir)
{
    BspRegion *newRegion = (BspRegion *)malloc(sizeof(BspRegion));

    if (numSegments == 0)
    {
        newRegion->boundarySize = oldRegion->boundarySize;
        newRegion->boundary = (Segment *)malloc(newRegion->boundarySize * sizeof(Segment));
        memcpy(newRegion->boundary, oldRegion->boundary, newRegion->boundarySize * sizeof(Segment));
    }
    else
    {
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

            newRegion->boundarySize = (isize)(oldRegion->boundarySize - mod(rightIdx - leftIdx, oldRegion->boundarySize)) + 2;
            printf("(%d - %d) %% %d = %d\n", rightIdx, leftIdx, oldRegion->boundarySize, mod(rightIdx - leftIdx, oldRegion->boundarySize));

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

        /* { */
        /*     newRegion->boundarySize = 0; */
        /*     newRegion->boundary = (Segment *)malloc(100 * sizeof(Segment)); */
        /*     newRegion->boundary[newRegion->boundarySize++] = line; */
        /*     newRegion->boundary[newRegion->boundarySize++] = (Segment){ */
        /*         .left = line.right, */
        /*         .right = oldRegion->boundary[rightIdx].right, */
        /*     }; */
        /*     usize oldBoundaryIdx = (oldRegion->rightIdx + 1) % oldRegion->boundarySize; */
        /*     while (oldBoundaryIdx != leftIdx) */
        /*     { */
        /*         newRegion->boundary[newRegion->boundarySize++] = oldRegion->boundary[oldBoundaryIdx]; */
        /*         oldBoundaryIdx = (oldBoundaryIdx + 1) % oldRegion->boundarySize; */
        /*     } */
        /*     newRegion->boundary[newRegion->boundarySize++] = (Segment){ */
        /*         .left = oldRegion->boundary[leftIdx].left, */
        /*         .right = line.left, */
        /*     }; */
        /* } */

        { /*
           * create new active segment from newLine
           * (also make sure it only intersects 2 boundary lines)
           */
            DVector2 intersections[2];
            usize indexes[2];
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
    }
    return newRegion;
}

void
AddSegment(BspRegion *region, Segment s, SplitDirection dir)
{
    if (!region->hasLine)
    {
        UpdateRegionSegment(s, region);
        region->hasLine = true;
    }
    else
    {
        /* if new active segment is on the right, then flip the current active segment to maintain cc ordering */
        if (dir == SplitRight)
        {
            DVector2 tmpIntersection = region->line.left;
            usize tmpIdx = region->leftIdx;
            region->line.left = region->line.right;
            region->leftIdx = region->rightIdx;
            region->line.right = tmpIntersection;
            region->rightIdx = tmpIdx;
        }

        /* new boundary starts at old active segment */
        Segment newBoundary[MAX_VERTICES + 4];
        usize newBoundarySize = 0;
        newBoundary[newBoundarySize++] = region->line;

        /* next segment in boundary is the split segment on the right end of the old active segment */
        newBoundary[newBoundarySize++] = (Segment){
            .left = region->line.right,
            .right = region->boundary[region->rightIdx].right,
        };

        /* all remaining boundary points before the last one are NOT intersected by the old active segment */
        for (usize i = (region->rightIdx + 1) % region->boundarySize; i != region->leftIdx; i = (i + 1) % region->boundarySize)
            newBoundary[newBoundarySize++] = region->boundary[i];

        /* last segment in boundary is the split segment on the left end of the old active segment */
        newBoundary[newBoundarySize++] = (Segment){
            .left = region->boundary[region->leftIdx].left,
            .right = region->line.left,
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
                    region->line.left = intersection;
                    region->leftIdx = i;
                    first = false;
                }
                else
                {
                    region->line.right = intersection;
                    region->rightIdx = i;
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
    DrawLineEx((Vector2){ region->line.left.x, region->line.left.y }, (Vector2){ region->line.right.x, region->line.right.y }, 5.0f, RED);
}
