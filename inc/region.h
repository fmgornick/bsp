#ifndef REGION_H_
#define REGION_H_

#include "bsp.h"
#include "segment.h"
#include "triangulation.h"
#include <stdbool.h>

typedef struct Region {
    Segment *boundary;       /* boundary enclosing region */
    Triangle *triangulation; /* region triangulation */
    usize triangulationSize; /* number of triangles in region triangulation (n-2) */
    usize boundarySize;      /* size of boundary enclosing region */
    Segment line;            /* line bisecting current region (if any) */
    bool hasLine;            /* true if line bisecting current region exists */
    usize leftIdx;           /* index of boundary segment intersecting left side of line*/
    usize rightIdx;          /* index of boundary segment intersecting right side of line*/
} Region;

typedef enum SplitDirection {
    SplitLeft,
    SplitRight,
} SplitDirection;

Region *BuildRegion(usize width, usize height, Segment initialLine);
Region *NewRegion(Region *oldRegion, Segment *newLine, usize numSegments, SplitDirection dir);
void FreeRegion(Region *region);
void DrawRegion(Region *region);

#endif // REGION_H_
