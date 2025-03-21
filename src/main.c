#include "bsp_s1.h"
#include "bsp_s2.h"
#include "bsp_stages.h"
#include "bsp_test.h"
#include "raylib.h"

isize
main(isize argc, char *argv[])
{
    BSP_Stage stage = S2_INITIALIZING;
    S1_Scene s1 = { 0 };
    S2_Scene s2 = { 0 };

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
