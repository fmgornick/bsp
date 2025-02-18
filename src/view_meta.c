#include "view_meta.h"
#include "bsp_utils.h"
#include "raylib.h"

void
DrawViewMeta()
{
    ClearBackground(BG);
    DrawText("meta", Q_WIDTH, Q_HEIGHT, 20, FG);
}
