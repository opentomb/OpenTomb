#include "ragdoll.h"

#include "LuaState.h"

#include "script/script.h"

namespace world
{

bool RDSetup::getSetup(int ragdoll_index)
{
    lua::Value rds = engine_lua["getRagdollSetup"](ragdoll_index);
    if(!rds.is<lua::Table>())
        return false;

    hit_func = static_cast<const char*>(rds["hit_callback"]);

    size_t tmp;

    tmp = rds["joint_count"];
    joint_setup.resize(tmp);

    tmp = rds["body_count"];
    body_setup.resize(tmp);

    joint_cfm = rds["joint_cfm"];
    joint_erp = rds["joint_erp"];

    for(size_t i = 0; i < body_setup.size(); i++)
    {
        body_setup[i].mass = rds["body"][i + 1]["mass"];
        body_setup[i].restitution = rds["body"][i + 1]["restitution"];
        body_setup[i].friction = rds["body"][i + 1]["friction"];
        if(rds["body"][i + 1]["damping"].is<lua::Table>())
        {
            body_setup[i].damping[0] = rds["body"][i + 1]["damping"][1];
            body_setup[i].damping[1] = rds["body"][i + 1]["damping"][2];
        }
    }

    for(size_t i = 0; i < joint_setup.size(); i++)
    {
        joint_setup[i].body_index = rds["joint"][i + 1]["body_index"];
        joint_setup[i].joint_type = static_cast<RDJointSetup::Type>(static_cast<int>(rds["joint"][i + 1]["joint_type"]));
        if(rds["joint"][i + 1]["body1_offset"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body1_offset[j] = rds["joint"][i + 1]["body1_offset"][j + 1];
        }
        if(rds["joint"][i + 1]["body2_offset"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body2_offset[j] = rds["joint"][i + 1]["body2_offset"][j + 1];
        }
        if(rds["joint"][i + 1]["body1_angle"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body1_angle[j] = rds["joint"][i + 1]["body1_angle"][j + 1];
        }
        if(rds["joint"][i + 1]["body2_angle"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body2_angle[j] = rds["joint"][i + 1]["body2_angle"][j + 1];
        }
        if(rds["joint"][i + 1]["joint_limit"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].joint_limit[j] = rds["joint"][i + 1]["joint_limit"][j + 1];
        }
    }

    return true;
}

void RDSetup::clearSetup()
{
    body_setup.clear();
    joint_setup.clear();
    hit_func.clear();
}

} // namespace world
