
#include <cmath>
#include <bullet/LinearMath/btScalar.h>

#include "helpers.h"

btScalar WrapAngle(const btScalar value)
{
    int i = value / 360.0;
    i = (value < 0.0)?(i-1):(i);
    return value - 360.0 * i;
}
