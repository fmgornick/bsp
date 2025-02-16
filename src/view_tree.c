#include "view_tree.h"
#include "bsp_utils.h"
#include "raylib.h"

void DrawViewTree() {
    ClearBackground(BG);
    DrawText("tree", Q_WIDTH, Q_HEIGHT, 20, FG);
}
