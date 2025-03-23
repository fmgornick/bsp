#include "bsp.h"
#include "bsp_test.h"
#include "raylib.h"
#include "s1.h"
#include "s2.h"

isize
main(isize argc, char *argv[])
{
    BspStage stage = S2_INITIALIZING;
    S1 s1 = { 0 };
    S2 s2 = { 0 };

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
            break;

        case S2_PENDING:
            stage = S2_Render(&s2);
            break;

        default:
            TraceLog(LOG_ERROR, "uh oh");
        }
    }

    if (stage > S1_INITIALIZING) S1_Free(&s1);
    if (stage > S2_INITIALIZING) S2_Free(&s2);

    return 0;
}
