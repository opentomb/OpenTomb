#include "ragdoll.h"

#include <cmath>

#include <bullet/LinearMath/btScalar.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/btBulletDynamicsCommon.h>

#include "vmath.h"
#include "character_controller.h"

#include "engine.h"
#include "entity.h"


bool RDSetup::getSetup(int ragdoll_index)
{
    bool result = true;

    int top = lua_gettop(engine_lua);
    assert(top >= 0);

    lua_getglobal(engine_lua, "getRagdollSetup");
    if(lua_isfunction(engine_lua, -1))
    {
        lua_pushinteger(engine_lua, ragdoll_index);
        if(lua_CallAndLog(engine_lua, 1, 1, 0))
        {
            if(lua_istable(engine_lua, -1))
            {
                lua_getfield(engine_lua, -1, "hit_callback");
                if(lua_isstring(engine_lua, -1))
                {
                    size_t string_length  = 0;
                    const char* func_name = lua_tolstring(engine_lua, -1, &string_length);

                    hit_func.assign(func_name+0, func_name+string_length);
                }
                else { result = false; }
                lua_pop(engine_lua, 1);

                joint_setup.resize( (uint32_t)lua_GetScalarField(engine_lua, "joint_count") );
                body_setup.resize( (uint32_t)lua_GetScalarField(engine_lua, "body_count") );

                joint_cfm   = lua_GetScalarField(engine_lua, "joint_cfm");
                joint_erp   = lua_GetScalarField(engine_lua, "joint_erp");

                if(!body_setup.empty()) {

                    lua_getfield(engine_lua, -1, "body");
                    if(lua_istable(engine_lua, -1)) {
                        for(size_t i=0; i<body_setup.size(); i++) {
                            lua_rawgeti(engine_lua, -1, i+1);
                            if(lua_istable(engine_lua, -1)) {
                                body_setup[i].mass = lua_GetScalarField(engine_lua, "mass");
                                body_setup[i].restitution = lua_GetScalarField(engine_lua, "restitution");
                                body_setup[i].friction = lua_GetScalarField(engine_lua, "friction");

                                lua_getfield(engine_lua, -1, "damping");
                                if(lua_istable(engine_lua, -1)) {
                                    body_setup[i].damping[0] = lua_GetScalarField(engine_lua, 1);
                                    body_setup[i].damping[1] = lua_GetScalarField(engine_lua, 2);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);
                            }
                            else { result = false; }
                            lua_pop(engine_lua, 1);
                        }
                    }
                    else { result = false; }
                    lua_pop(engine_lua, 1);
                }
                else { result = false; }

                if(!joint_setup.empty()) {

                    lua_getfield(engine_lua, -1, "joint");
                    if(lua_istable(engine_lua, -1)) {
                        for(size_t i=0; i<joint_setup.size(); i++) {
                            lua_rawgeti(engine_lua, -1, i+1);
                            if(lua_istable(engine_lua, -1)) {
                                joint_setup[i].body_index = (uint16_t)lua_GetScalarField(engine_lua, "body_index");
                                joint_setup[i].joint_type = static_cast<RDJointSetup::Type>(lua_GetScalarField(engine_lua, "joint_type"));

                                lua_getfield(engine_lua, -1, "body1_offset");
                                if(lua_istable(engine_lua, -1)) {
                                    joint_setup[i].body1_offset.m_floats[0] = lua_GetScalarField(engine_lua, 1);
                                    joint_setup[i].body1_offset.m_floats[1] = lua_GetScalarField(engine_lua, 2);
                                    joint_setup[i].body1_offset.m_floats[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "body2_offset");
                                if(lua_istable(engine_lua, -1)) {
                                    joint_setup[i].body2_offset.m_floats[0] = lua_GetScalarField(engine_lua, 1);
                                    joint_setup[i].body2_offset.m_floats[1] = lua_GetScalarField(engine_lua, 2);
                                    joint_setup[i].body2_offset.m_floats[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "body1_angle");
                                if(lua_istable(engine_lua, -1)) {
                                    joint_setup[i].body1_angle[0] = lua_GetScalarField(engine_lua, 1);
                                    joint_setup[i].body1_angle[1] = lua_GetScalarField(engine_lua, 2);
                                    joint_setup[i].body1_angle[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "body2_angle");
                                if(lua_istable(engine_lua, -1)) {
                                    joint_setup[i].body2_angle[0] = lua_GetScalarField(engine_lua, 1);
                                    joint_setup[i].body2_angle[1] = lua_GetScalarField(engine_lua, 2);
                                    joint_setup[i].body2_angle[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);

                                lua_getfield(engine_lua, -1, "joint_limit");
                                if(lua_istable(engine_lua, -1)) {
                                    joint_setup[i].joint_limit[0] = lua_GetScalarField(engine_lua, 1);
                                    joint_setup[i].joint_limit[1] = lua_GetScalarField(engine_lua, 2);
                                    joint_setup[i].joint_limit[2] = lua_GetScalarField(engine_lua, 3);
                                }
                                else { result = false; }
                                lua_pop(engine_lua, 1);
                            }
                            else { result = false; }
                            lua_pop(engine_lua, 1);
                        }
                    }
                    else { result = false; }
                    lua_pop(engine_lua, 1);
                }
                else { result = false; }
            }
            else { result = false; }
        }
        else { result = false; }
    }
    else { result = false; }

    lua_settop(engine_lua, top);

    if(result == false)
        clearSetup();  // PARANOID: Clean up the mess, if something went wrong.
    return result;
}


void RDSetup::clearSetup()
{
    body_setup.clear();
    joint_setup.clear();
    hit_func.clear();
}
