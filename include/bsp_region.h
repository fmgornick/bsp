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

BspRegion *BuildBspRegion(usize width, usize height, Segment initialLine);
BspRegion *NewBspRegion(const BspRegion *oldRegion, const Segment *newLine, usize numSegments, SplitDirection dir);
void FreeBspRegion(BspRegion *region);
void DrawBspRegion(BspRegion *region);

#endif // BSP_REGION_H_
