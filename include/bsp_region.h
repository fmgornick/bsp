#ifndef BSP_REGION_H_
#define BSP_REGION_H_

#include "bsp_segment.h"
#include "bsp_utils.h"
#include <stdbool.h>

typedef struct BspRegion {
    Segment boundary[MAX_VERTICES + 4];
    usize boundarySize;

    Segment segment;
    usize leftIntersectIdx;
    usize rightIntersectIdx;
    bool active;
} BspRegion;

typedef enum SplitDirection {
    SplitLeft,
    SplitRight,
} SplitDirection;

BspRegion *InitRegion(usize width, usize height);
void AddSegment(BspRegion *region, Segment s, SplitDirection dir);
void UpdateRegionSegment(Segment s, BspRegion *region);
void DrawRegion(BspRegion *region);

#endif // BSP_REGION_H_
