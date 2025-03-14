#include "bsp_utils.h"

inline usize
mod(isize i, isize n)
{
    i32 result = (i % n + n) % n;
    return result;
}
