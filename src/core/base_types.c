
#include <stdlib.h>
#include "base_types.h"

engine_container_p Container_Create()
{
    engine_container_p ret;

    ret = (engine_container_p)malloc(sizeof(engine_container_t));
    ret->collision_shape = 0;
    ret->collision_heavy = 0x00;
    ret->collision_group = COLLISION_GROUP_KINEMATIC;
    ret->collision_mask = COLLISION_MASK_ALL;
    ret->next = NULL;
    ret->object = NULL;
    ret->room = NULL;
    ret->sector = NULL;
    ret->object_type = 0;
    return ret;
}

void Container_Delete(engine_container_p cont)
{
    free(cont);
}
