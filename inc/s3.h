#ifndef S3_H_
#define S3_H_

#include "bsp.h"
#include "bsp_tree.h"

typedef struct S3 {
    BspTreeMeta *tree;
    bool initialized;
} S3;

BspStage S3_Init(BspTreeMeta *tree, S3 *scene);
BspStage S3_Render(S3 *scene);
BspStage S3_RenderFailure(S3 *scene);
void S3_Free(S3 *scene);

#endif // S3_H_
