#include "raylib.h"
#include <stdint.h>

#define WIDTH 640
#define HEIGHT 640

typedef struct BSP_Node {
    struct BSP_Node *left;
    struct BSP_Node *right;
    uint32_t data;
} BSP_Node;

int32_t
main(int32_t argc, char *argv[])
{
    InitWindow(WIDTH, HEIGHT, "bsp tree");
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    return 0;
}
