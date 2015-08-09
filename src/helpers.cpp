#include "helpers.h"

#include <LinearMath/btScalar.h>

btScalar WrapAngle(const btScalar value)
{
    int i = static_cast<int>(value / 360.0);
    i = (value < 0.0)?(i-1):(i);
    return value - 360.0f * i;
}
