#include "f64_vector.h"
#include <math.h>

DVector2
DVector2Add(DVector2 v1, DVector2 v2)
{
    DVector2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

DVector2
DVector2AddValue(DVector2 v, float add)
{
    DVector2 result = { v.x + add, v.y + add };
    return result;
}

DVector2
DVector2Subtract(DVector2 v1, DVector2 v2)
{
    DVector2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

DVector2
DVector2SubtractValue(DVector2 v, float sub)
{
    DVector2 result = { v.x - sub, v.y - sub };
    return result;
}

DVector2
DVector2Scale(DVector2 v, float scale)
{
    DVector2 result = { v.x * scale, v.y * scale };
    return result;
}

DVector2
DVector2Multiply(DVector2 v1, DVector2 v2)
{
    DVector2 result = { v1.x * v2.x, v1.y * v2.y };
    return result;
}

DVector2
DVector2Divide(DVector2 v1, DVector2 v2)
{
    DVector2 result = { v1.x / v2.x, v1.y / v2.y };
    return result;
}

f64
DVector2DotProduct(DVector2 v1, DVector2 v2)
{
    f64 result = (v1.x * v2.x + v1.y * v2.y);
    return result;
}

f64
DVector2Determinant(DVector2 v1, DVector2 v2)
{
    f64 result = (v1.x * v2.y - v1.y * v2.x);
    return result;
}

bool
DVector2DIsEqual(DVector2 v1, DVector2 v2)
{
    return babs((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y)) < BSP_EPSILON;
}

DVector2
DVector2Negate(DVector2 v)
{
    DVector2 result = { -v.x, -v.y };
    return result;
}

f64
DVector2Length(DVector2 v)
{
    f64 result = sqrt(v.x * v.x + v.y * v.y);
    return result;
}

f64
DVector2LineAngle(DVector2 u, DVector2 v)
{
    return acos(DVector2DotProduct(u, v));
}

DVector2
DVector2Normalize(DVector2 v)
{
    DVector2 result = { 0 };
    float length = sqrt((v.x * v.x) + (v.y * v.y));
    if (length > 0)
    {
        float ilength = 1.0f / length;
        result.x = v.x * ilength;
        result.y = v.y * ilength;
    }
    return result;
}

DVector2
DVector2Rotate(DVector2 v, f64 angle)
{
    DVector2 result = { 0 };
    f64 cosres = cos(angle);
    f64 sinres = sin(angle);
    result.x = v.x * cosres - v.y * sinres;
    result.y = v.x * sinres + v.y * cosres;
    return result;
}
