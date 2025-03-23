#ifndef F64_VECTOR_H_
#define F64_VECTOR_H_

#include "bsp.h"
#include <stdbool.h>

typedef struct DVector2 {
    f64 x;
    f64 y;
} DVector2;

DVector2 DVector2Add(DVector2 v1, DVector2 v2);
DVector2 DVector2AddValue(DVector2 v, float add);
DVector2 DVector2Subtract(DVector2 v1, DVector2 v2);
DVector2 DVector2SubtractValue(DVector2 v, float sub);

DVector2 DVector2Scale(DVector2 v, float scale);
DVector2 DVector2Multiply(DVector2 v1, DVector2 v2);
DVector2 DVector2Divide(DVector2 v1, DVector2 v2);

f64 DVector2DotProduct(DVector2 v1, DVector2 v2);
f64 DVector2Determinant(DVector2 v1, DVector2 v2);

bool DVector2DIsEqual(DVector2 v1, DVector2 v2);
DVector2 DVector2Negate(DVector2 v);

#endif // F64_VECTOR_H_
