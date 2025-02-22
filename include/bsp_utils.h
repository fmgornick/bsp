#ifndef BSP_UTILS_H_
#define BSP_UTILS_H_

#define WIDTH 1280
#define HEIGHT 720
#define MAX_VERTICES 24

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SAME_SIGN(x, y) ((x) <= 0) == ((y) <= 0)

typedef int i32;
typedef long long int i64;
typedef int isize;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef unsigned int usize;
typedef float f32;
typedef double f64;

#endif // BSP_UTILS_H_
