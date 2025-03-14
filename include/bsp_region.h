#ifndef BSP_REGION_H_
#define BSP_REGION_H_

#include "bsp_segment.h"
#include "bsp_utils.h"
#include <stdbool.h>

typedef struct BspRegion {
    Segment *boundary;  /* boundary enclosing region */
    usize boundarySize; /* size of boundary enclosing region */
    Segment line;       /* line bisecting current region (if any) */
    bool hasLine;       /* true if line bisecting current region exists */
    usize leftIdx;      /* index of boundary segment intersecting left side of line*/
    usize rightIdx;     /* index of boundary segment intersecting right side of line*/
} BspRegion;

typedef enum SplitDirection {
    SplitLeft,
    SplitRight,
} SplitDirection;

BspRegion *InitRegion(usize width, usize height);
void AddSegment(BspRegion *region, Segment s, SplitDirection dir);
void UpdateRegionSegment(Segment s, BspRegion *region);
void DrawRegion(BspRegion *region);

BspRegion *BuildRegion(usize width, usize height, Segment initialLine);
BspRegion *NewRegion(const BspRegion *oldRegion, const Segment *newLine, usize numSegments, SplitDirection dir);

#endif // BSP_REGION_H_
