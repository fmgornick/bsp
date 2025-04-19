#include "s3.h"
#include "bsp.h"
#include "bsp_tree.h"
#include "f32_segment.h"
#include "raylib.h"
#include "raymath.h"
#include "region.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* *********************** helpers *********************** */
/* ******************************************************* */
Player PlayerInit(Vector2 pos, Vector2 dir, f32 hfov);
void PlayerMove(Player *p, Vector2 dir);
void PlayerRotate(Player *p, f32 angle);
void PlayerUpdate(Player *p);
void DrawWall(Player p, FSegment s, f32 height, Color color);
/* ******************************************************* */
/* ******************************************************* */

BspStage
S3_Init(const DSegment *segments, usize numSegments, BspTreeMeta *tree, S3 *scene)
{
    scene->numSegments = numSegments;
    scene->segments = malloc(numSegments * sizeof(FSegment));
    for (usize i = 0; i < numSegments; i++)
        scene->segments[i] = (FSegment){
            .origin = (Vector2){ segments[i].left.x, segments[i].left.y },
            .dest = (Vector2){ segments[i].right.x, segments[i].right.y },
        };

    scene->tree = (BspTreeMeta *)malloc(sizeof(BspTreeMeta));
    CopyBspTree(tree, scene->tree);

    {
        usize numInnerRegions = 0;
        { /* calculate number of inner regions */
            for (usize i = 0; i < tree->size; i++)
            {
                BspNode *node = tree->meta[i].node;
                if (IsLeaf(node) && ((node->numSegments == 0 && IsRightChild(node)) || (node->numSegments > 0))) numInnerRegions += 1;
            }
        }

        Region **innerRegions = (Region **)malloc(numInnerRegions * sizeof(Region *));
        { /* find all inner regions */
            usize regionIdx = 0;
            for (usize i = 0; i < tree->size; i++)
            {
                BspNode *node = tree->meta[i].node;
                if (IsLeaf(node))
                {
                    if (node->numSegments == 0 && IsRightChild(node))
                        innerRegions[regionIdx++]
                            = NewRegion(tree->meta[idxParent(tree, i)].region, node->parent->segments, node->parent->numSegments, SplitRight);
                    else if (node->numSegments > 0) innerRegions[regionIdx++] = NewRegion(tree->meta[i].region, NULL, 0, SplitRight);
                }
            }
        }

        Vector2 randomCentroid;
        { /*pick random inner region and find it's centroid */
            int randIdx = rand() % numInnerRegions;
            Region *region = innerRegions[randIdx];
            f32 xSum = 0.0f, ySum = 0.0f;
            for (usize i = 0; i < region->boundarySize; i++)
            {
                xSum += region->boundary[i].left.x;
                ySum += region->boundary[i].left.y;
            }
            randomCentroid = (Vector2){
                .x = xSum / region->boundarySize,
                .y = ySum / region->boundarySize,
            };
        }

        { /* set player initial position to random region centroid */
            Vector2 initialDir = { 0.0f, -1.0f };
            f32 hfov = PI / 4.0f;
            scene->player = PlayerInit(randomCentroid, initialDir, hfov);
        }
    }

    scene->initialized = true;

    return S3_PENDING;
}

BspStage
S3_Render(S3 *scene)
{
    if (IsKeyDown(KEY_W)) PlayerMove(&scene->player, Vector2Scale(scene->player.dir, 0.01f));
    if (IsKeyDown(KEY_A)) PlayerMove(&scene->player, Vector2Scale(scene->player.ldir, 0.01f));
    if (IsKeyDown(KEY_S)) PlayerMove(&scene->player, Vector2Scale(scene->player.dir, -0.01f));
    if (IsKeyDown(KEY_D)) PlayerMove(&scene->player, Vector2Scale(scene->player.ldir, -0.01f));
    if (IsKeyDown(KEY_LEFT)) PlayerRotate(&scene->player, 0.001f);
    if (IsKeyDown(KEY_RIGHT)) PlayerRotate(&scene->player, -0.001f);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (usize i = 0; i < scene->numSegments; i++)
        DrawWall(scene->player, scene->segments[i], 20.0f, colors[i % numColors]);
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
    FreeBspTreeMeta(scene->tree);
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
    p->pos = Vector2Add(p->pos, dir);
    PlayerUpdate(p);
}

void
PlayerRotate(Player *p, f32 angle)
{
    p->dir = Vector2Rotate(p->dir, angle);
    p->ldir = (Vector2){ -p->dir.y, p->dir.x };
    PlayerUpdate(p);
}

void
DrawWall(Player p, FSegment s, f32 height, Color color)
{
    FSegment sp = s;
    bool segmentVisible = true;
    bool leftSideInView = FSegmentSide(p.right, sp.origin) == FSideLeft && FSegmentSide(p.left, sp.origin) == FSideRight;
    bool rightSideInView = FSegmentSide(p.right, sp.dest) == FSideLeft && FSegmentSide(p.left, sp.dest) == FSideRight;
    if (leftSideInView && !rightSideInView) sp.dest = FSegmentIntersection(p.left, sp);
    else if (!leftSideInView && rightSideInView) sp.origin = FSegmentIntersection(p.right, sp);
    if (!leftSideInView && !rightSideInView && FSegmentsIntersect(sp, p.left))
    {
        sp.dest = FSegmentIntersection(p.left, sp);
        sp.origin = FSegmentIntersection(p.right, sp);
    }
    else if (!leftSideInView && !rightSideInView) segmentVisible = false;
    if (segmentVisible)
    {
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
        Vector2 x = { projOrigin, HEIGHT / 2.0f + heightOrigin };
        Vector2 y = { projDest, HEIGHT / 2.0f + heightDest };
        Vector2 z = { projDest, HEIGHT / 2.0f - heightDest };
        Vector2 w = { projOrigin, HEIGHT / 2.0f - heightOrigin };
        DrawTriangle(x, z, y, color);
        DrawTriangle(x, w, z, color);
    }
}
