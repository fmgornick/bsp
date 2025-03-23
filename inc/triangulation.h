#ifndef TRIANGULATION_H_
#define TRIANGULATION_H_

#include "dcel.h"

typedef struct Triangle {
    Vector2 v1;
    Vector2 v2;
    Vector2 v3;
} Triangle;

Triangle *MonotoneTriangulation(Segment *segments, usize numSegments);
void MonotoneTriangulateDCEL(DCEL *polygon);

#endif // TRIANGULATION_H_
