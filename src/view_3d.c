#include "view_3d.h"
#include "bsp_utils.h"
#include "raylib.h"

void
DrawView3D()
{
    ClearBackground(BG);
    DrawText("3D", Q_WIDTH, Q_HEIGHT, 20, FG);
}
