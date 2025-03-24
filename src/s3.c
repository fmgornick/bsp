#include "s3.h"

BspStage
S3_Init(S3 *scene)
{
    return S3_PENDING;
}

BspStage
S3_Render(S3 *scene)
{
    return S3_PENDING;
}

BspStage
S3_RenderFailure(S3 *scene)
{
    return S3_FAILED;
}

void
S3_Free(S3 *scene)
{
}
