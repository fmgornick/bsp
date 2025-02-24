#include "bsp_tree.h"
#include <stdlib.h>

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
    u32 shorter = min(WIDTH, HEIGHT);
    f32 radius = (f32)shorter / max(tree->height * 2, tree->size);

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
