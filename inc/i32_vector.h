#ifndef I32_VECTOR_H_
#define I32_VECTOR_H_

#include "bsp_utils.h"
#include <stdbool.h>

typedef struct IVector2 {
    i32 x;
    i32 y;
} IVector2;
typedef IVector2 IPoint2;

IVector2 IVector2Add(IVector2 v1, IVector2 v2);
IVector2 IVector2AddValue(IVector2 v, float add);
IVector2 IVector2Subtract(IVector2 v1, IVector2 v2);
IVector2 IVector2SubtractValue(IVector2 v, float sub);

IVector2 IVector2Scale(IVector2 v, float scale);
IVector2 IVector2Multiply(IVector2 v1, IVector2 v2);
IVector2 IVector2Divide(IVector2 v1, IVector2 v2);

i32 IVector2DotProduct(IVector2 v1, IVector2 v2);
i32 IVector2Determinant(IVector2 v1, IVector2 v2);

bool IVector2DIsEqual(IVector2 v1, IVector2 v2);
IVector2 IVector2Negate(IVector2 v);

#endif // I32_VECTOR_H_
