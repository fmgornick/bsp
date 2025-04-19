#include "region.h"
#include "bsp.h"
#include "f64_segment.h"
#include "f64_vector.h"
#include "raylib.h"
#include "triangulation.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/* ********** helpers ********** */
void flipSplit(Region *region);
/* ***************************** */

Region *
BuildRegion(usize width, usize height, DSegment initialLine)
{
    Region *region = (Region *)malloc(sizeof(Region));

    { /*
       * create counter-clockwise boundary around provided region
       */
        region->boundarySize = 4;
        region->boundary = (DSegment *)malloc(region->boundarySize * sizeof(DSegment));
        region->boundary[0] = (DSegment){
            .left = (DVector2){ 0, 0 },
            .right = (DVector2){ 0, height },
        };
        region->boundary[1] = (DSegment){
            .left = (DVector2){ 0, height },
            .right = (DVector2){ width, height },
        };
        region->boundary[2] = (DSegment){
            .left = (DVector2){ width, height },
            .right = (DVector2){ width, 0 },
        };
        region->boundary[3] = (DSegment){
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
            if (DSegmentsParallel(initialLine, region->boundary[i])) continue;
            DVector2 intersection = DSegmentIntersection(initialLine, region->boundary[i]);
            if (DSegmentContainsPoint(region->boundary[i], intersection))
            {
                intersections[numIntersections] = intersection;
                indexes[numIntersections] = i;
                numIntersections += 1;
            }
        }
        assert(numIntersections == 2);
        region->line.left = intersections[0];
        region->leftIdx = indexes[0];
        region->line.right = intersections[1];
        region->rightIdx = indexes[1];
        /* split line and node segment(s) go in opposing directions => flip split line */
        if (DSegmentsDotProduct(initialLine, region->line) < 0) flipSplit(region);
    }

    { /* triangulate region */
        region->triangulation = MonotoneTriangulation(region->boundary, region->boundarySize);
        region->triangulationSize = region->boundarySize - 2;
    }

    return region;
}

Region *
NewRegion(Region *oldRegion, DSegment *segments, usize numSegments, SplitDirection dir)
{
    Region *newRegion = (Region *)malloc(sizeof(Region));

    DSegment line;
    usize leftIdx, rightIdx;
    { /*
       * new active segment is on the right => flip active segment orientation
       * (maintain counter-clockwise ordering)
       */
        if (dir == SplitRight)
        {
            line = (DSegment){
                .left = oldRegion->line.right,
                .right = oldRegion->line.left,
            };
            leftIdx = oldRegion->rightIdx;
            rightIdx = oldRegion->leftIdx;
        }
        else /* dir == SplitLeft */
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
        newRegion->boundary = (DSegment *)malloc(newRegion->boundarySize * sizeof(DSegment));
        newRegion->boundary[0] = line;
        newRegion->boundary[1] = (DSegment){
            .left = line.right,
            .right = oldRegion->boundary[rightIdx].right,
        };
        newRegion->boundary[newRegion->boundarySize - 1] = (DSegment){
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
       *
       * there's a chance that the intersection is in a corner between two
       * boundary segments, in which case we just choose one
       */
        bool intersectedOnce = false;
        for (usize i = 0; i < newRegion->boundarySize; i++)
        {
            if (DSegmentsParallel(segments[0], newRegion->boundary[i])) continue;
            DVector2 intersection = DSegmentIntersection(segments[0], newRegion->boundary[i]);
            if (DSegmentContainsPoint(newRegion->boundary[i], intersection))
            {
                if (!intersectedOnce)
                {
                    newRegion->line.left = intersection;
                    newRegion->leftIdx = i;
                    intersectedOnce = true;
                }
                else if (!DVector2DIsEqual(newRegion->line.left, intersection))
                {
                    newRegion->line.right = intersection;
                    newRegion->rightIdx = i;
                    break;
                }
            }
        }
        /* split line and node segment(s) go in opposing directions => flip split line */
        if (DSegmentsDotProduct(segments[0], newRegion->line) < 0) flipSplit(newRegion);
    }

    { /* triangulate region */
        newRegion->triangulation = MonotoneTriangulation(newRegion->boundary, newRegion->boundarySize);
        newRegion->triangulationSize = newRegion->boundarySize - 2;
    }

    return newRegion;
}

void
FreeRegion(Region *region)
{
    free(region->boundary);
    free(region->triangulation);
    free(region);
}

void
DrawRegion(Region *region)
{
    if (region)
    {
        for (usize i = 0; i < region->triangulationSize; i++)
        {
            Triangle t = region->triangulation[i];
            DrawTriangle(t.v1, t.v2, t.v3, Fade(BLUE, 0.5f));
        }
        for (usize i = 0; i < region->boundarySize; i++)
            DrawSegment(region->boundary[i], 3.0f, BLUE, false);
        DrawSegment(region->line, 3.0f, RED, false);
    }
}

void
flipSplit(Region *region)
{
    DVector2 tmp = region->line.left;
    usize tmpIdx = region->leftIdx;
    region->line.left = region->line.right;
    region->leftIdx = region->rightIdx;
    region->line.right = tmp;
    region->rightIdx = tmpIdx;
}
