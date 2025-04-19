#ifndef S3_H_
#define S3_H_

#include "bsp.h"
#include "bsp_tree.h"
#include "f32_segment.h"

typedef struct Player {
    Vector2 pos;
    Vector2 dir;
    Vector2 ldir;
    FSegment left;
    FSegment right;
    f32 distance;
    f32 hfov;
    f32 vfov;
} Player;

typedef struct S3 {
    Player player;
    FSegment *segments;
    usize numSegments;
    BspTreeMeta *tree;
    bool initialized;
} S3;

BspStage S3_Init(const DSegment *segments, usize numSegments, BspTreeMeta *tree, S3 *scene);
BspStage S3_Render(S3 *scene);
BspStage S3_RenderFailure(S3 *scene);
void S3_Free(S3 *scene);

#endif // S3_H_
