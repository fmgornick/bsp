#ifndef BSP_H_
#define BSP_H_

#include "raylib.h"

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

static inline void
DrawMessage(char *msg, Color fg, Color bg)
{
    DrawRectangle(0, 3 * (HEIGHT / 4), WIDTH, HEIGHT / 4, Fade(bg, 0.5));
    Vector2 textSize = MeasureTextEx(GetFontDefault(), msg, 20, 1.0f);
    u32 xPos = (WIDTH - textSize.x) / 2;
    u32 yPos = 3 * (HEIGHT / 4.0f) + (HEIGHT / 4.0f - textSize.y) / 2;
    DrawText(msg, xPos, yPos, 20, fg);
}

#endif // BSP_H_
