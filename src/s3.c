#include "s3.h"
#include "bsp.h"
#include "bsp_tree.h"
#include "f32_segment.h"
#include "f64_segment.h"
#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ************************** helpers ************************** */
/* ************************************************************* */
void DrawWall(Player p, FSegment s, f32 height, Color color);
void DrawNode(BspNode *node, Player p);
void DrawScene(BspNode *node, Player p);
Vector2 TranslatePoint(Vector2 pt, BoundingRegion region);
FSegment TranslateSegment(FSegment segment, BoundingRegion region);
/* ************************************************************* */
/* ************************************************************* */

BspStage
S3_Init(IVector2 *polygon, usize numVertices, S3 *scene)
{
    BoundingRegion fullScreen = { 0, WIDTH, 0, HEIGHT };
    usize numSegments = 0;
    DSegment *segments = BuildSegments(polygon, numVertices, fullScreen, &numSegments);
    DSegment *segmentsCopy = (DSegment *)malloc(numSegments * sizeof(DSegment));
    memcpy(segmentsCopy, segments, numSegments * sizeof(DSegment));

    scene->minimapRegion = (BoundingRegion){ 2 * WIDTH / 3, WIDTH, 0, HEIGHT / 3 };
    scene->minimap = BuildFSegments(segmentsCopy, numSegments, scene->minimapRegion, &scene->numSegments);
    scene->tree = BuildBspTree(segmentsCopy, numSegments, NULL);
    scene->player = PlayerInit((Vector2){ WIDTH / 2.0f, HEIGHT / 2.0f }, (Vector2){ 0.0f, -1.0f }, PI / 6.0f);
    scene->colors = (Color *)malloc(numSegments * sizeof(Color));
    for (usize i = 0; i < numSegments; i++)
        scene->colors[i] = BLANK;

    usize idx = 0;
    for (BspNode *node = MinNode(scene->tree); node != NULL; node = SuccNode(node))
    {
        if (node->numSegments > 0)
        {
            for (usize i = 0; i < numSegments; i++)
            {
                if (DSegmentSides(segments[i], node->segments[0]) == DSideInside)
                {
                    if (scene->colors[i].a == 0) scene->colors[i] = colors[idx % numColors];
                    node->color = scene->colors[i];
                    idx += 1;
                    break;
                }
            }
        }
    }

    scene->useBspTree = true;
    scene->initialized = true;

    return S3_PENDING;
}

BspStage
S3_Render(S3 *scene)
{
#ifdef WASM
    f32 movementMultiplier = 400.0f;
    f32 rotationMultiplier = 100.0f;
#else
    f32 movementMultiplier = 4.0f;
    f32 rotationMultiplier = 1.0f;
#endif
    if (IsKeyDown(KEY_W)) PlayerMove(&scene->player, Vector2Scale(scene->player.dir, 0.01f * movementMultiplier));
    if (IsKeyDown(KEY_A)) PlayerMove(&scene->player, Vector2Scale(scene->player.ldir, -0.01f * movementMultiplier));
    if (IsKeyDown(KEY_S)) PlayerMove(&scene->player, Vector2Scale(scene->player.dir, -0.01f * movementMultiplier));
    if (IsKeyDown(KEY_D)) PlayerMove(&scene->player, Vector2Scale(scene->player.ldir, 0.01f * movementMultiplier));
    if (IsKeyDown(KEY_LEFT)) PlayerRotate(&scene->player, -0.001f * rotationMultiplier);
    if (IsKeyDown(KEY_RIGHT)) PlayerRotate(&scene->player, 0.001f * rotationMultiplier);
    if (IsKeyDown(KEY_UP)) PlayerUpdateFov(&scene->player, 0.0001f);
    if (IsKeyDown(KEY_DOWN)) PlayerUpdateFov(&scene->player, -0.0001f);
    if (IsKeyPressed(KEY_SPACE)) scene->useBspTree = scene->useBspTree ^ true;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (scene->useBspTree) DrawScene(scene->tree, scene->player);
    else
    {
        for (BspNode *node = MinNode(scene->tree); node != NULL; node = SuccNode(node))
            DrawNode(node, scene->player);
    }
    DrawMinimap(scene);
    EndDrawing();

    return S3_PENDING;
}

BspStage
S3_RenderFailure(S3 *scene)
{
    return S3_FAILED;
}

void
S3_Free(S3 *scene)
{
    // FreeSegments(scene->segments);
    // FreeBspTreeMeta(scene->tree);
    scene->initialized = false;
}

