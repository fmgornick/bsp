#include "view_2d.h"
#include "bsp_utils.h"
#include "raylib.h"

void DrawView2D() {
    ClearBackground(BG);
    DrawText("2D", Q_WIDTH, Q_HEIGHT, 20, FG);
}
