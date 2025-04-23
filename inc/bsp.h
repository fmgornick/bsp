#ifndef BSP_H_
#define BSP_H_

#include "raylib.h"
#include <stdlib.h>

#define WIDTH 1280
#define HEIGHT 720
#define MAX_VERTICES 24
#define BSP_EPSILON 0.000001f

#define babs(x) ((x) < 0 ? -(x) : (x))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(a, lower, upper) min(max(a, lower), upper)
#define sign(v) (((v) > 0) - ((v) < 0))

typedef int i32;
typedef long long int i64;
typedef int isize;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef unsigned int usize;
typedef float f32;
typedef double f64;

static usize numColors = 18;
static Color colors[18] = {
    YELLOW,  GOLD, ORANGE,   PINK,   RED,    MAROON,     GREEN, LIME,  DARKGREEN, //
    SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE, BROWN, DARKBROWN, //
};

typedef enum BspStage {
    S1_INITIALIZING,
    S1_PENDING,
    S1_FAILED,
    S1_COMPLETED,

    S2_INITIALIZING,
    S2_PENDING,
    S2_FAILED,
    S2_COMPLETED,

    S3_INITIALIZING,
    S3_PENDING,
    S3_FAILED,
    S3_COMPLETED,
} BspStage;

typedef struct BoundingRegion {
    u32 left;
    u32 right;
    u32 top;
    u32 bottom;
} BoundingRegion;

static inline usize
mod(isize i, isize n)
{
    i32 result = (i % n + n) % n;
    return result;
}

static inline f32
rand_f32(f32 min, f32 max)
{
    return min + (f32)rand() / (f32)RAND_MAX * (max - min);
}

static inline void
DrawMessage(char *msg, Color fg, Color bg)
{
    DrawRectangle(0, 3 * (HEIGHT / 4), WIDTH, HEIGHT / 4, Fade(bg, 0.5));
    Vector2 textSize = MeasureTextEx(GetFontDefault(), msg, 20, 1.0f);
    u32 xPos = (WIDTH - textSize.x) / 2;
    u32 yPos = 3 * (HEIGHT / 4.0f) + (HEIGHT / 4.0f - textSize.y) / 2;
    DrawText(msg, xPos, yPos, 20, fg);
}

static inline void
DrawHelpMenu(const char (*commands)[128], usize numCommands)
{
    u32 helpMenuSize = MeasureText("HELP MENU", 40);
    u32 commandSize = 0;
    for (usize i = 0; i < numCommands; i++)
        commandSize = max(commandSize, MeasureText(commands[i], 20));
    u32 height = 60 + 30 * numCommands;
    u32 width = 20 + commandSize;
    f32 x = (f32)(WIDTH - width) / 2;
    f32 y = (f32)(HEIGHT - height) / 2;

    Rectangle rec = { x, y, width, height };
    DrawRectangleRec(rec, RAYWHITE);
    DrawRectangleLinesEx(rec, 3.0f, BLACK);

    u32 xPos = rec.x + (rec.width - helpMenuSize) / 2;
    u32 yPos = rec.y + 10;
    DrawText("HELP MENU", xPos, yPos, 40, BLACK);

    xPos = rec.x + (rec.width - commandSize) / 2;
    yPos += 50;
    for (usize i = 0; i < numCommands; i++)
    {
        DrawText(commands[i], xPos, yPos, 20, BLACK);
        yPos += 30;
    }
}

static inline void
DrawHelpMenuButton(Vector2 pos)
{
    DrawCircle(pos.x, pos.y, 23, BLACK);
    DrawCircle(pos.x, pos.y, 20, RAYWHITE);
    DrawText("?", pos.x - 12, pos.y - 16, 40, BLACK);
}

static inline bool
HelpButtonClicked(Vector2 pos)
{
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;
    else return CheckCollisionPointCircle(GetMousePosition(), pos, 23);
}

static inline f32
Vector2Determinant(Vector2 v1, Vector2 v2)
{
    f32 result = (v1.x * v2.y - v1.y * v2.x);
    return result;
}

#endif // BSP_H_
