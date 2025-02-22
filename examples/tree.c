#include "raylib.h"
#include <stdlib.h>
#include <string.h>

#define WIDTH 640
#define HEIGHT 640
#define MAX_DEPTH 24

typedef int i32;
typedef long long int i64;
typedef int isize;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef unsigned int usize;
typedef float f32;
typedef double f64;

typedef struct BSP_Node {
    usize rowIdx;
    usize rowSize;
    bool exists;
} BSP_Node;

BSP_Node *tree;
usize node;
u32 height;
u32 size;

void DrawTree();
void ResizeTree();
void InitTree();

isize
main(isize argc, char *argv[])
{
    InitWindow(WIDTH, HEIGHT, "bsp tree");
    InitTree();

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_RIGHT))
        {
            node = node * 2 + 1;
            if (node >= size)
                ResizeTree();
            if (tree[node].exists == false)
                tree[node].exists = true;
        }

        if (IsKeyPressed(KEY_LEFT))
        {
            node = node * 2;
            if (node >= size)
                ResizeTree();
            if (tree[node].exists == false)
                tree[node].exists = true;
        }

        if (IsKeyPressed(KEY_UP))
        {
            if (node > 1)
                node = node / 2;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTree();
        EndDrawing();
    }

    free(tree);
    return 0;
}

void
ResizeTree()
{
    BSP_Node *tmp = (BSP_Node *)malloc(2 * size * sizeof(BSP_Node));
    memcpy(tmp, tree, size * sizeof(BSP_Node));

    free(tree);
    tree = tmp;

    height += 1;
    size <<= 1;
    TraceLog(LOG_INFO, "new size: %d", size);
}

void
InitTree()
{
    tree = (BSP_Node *)malloc(2 * sizeof(BSP_Node));
    height = 1;
    size = 2;
    node = 1;
    tree[node].rowSize = 1;
    tree[node].rowIdx = 1;
    tree[node].exists = true;
}

void
DrawTree()
{
    f32 radius = (f32)WIDTH / size;
    f32 xPos = (WIDTH / 2.0f);
    f32 yPos = radius;
    for (usize i = 1; i <= size; i <<= 1)
    {
        for (usize j = 0; j < i; j++)
        {
            f32 dx = j * ((f32)WIDTH / i);
            if (tree[i + j].exists)
                DrawCircle(xPos + dx, yPos, radius, BLACK);
        }
        xPos /= 2.0f;
        yPos += (f32)HEIGHT / height;
    }
}
