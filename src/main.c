#include "bsp_utils.h"
#include "raylib.h"
#include "view_2d.h"
#include "view_3d.h"
#include "view_meta.h"
#include "view_tree.h"
#include <stdint.h>

int
main()
{

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(WIDTH, HEIGHT, "csci 8442 bsp demo");

    uint32_t width = GetScreenWidth();
    uint32_t height = GetScreenHeight();
    uint32_t q_width = width / 2;
    uint32_t q_height = height / 2;

    RenderTexture2D q1_screen = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D q2_screen = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D q3_screen = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D q4_screen = LoadRenderTexture(WIDTH, HEIGHT);

    Rectangle source = { 0, (float)-HEIGHT, (float)WIDTH, (float)-HEIGHT };
    Rectangle dest1 = { q_width, 0, (float)q_width, (float)q_height };
    Rectangle dest2 = { 0, 0, (float)q_width, (float)q_height };
    Rectangle dest3 = { 0, q_height, (float)q_width, (float)q_height };
    Rectangle dest4 = { q_width, q_height, (float)q_width, (float)q_height };

    while (!WindowShouldClose())
    {
        if (IsWindowResized() && !IsWindowFullscreen())
        {
            width = GetScreenWidth();
            height = GetScreenHeight();
            q_width = width / 2;
            q_height = height / 2;

            dest1 = (Rectangle){ q_width, 0, (float)q_width, (float)q_height };
            dest2 = (Rectangle){ 0, 0, (float)q_width, (float)q_height };
            dest3 = (Rectangle){ 0, q_height, (float)q_width, (float)q_height };
            dest4 = (Rectangle){ q_width, q_height, (float)q_width, (float)q_height };
        }

        BeginTextureMode(q1_screen);
        DrawView2D();
        EndTextureMode();

        BeginTextureMode(q2_screen);
        DrawView3D();
        EndTextureMode();

        BeginTextureMode(q3_screen);
        DrawViewTree();
        EndTextureMode();

        BeginTextureMode(q4_screen);
        DrawViewMeta();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BG);

        DrawTexturePro(q1_screen.texture, source, dest1, (Vector2){ 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(q2_screen.texture, source, dest2, (Vector2){ 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(q3_screen.texture, source, dest3, (Vector2){ 0, 0 }, 0.0f, WHITE);
        DrawTexturePro(q4_screen.texture, source, dest4, (Vector2){ 0, 0 }, 0.0f, WHITE);

        /* TraceLog(LOG_INFO, ""); */

        DrawLine(0, q_height, width, q_height, FG);
        DrawLine(q_width, 0, q_width, height, FG);
        EndDrawing();
    }
}