Player
PlayerInit(Vector2 pos, Vector2 dir, f32 hfov)
{
    assert(babs(Vector2Length(dir) - 1.0f) < BSP_EPSILON);
    assert(hfov > 0.0f);
    f32 distance = babs(WIDTH / (2.0f * tanf(hfov)));
    f32 vfov = atanf(HEIGHT / (2.0f * distance));
    Player p = {
        .pos = pos,
        .dir = dir,
        .ldir = { -dir.y, dir.x },
        .hfov = hfov,
        .vfov = vfov,
        .distance = distance,
    };
    PlayerUpdate(&p);
    return p;
}

void
PlayerUpdate(Player *p)
{
    Vector2 pos = p->pos;
    Vector2 dir = p->dir;
    Vector2 ldir = { -dir.y, dir.x };
    Vector2 ihat = Vector2Scale(dir, p->distance);
    Vector2 jhat = Vector2Scale(ldir, WIDTH / 2.0f);
    p->left.origin = pos;
    p->left.dest = Vector2Add(pos, Vector2Add(ihat, jhat));
    p->right.origin = pos;
    p->right.dest = Vector2Add(pos, Vector2Subtract(ihat, jhat));
}

void
PlayerMove(Player *p, Vector2 dir)
{
    Vector2 newPos = Vector2Add(p->pos, dir);
    if (newPos.x > 0 && newPos.x < WIDTH && newPos.y > 0 && newPos.y < HEIGHT)
    {
        p->pos = newPos;
        PlayerUpdate(p);
    }
}

void
PlayerRotate(Player *p, f32 angle)
{
    p->dir = Vector2Rotate(p->dir, angle);
    p->ldir = (Vector2){ -p->dir.y, p->dir.x };
    PlayerUpdate(p);
}

void
PlayerUpdateFov(Player *p, f32 dt)
{
    f32 hfov = p->hfov + dt;
    if ((hfov > 0.1) && (hfov < PI / 2.0f - 0.1))
    {
        p->hfov = hfov;
        p->distance = babs(WIDTH / (2.0f * tanf(p->hfov)));
        p->vfov = atanf(HEIGHT / (2.0f * p->distance));
        PlayerUpdate(p);
    }
}

void
DrawNode(BspNode *node, Player p)
{
    for (usize i = 0; i < node->numSegments; i++)
    {
        FSegment segment = {
            .origin = (Vector2){ node->segments[0].left.x, node->segments[0].left.y },
            .dest = (Vector2){ node->segments[0].right.x, node->segments[0].right.y },
        };
        DrawWall(p, segment, 30.0f, node->color);
    }
}

void
DrawScene(BspNode *node, Player p)
{
    if (node == NULL) return;
    else if (IsLeaf(node)) DrawNode(node, p);
    else if (DSegmentSide(node->segments[0], (DVector2){ p.pos.x, p.pos.y }) == DSideRight)
    {
        DrawScene(node->right, p);
        DrawNode(node, p);
        DrawScene(node->left, p);
    }
    else if (DSegmentSide(node->segments[0], (DVector2){ p.pos.x, p.pos.y }) == DSideLeft)
    {
        DrawScene(node->left, p);
        DrawNode(node, p);
        DrawScene(node->right, p);
    }
    else
    {
        DrawScene(node->left, p);
        DrawScene(node->right, p);
    }
}

void
DrawMinimap(S3 *scene)
{
    usize minimapWidth = scene->minimapRegion.right - scene->minimapRegion.left;
    usize minimapHeight = scene->minimapRegion.bottom - scene->minimapRegion.top;
    DrawRectangle(scene->minimapRegion.left, scene->minimapRegion.top, minimapWidth, minimapHeight, RAYWHITE);

    for (usize i = 0; i < scene->numSegments; i++)
        DrawLineEx(scene->minimap[i].origin, scene->minimap[i].dest, 3.0f, scene->colors[i]);

    Vector2 p = TranslatePoint(scene->player.pos, scene->minimapRegion);
    Vector2 pp = Vector2Subtract(p, Vector2Scale(scene->player.dir, 10.0f));
    DrawTriangle(p, Vector2Subtract(pp, Vector2Scale(scene->player.ldir, 5.0f)), Vector2Add(pp, Vector2Scale(scene->player.ldir, 5.0f)), RED);

    FSegment left = TranslateSegment(scene->player.left, scene->minimapRegion);
    FSegment right = TranslateSegment(scene->player.right, scene->minimapRegion);
    DrawLineEx(left.origin, left.dest, 2.0f, BLACK);
    DrawLineEx(right.origin, right.dest, 2.0f, BLACK);
}

