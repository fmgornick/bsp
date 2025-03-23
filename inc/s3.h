#ifndef S3_H_
#define S3_H_

#include "bsp.h"

typedef struct S3 {

} S3;

BspStage S3_Init(S3 *scene);
BspStage S3_Render(S3 *scene);
BspStage S3_RenderFailure(S3 *scene);
void S3_Free(S3 *scene);

#endif // S3_H_
