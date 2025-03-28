#include "s3.h"
#include "bsp_tree.h"
#include "raylib.h"
#include <stdlib.h>

BspStage
S3_Init(BspTreeMeta *tree, S3 *scene)
{
    scene->tree = (BspTreeMeta *)malloc(sizeof(BspTreeMeta));
    CopyBspTree(tree, scene->tree);
    scene->initialized = true;
    return S3_PENDING;
}

BspStage
S3_Render(S3 *scene)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();

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
    FreeBspTreeMeta(scene->tree);
    scene->initialized = false;
}
