#ifndef TRIANGULATION_H_
#define TRIANGULATION_H_

#include "dcel.h"
#include "raylib.h"

typedef struct Triangle {
    Vector2 v1;
    Vector2 v2;
    Vector2 v3;
} Triangle;

Triangle *MonotoneTriangulation(DSegment *segments, usize numSegments);
void MonotoneTriangulateDCEL(DCEL *polygon);
Vector2 TriangleCenter(Triangle t);

#endif // TRIANGULATION_H_
