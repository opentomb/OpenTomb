#include "ragdoll.h"

#include <cmath>
#include <assert.h>

#include "bullet/LinearMath/btScalar.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bullet/btBulletDynamicsCommon.h"

#include "vmath.h"
#include "character_controller.h"

#include "engine.h"
#include "entity.h"

#include "LuaState.h"
#include "luastate_extra.h"

bool RDSetup::getSetup(int ragdoll_index)
{
    bool result = true;

    lua::Value rds = engine_lua["getRagdollSetup"](ragdoll_index);
    rds["hit_callback"].get(hit_func);

    size_t tmp;

    rds["joint_count"].get(tmp);
    joint_setup.resize(tmp);

    rds["body_count"].get(tmp);
    body_setup.resize(tmp);

    rds["joint_cfm"].get(joint_cfm);
    rds["joint_erp"].get(joint_erp);

    for(size_t i=0; i<body_setup.size(); i++) {
        rds["body"][i+1]["mass"].get(body_setup[i].mass);
        rds["body"][i+1]["restitution"].get(body_setup[i].restitution);
        rds["body"][i+1]["friction"].get(body_setup[i].friction);
        if(rds["body"][i+1]["damping"].is<lua::Table>()) {
            rds["body"][i+1]["damping"][1].get(body_setup[i].damping[0]);
            rds["body"][i+1]["damping"][2].get(body_setup[i].damping[1]);
        }
        rds["body"][i+1]["damping"].get(body_setup[i].friction);
    }

    for(size_t i=0; i<joint_setup.size(); i++) {
        rds["joint"][i+1]["body_index"].get(joint_setup[i].body_index);
        int tmp;
        rds["joint"][i+1]["joint_type"].get(tmp);
        joint_setup[i].joint_type = static_cast<RDJointSetup::Type>(tmp);
        if(rds["joint"][i+1]["body1_offset"].is<lua::Table>()) {
            rds["joint"][i+1]["body1_offset"][1].get(joint_setup[i].body1_offset[0]);
            rds["joint"][i+1]["body1_offset"][2].get(joint_setup[i].body1_offset[1]);
            rds["joint"][i+1]["body1_offset"][3].get(joint_setup[i].body1_offset[2]);
        }
        if(rds["joint"][i+1]["body2_offset"].is<lua::Table>()) {
            rds["joint"][i+1]["body2_offset"][1].get(joint_setup[i].body2_offset[0]);
            rds["joint"][i+1]["body2_offset"][2].get(joint_setup[i].body2_offset[1]);
            rds["joint"][i+1]["body2_offset"][3].get(joint_setup[i].body2_offset[2]);
        }
        if(rds["joint"][i+1]["body1_angle"].is<lua::Table>()) {
            rds["joint"][i+1]["body1_angle"][1].get(joint_setup[i].body1_angle[0]);
            rds["joint"][i+1]["body1_angle"][2].get(joint_setup[i].body1_angle[1]);
            rds["joint"][i+1]["body1_angle"][3].get(joint_setup[i].body1_angle[2]);
        }
        if(rds["joint"][i+1]["body2_angle"].is<lua::Table>()) {
            rds["joint"][i+1]["body2_angle"][1].get(joint_setup[i].body2_angle[0]);
            rds["joint"][i+1]["body2_angle"][2].get(joint_setup[i].body2_angle[1]);
            rds["joint"][i+1]["body2_angle"][3].get(joint_setup[i].body2_angle[2]);
        }
        if(rds["joint"][i+1]["joint_limit"].is<lua::Table>()) {
            rds["joint"][i+1]["joint_limit"][1].get(joint_setup[i].joint_limit[0]);
            rds["joint"][i+1]["joint_limit"][2].get(joint_setup[i].joint_limit[1]);
            rds["joint"][i+1]["joint_limit"][3].get(joint_setup[i].joint_limit[2]);
        }
    }

    return result;
}


void RDSetup::clearSetup()
{
    body_setup.clear();
    joint_setup.clear();
    hit_func.clear();
}
