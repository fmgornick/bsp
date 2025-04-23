#ifndef S3_H_
#define S3_H_

#include "bsp.h"
#include "bsp_tree.h"
#include "f32_segment.h"

typedef struct Player {
    Vector2 pos;
    Vector2 dir;
    Vector2 ldir;
    FSegment left;
    FSegment right;
    f32 distance;
    f32 hfov;
    f32 vfov;
} Player;

typedef struct S3 {
    Player player;
    usize numSegments;
    Color *colors;
    FSegment *minimap;
    BoundingRegion minimapRegion;
    BspNode *tree;
    Vector2 helpButton;
    bool useBspTree;
    bool initialized;
    bool helpMenu;
} S3;

/* clang-format off */
static const char S3_HELP_MENU[7][128] = {
    "- toggle help menu : H",
    "- quit demonstration : Q",
    "- restart from beginning : R",
    "- move character w.r.t. viewing direction : W/A/S/D",
    "- rotate viewing direction : LEFT/RIGHT",
    "- increase/decrease field of view : UP/DOWN",
    "- toggle on/off BSP Tree rendering : SPACE",
};
/* clang-format on */

BspStage S3_Init(IVector2 *polygon, usize numVertices, S3 *scene);
BspStage S3_Render(S3 *scene);
BspStage S3_RenderFailure(S3 *scene);
void S3_Free(S3 *scene);

Player PlayerInit(Vector2 pos, Vector2 dir, f32 hfov);
void PlayerUpdate(Player *p);
void PlayerMove(Player *p, Vector2 dir);
void PlayerRotate(Player *p, f32 angle);
void PlayerUpdateFov(Player *p, f32 dt);

// void DrawScene(S3 *scene);
void DrawMinimap(S3 *scene);

#endif // S3_H_
