#include "i32_vector.h"

IVector2
IVector2Add(IVector2 v1, IVector2 v2)
{
    IVector2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

IVector2
IVector2AddValue(IVector2 v, float add)
{
    IVector2 result = { v.x + add, v.y + add };
    return result;
}

IVector2
IVector2Subtract(IVector2 v1, IVector2 v2)
{
    IVector2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

IVector2
IVector2SubtractValue(IVector2 v, float sub)
{
    IVector2 result = { v.x - sub, v.y - sub };
    return result;
}

IVector2
IVector2Scale(IVector2 v, float scale)
{
    IVector2 result = { v.x * scale, v.y * scale };
    return result;
}

IVector2
IVector2Multiply(IVector2 v1, IVector2 v2)
{
    IVector2 result = { v1.x * v2.x, v1.y * v2.y };
    return result;
}

IVector2
IVector2Divide(IVector2 v1, IVector2 v2)
{
    IVector2 result = { v1.x / v2.x, v1.y / v2.y };
    return result;
}

i32
IVector2DotProduct(IVector2 v1, IVector2 v2)
{
    i32 result = (v1.x * v2.x + v1.y * v2.y);
    return result;
}

i32
IVector2Determinant(IVector2 v1, IVector2 v2)
{
    i32 result = (v1.x * v2.y - v1.y * v2.x);
    return result;
}

bool
IVector2DIsEqual(IVector2 v1, IVector2 v2)
{
    return (v1.x == v2.x && v1.y == v2.y);
}

IVector2
IVector2Negate(IVector2 v)
{
    IVector2 result = { -v.x, -v.y };
    return result;
}
