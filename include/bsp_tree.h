#ifndef BSP_TREE_H_
#define BSP_TREE_H_

#include "bsp_utils.h"
#include "raylib.h"

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

#endif // BSP_TREE_H_
