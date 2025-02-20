#ifndef BSP_TEST_H_
#define BSP_TEST_H_

#include "bsp_utils.h"
#include "bsp_vector.h"

/* triangle */
IVector2 trianglePolygon[MAX_VERTICES] = {
    { .x = 17, .y = 27 }, { .x = 31, .y = 5 }, { .x = 46, .y = 27 }, { .x = 0, .y = 0 }, { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 }, { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 }, { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 }, { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
};
uint32_t triangleNumVertices = 3;

/* square */
IVector2 squarePolygon[MAX_VERTICES] = {
    { .x = 16, .y = 4 }, { .x = 16, .y = 29 }, { .x = 46, .y = 29 }, { .x = 46, .y = 4 }, { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },  { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },  { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },  { .x = 0, .y = 0 }, { .x = 0, .y = 0 },
};
uint32_t squareNumVertices = 4;

/* complex polygon, uses all available vertices */
IVector2 complexPolygon[MAX_VERTICES] = {
    { .x = 24, .y = 16 }, { .x = 14, .y = 2 },  { .x = 1, .y = 11 },  { .x = 12, .y = 24 }, { .x = 1, .y = 34 },  { .x = 19, .y = 33 },
    { .x = 12, .y = 29 }, { .x = 22, .y = 20 }, { .x = 24, .y = 35 }, { .x = 37, .y = 24 }, { .x = 56, .y = 27 }, { .x = 36, .y = 33 },
    { .x = 63, .y = 29 }, { .x = 53, .y = 18 }, { .x = 62, .y = 10 }, { .x = 49, .y = 10 }, { .x = 49, .y = 1 },  { .x = 44, .y = 16 },
    { .x = 50, .y = 21 }, { .x = 39, .y = 18 }, { .x = 35, .y = 2 },  { .x = 27, .y = 23 }, { .x = 29, .y = 4 },  { .x = 23, .y = 4 },
};
uint32_t complexNumVertices = 24;

/* convex polygon, uses lots of vertices, but no concavities */
IVector2 convexPolygon[MAX_VERTICES] = {
    { .x = 5, .y = 26 }, { .x = 12, .y = 33 }, { .x = 24, .y = 35 }, { .x = 49, .y = 34 }, { .x = 61, .y = 24 }, { .x = 62, .y = 11 },
    { .x = 52, .y = 1 }, { .x = 27, .y = 0 },  { .x = 11, .y = 1 },  { .x = 1, .y = 11 },  { .x = 0, .y = 21 },  { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },  { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
};
uint32_t convexNumVertices = 12;

/* normal polygon, fills texture width */
IVector2 fatPolygon[MAX_VERTICES] = {
    { .x = 18, .y = 19 }, { .x = 24, .y = 31 }, { .x = 40, .y = 26 }, { .x = 32, .y = 14 }, { .x = 28, .y = 24 }, { .x = 29, .y = 8 },
    { .x = 17, .y = 5 },  { .x = 22, .y = 12 }, { .x = 5, .y = 12 },  { .x = 12, .y = 28 }, { .x = 19, .y = 27 }, { .x = 11, .y = 16 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
};
uint32_t fatNumVertices = 12;

/* tall polygon, fills texture height */
IVector2 tallPolygon[MAX_VERTICES] = {
    { .x = 22, .y = 12 }, { .x = 17, .y = 26 }, { .x = 20, .y = 31 }, { .x = 22, .y = 21 }, { .x = 23, .y = 34 }, { .x = 28, .y = 22 },
    { .x = 23, .y = 2 },  { .x = 19, .y = 15 }, { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
    { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },   { .x = 0, .y = 0 },
};
uint32_t tallNumVertices = 8;

#endif // BSP_TEST_H_
