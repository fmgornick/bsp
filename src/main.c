#include "bsp.h"
#include "bsp_test.h"
#include "raylib.h"
#include "s1.h"
#include "s2.h"
#include "s3.h"

isize
main(isize argc, char *argv[])
{
    BspStage stage = S3_INITIALIZING;
    S1 s1 = { 0 };
    S2 s2 = { 0 };
    S3 s3 = { 0 };

    /* SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); */
    InitWindow(WIDTH, HEIGHT, "csci 8442 bsp demo");

    while (!WindowShouldClose())
    {
        switch (stage)
        {
        case S1_INITIALIZING:
            stage = S1_Init(&s1);
            break;

        case S1_PENDING:
            stage = S1_Render(&s1);
            break;

        case S1_FAILED:
            stage = S1_RenderFailure(&s1);
            break;

        case S1_COMPLETED:
            stage = S2_INITIALIZING;
            break;

        case S2_INITIALIZING:
            stage = S2_Init(complexPolygon, complexNumVertices, &s2);
            S1_Free(&s1);
            break;

        case S2_PENDING:
            stage = S2_Render(&s2);
            break;

        case S2_COMPLETED:
            stage = S3_INITIALIZING;
            break;

        case S3_INITIALIZING:
            stage = S3_Init(complexPolygon, complexNumVertices, &s3);
            break;

        case S3_PENDING:
            stage = S3_Render(&s3);
            break;

        case S3_COMPLETED:
            S3_Free(&s3);
            break;

        default:
            TraceLog(LOG_ERROR, "uh oh");
            goto cleanup;
        }
    }

cleanup:
    if (s1.initialized) S1_Free(&s1);
    if (s2.initialized) S2_Free(&s2);
    if (s3.initialized) S3_Free(&s3);

    return 0;
}
