
#ifndef HELPERS_H
#define HELPERS_H

#include <cmath>
#include <LinearMath/btScalar.h>

#define Clamp(v,l,h) ((v<l)?l:((v>h)?h:v))

btScalar WrapAngle(const btScalar value);

#endif  // HELPERS_H