void
DrawWall(Player p, FSegment s, f32 height, Color color)
{
    FSegment sp = s;
    if (Vector2Determinant(p.dir, Vector2Subtract(s.dest, s.origin)) < 0.0f)
    {
        sp.origin = s.dest;
        sp.dest = s.origin;
    }

    Side playerLeftShapeOriginSide = FSegmentSide(p.left, sp.origin);
    Side playerLeftShapeDestSide = FSegmentSide(p.left, sp.dest);
    if (playerLeftShapeOriginSide != FSideRight && playerLeftShapeDestSide != FSideRight) return;

    Side playerRightShapeOriginSide = FSegmentSide(p.right, sp.origin);
    Side playerRightShapeDestSide = FSegmentSide(p.right, sp.dest);
    if (playerRightShapeOriginSide != FSideLeft && playerRightShapeDestSide != FSideLeft) return;

    if (Vector2DotProduct(p.dir, Vector2Subtract(sp.origin, p.pos)) < 0.0f && Vector2DotProduct(p.dir, Vector2Subtract(sp.dest, p.pos)) < 0.0f) return;

    // if (playerLeftShapeOriginSide == FSideLeft && playerLeftShapeDestSide == FSideRight) sp.origin = FSegmentIntersection(p.left, s);
    // if (playerLeftShapeOriginSide == FSideRight && playerLeftShapeDestSide == FSideLeft) sp.dest = FSegmentIntersection(p.left, s);
    // if (playerRightShapeOriginSide == FSideLeft && playerRightShapeDestSide == FSideRight) sp.dest = FSegmentIntersection(p.right, s);
    // if (playerRightShapeOriginSide == FSideRight && playerRightShapeDestSide == FSideLeft) sp.origin = FSegmentIntersection(p.right, s);

    if (playerLeftShapeOriginSide == FSideLeft && playerLeftShapeDestSide == FSideRight)
    {
        Vector2 intersection = FSegmentIntersection(p.left, s);
        if (Vector2DotProduct(p.dir, Vector2Subtract(intersection, p.pos)) < 0) return;
        else sp.origin = intersection;
    }
    if (playerLeftShapeOriginSide == FSideRight && playerLeftShapeDestSide == FSideLeft)
    {
        Vector2 intersection = FSegmentIntersection(p.left, s);
        if (Vector2DotProduct(p.dir, Vector2Subtract(intersection, p.pos)) < 0) return;
        else sp.dest = intersection;
    }
    if (playerRightShapeOriginSide == FSideLeft && playerRightShapeDestSide == FSideRight)
    {
        Vector2 intersection = FSegmentIntersection(p.right, s);
        if (Vector2DotProduct(p.dir, Vector2Subtract(intersection, p.pos)) < 0) return;
        else sp.dest = intersection;
    }
    if (playerRightShapeOriginSide == FSideRight && playerRightShapeDestSide == FSideLeft)
    {
        Vector2 intersection = FSegmentIntersection(p.right, s);
        if (Vector2DotProduct(p.dir, Vector2Subtract(intersection, p.pos)) < 0) return;
        else sp.origin = intersection;
    }

    FSegment plane = { .origin = p.left.dest, .dest = p.right.dest };
    FSegment origin = { .origin = p.pos, .dest = sp.origin };
    FSegment dest = { .origin = p.pos, .dest = sp.dest };
    Vector2 intersectOrigin = FSegmentIntersection(plane, origin);
    Vector2 intersectDest = FSegmentIntersection(plane, dest);
    f32 projOrigin = Vector2Length(Vector2Subtract(intersectOrigin, plane.origin));
    f32 projDest = Vector2Length(Vector2Subtract(intersectDest, plane.origin));
    f32 distanceOrigin = Vector2Length(Vector2Subtract(sp.origin, p.pos));
    f32 distanceDest = Vector2Length(Vector2Subtract(sp.dest, p.pos));
    f32 heightOrigin = (p.distance * height) / distanceOrigin;
    f32 heightDest = (p.distance * height) / distanceDest;
    Vector2 x = { WIDTH - projOrigin, HEIGHT / 2.0f + heightOrigin };
    Vector2 y = { WIDTH - projDest, HEIGHT / 2.0f + heightDest };
    Vector2 z = { WIDTH - projDest, HEIGHT / 2.0f - heightDest };
    Vector2 w = { WIDTH - projOrigin, HEIGHT / 2.0f - heightOrigin };
    DrawTriangle(x, y, z, color);
    DrawTriangle(x, z, w, color);
}

Vector2
TranslatePoint(Vector2 pt, BoundingRegion region)
{
    f32 newX = region.left + pt.x * ((f32)(region.right - region.left) / WIDTH);
    f32 newY = region.top + pt.y * ((f32)(region.bottom - region.top) / HEIGHT);
    return (Vector2){ newX, newY };
}

FSegment
TranslateSegment(FSegment segment, BoundingRegion region)
{
    return (FSegment){
        .origin = TranslatePoint(segment.origin, region),
        .dest = TranslatePoint(segment.dest, region),
    };
}
