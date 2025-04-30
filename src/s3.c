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
void DrawSceneReverse(BspNode *node, Player p);
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

    scene->helpButton = (Vector2){ WIDTH - 40, HEIGHT - 40 };
    scene->helpMenu = true;
    scene->useBspTree = true;
    scene->initialized = true;
    FreeSegments(segments);

    return S3_PENDING;
}

BspStage
S3_Render(S3 *scene)
{
    f32 movementMultiplier = 250.0f;
    f32 rotationMultiplier = 25.0f;
    f32 fovMultiplier = 50.0f;

    if (IsKeyPressed(KEY_H)) scene->helpMenu = !scene->helpMenu;
    else if (HelpButtonClicked(scene->helpButton)) scene->helpMenu = !scene->helpMenu;
    else if (GetKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) scene->helpMenu = false;

    if (IsKeyDown(KEY_W)) PlayerMove(&scene->player, Vector2Scale(scene->player.dir, 0.01f * movementMultiplier));
    if (IsKeyDown(KEY_A)) PlayerMove(&scene->player, Vector2Scale(scene->player.ldir, -0.01f * movementMultiplier));
    if (IsKeyDown(KEY_S)) PlayerMove(&scene->player, Vector2Scale(scene->player.dir, -0.01f * movementMultiplier));
    if (IsKeyDown(KEY_D)) PlayerMove(&scene->player, Vector2Scale(scene->player.ldir, 0.01f * movementMultiplier));
    if (IsKeyDown(KEY_LEFT)) PlayerRotate(&scene->player, -0.001f * rotationMultiplier);
    if (IsKeyDown(KEY_RIGHT)) PlayerRotate(&scene->player, 0.001f * rotationMultiplier);
    if (IsKeyDown(KEY_UP)) PlayerUpdateFov(&scene->player, 0.0001f * fovMultiplier);
    if (IsKeyDown(KEY_DOWN)) PlayerUpdateFov(&scene->player, -0.0001f * fovMultiplier);
    if (IsKeyPressed(KEY_SPACE)) scene->useBspTree = !scene->useBspTree;

    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawRectangle(0, HEIGHT / 2, WIDTH, HEIGHT / 2, LIGHTGRAY);
    if (scene->useBspTree) DrawScene(scene->tree, scene->player);
    else DrawSceneReverse(scene->tree, scene->player);
    DrawMinimap(scene);

    if (scene->helpMenu) DrawHelpMenu(S3_HELP_MENU, 7);
    DrawHelpMenuButton(scene->helpButton);

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
    free(scene->minimap);
    free(scene->colors);
    FreeBspTree(scene->tree);
    scene->initialized = false;
    *scene = (S3){ 0 };
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
            .origin = (Vector2){ node->segments[i].left.x, node->segments[i].left.y },
            .dest = (Vector2){ node->segments[i].right.x, node->segments[i].right.y },
        };
        DrawWall(p, segment, 100.0f * p.vfov, node->color);
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
DrawSceneReverse(BspNode *node, Player p)
{
    if (node == NULL) return;
    else if (IsLeaf(node)) DrawNode(node, p);
    else if (DSegmentSide(node->segments[0], (DVector2){ p.pos.x, p.pos.y }) == DSideRight)
    {
        DrawSceneReverse(node->left, p);
        DrawNode(node, p);
        DrawSceneReverse(node->right, p);
    }
    else if (DSegmentSide(node->segments[0], (DVector2){ p.pos.x, p.pos.y }) == DSideLeft)
    {
        DrawSceneReverse(node->right, p);
        DrawNode(node, p);
        DrawSceneReverse(node->left, p);
    }
    else
    {
        DrawSceneReverse(node->right, p);
        DrawSceneReverse(node->left, p);
    }
}

void
DrawMinimap(S3 *scene)
{
    BoundingRegion region = scene->minimapRegion;
    usize minimapWidth = region.right - region.left;
    usize minimapHeight = region.bottom - region.top;
    Rectangle rec = { region.left, region.top, minimapWidth, minimapHeight };
    DrawRectangleRec(rec, RAYWHITE);
    DrawRectangleLinesEx(rec, 3.0f, BLACK);

    for (usize i = 0; i < scene->numSegments; i++)
        DrawLineEx(scene->minimap[i].origin, scene->minimap[i].dest, 3.0f, scene->colors[i]);

    Vector2 p = TranslatePoint(scene->player.pos, region);
    Vector2 pp = Vector2Subtract(p, Vector2Scale(scene->player.dir, 10.0f));
    DrawTriangle(p, Vector2Subtract(pp, Vector2Scale(scene->player.ldir, 5.0f)), Vector2Add(pp, Vector2Scale(scene->player.ldir, 5.0f)), RED);

    { /*
       * find region enclosed by vewport in minimap
       * triangulate this region and shade it so the user knows what should be visible
       */
        FSegment left = TranslateSegment(scene->player.left, region);
        FSegment right = TranslateSegment(scene->player.right, region);
        left.dest = Vector2Add(left.dest, Vector2Scale(Vector2Subtract(left.dest, left.origin), 10.0f));
        right.dest = Vector2Add(right.dest, Vector2Scale(Vector2Subtract(right.dest, right.origin), 10.0f));
        FSegment regions[4] = {
            (FSegment){ (Vector2){ region.left, region.top }, (Vector2){ region.right, region.top } },
            (FSegment){ (Vector2){ region.right, region.top }, (Vector2){ region.right, region.bottom } },
            (FSegment){ (Vector2){ region.right, region.bottom }, (Vector2){ region.left, region.bottom } },
            (FSegment){ (Vector2){ region.left, region.bottom }, (Vector2){ region.left, region.top } },
        };
        Vector2 points[5];
        usize i, numPoints = 0;
        for (i = 0; i < 4; i++)
        {
            if (FSegmentsIntersect(right, regions[i]))
            {
                points[numPoints++] = FSegmentIntersection(right, regions[i]);
                break;
            }
        }
        assert(numPoints == 1);
        for (usize jj = 0; jj < 4; jj++)
        {
            usize j = mod(i + jj, 4);
            if (FSegmentsIntersect(left, regions[j]))
            {
                points[numPoints++] = FSegmentIntersection(left, regions[j]);
                break;
            }
            else points[numPoints++] = regions[j].dest;
        }
        for (usize k = 0; k < numPoints - 1; k++)
            DrawTriangle(p, points[k + 1], points[k], Fade(BLUE, 0.5));
    }
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

    if (playerLeftShapeOriginSide == FSideLeft && playerLeftShapeDestSide == FSideRight)
    {
        Vector2 intersection = FSegmentIntersection(p.left, s);
        if (Vector2DotProduct(p.dir, Vector2Subtract(intersection, p.pos)) < 0) return;
        else sp.origin = intersection;
    }
    else if (playerLeftShapeOriginSide == FSideRight && playerLeftShapeDestSide == FSideLeft)
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
    else if (playerRightShapeOriginSide == FSideRight && playerRightShapeDestSide == FSideLeft)
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
    // if (color.r == 102) printf("SDJKFHSDFKLJH\n");
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
