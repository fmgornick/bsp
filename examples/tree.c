#include "raylib.h"
#include <stdlib.h>

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
    struct BSP_Node *left;
    struct BSP_Node *right;
    struct BSP_Node *parent;

    Vector2 pos;
    f32 radius;
    u32 depth;
} BSP_Node;

typedef struct BSP_Tree {
    BSP_Node *root;
    BSP_Node *active;
    u32 size;
    u32 height;
} BSP_Tree;

BSP_Tree *NewTree();
void FreeTree(BSP_Tree *tree);
void UpdateTree(BSP_Tree *tree);
void DrawTree(BSP_Tree *tree);

BSP_Node *NewNode(BSP_Tree *tree, BSP_Node *parent);
void FreeNode(BSP_Node *node);
void DrawNode(BSP_Node *node);

BSP_Node *MinNode(BSP_Node *node);
BSP_Node *MaxNode(BSP_Node *node);
BSP_Node *PrevNode(BSP_Node *node);
BSP_Node *SuccNode(BSP_Node *node);

isize
main(isize argc, char *argv[])
{
    InitWindow(WIDTH, HEIGHT, "bsp tree");
    BSP_Tree *tree = NewTree();

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_LEFT))
        {
            if (!tree->active->left)
                tree->active->left = NewNode(tree, tree->active);
            tree->active = tree->active->left;
            UpdateTree(tree);
        }

        if (IsKeyPressed(KEY_RIGHT))
        {
            if (!tree->active->right)
                tree->active->right = NewNode(tree, tree->active);
            tree->active = tree->active->right;
            UpdateTree(tree);
        }

        if (IsKeyPressed(KEY_UP))
        {
            if (tree->active->parent)
                tree->active = tree->active->parent;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTree(tree);
        EndDrawing();
    }

    FreeTree(tree);
    return 0;
}

BSP_Tree *
NewTree()
{
    BSP_Node *root = (BSP_Node *)malloc(sizeof(BSP_Node));
    root->depth = 0;

    BSP_Tree *tree = (BSP_Tree *)malloc(sizeof(BSP_Tree));
    tree->root = root;
    tree->active = root;
    tree->size = 1;
    tree->height = 1;
    UpdateTree(tree);

    return tree;
}

void
FreeTree(BSP_Tree *tree)
{
    FreeNode(tree->root);
    free(tree);
}

void
UpdateTree(BSP_Tree *tree)
{
    f32 radius = (f32)WIDTH / (tree->size * 2.0f);
    BSP_Node *node = MinNode(tree->root);
    u32 i = 0;
    while (node)
    {
        f32 x = ((f32)i / tree->size) * WIDTH + radius;
        f32 y = radius + node->depth * ((f32)HEIGHT / tree->height);
        /* u32 y = radius * (2 * node->depth + 1); */

        node->pos = (Vector2){ x, y };
        node->radius = radius;
        node = SuccNode(node);
        i++;
    }
}

void
DrawTree(BSP_Tree *tree)
{
    DrawNode(tree->root);
    DrawCircle(tree->active->pos.x, tree->active->pos.y, tree->active->radius, RED);
}

BSP_Node *
NewNode(BSP_Tree *tree, BSP_Node *parent)
{
    BSP_Node *child = (BSP_Node *)malloc(sizeof(BSP_Node));
    child->parent = parent;
    child->depth = parent->depth + 1;

    tree->size += 1;
    if (child->depth == tree->height)
        tree->height += 1;

    return child;
}

void
FreeNode(BSP_Node *node)
{
    if (node)
    {
        FreeNode(node->left);
        FreeNode(node->right);
        free(node);
    }
}

void
DrawNode(BSP_Node *node)
{
    if (node->left)
    {
        DrawLineEx(node->pos, node->left->pos, 1.0f, BLACK);
        DrawNode(node->left);
    }
    if (node->right)
    {
        DrawLineEx(node->pos, node->right->pos, 1.0f, BLACK);
        DrawNode(node->right);
    }
    DrawCircle(node->pos.x, node->pos.y, node->radius, BLUE);
}

BSP_Node *
MinNode(BSP_Node *r)
{
    BSP_Node *node = r;
    while (node->left)
        node = node->left;
    return node;
}

BSP_Node *
MaxNode(BSP_Node *r)
{
    BSP_Node *node = r;
    while (node->right)
        node = node->right;
    return node;
}

BSP_Node *
PrevNode(BSP_Node *node)
{
    if (node->left)
        return MaxNode(node->left);

    BSP_Node *tmp = node;
    while (tmp->parent && tmp == tmp->parent->left)
        tmp = tmp->parent;

    return tmp->parent;
}

BSP_Node *
SuccNode(BSP_Node *node)
{
    if (node->right)
        return MinNode(node->right);

    BSP_Node *tmp = node;
    while (tmp->parent && tmp == tmp->parent->right)
        tmp = tmp->parent;

    return tmp->parent;
}
