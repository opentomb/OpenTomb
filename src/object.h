#ifndef OBJECT_H
#define OBJECT_H

#include <memory>

struct Object : public std::enable_shared_from_this<Object>
{
    virtual ~Object()
    {
    }
};

#endif // OBJECT_H

