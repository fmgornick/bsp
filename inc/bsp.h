#ifndef BSP_H_
#define BSP_H_

#define WIDTH 1280
#define HEIGHT 720
#define MAX_VERTICES 24

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

#endif // BSP_H_
