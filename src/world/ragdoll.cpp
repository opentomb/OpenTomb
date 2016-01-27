#include "ragdoll.h"

#include "engine/engine.h"

#include "LuaState.h"

namespace world
{
bool RagdollSetup::getSetup(engine::Engine& engine, int ragdoll_index)
{
    lua::Value rds = engine.engine_lua["getRagdollSetup"](ragdoll_index);
    if(!rds.is<lua::Table>())
        return false;

    hit_func = rds["hit_callback"].toCStr();

    size_t tmp;

    tmp = rds["joint_count"].to<size_t>();
    joint_setup.resize(tmp);

    tmp = rds["body_count"].to<size_t>();
    body_setup.resize(tmp);

    joint_cfm = rds["joint_cfm"].toFloat();
    joint_erp = rds["joint_erp"].toFloat();

    for(size_t i = 0; i < body_setup.size(); i++)
    {
        body_setup[i].mass = rds["body"][i + 1]["mass"].toFloat();
        body_setup[i].restitution = rds["body"][i + 1]["restitution"].toFloat();
        body_setup[i].friction = rds["body"][i + 1]["friction"].toFloat();
        if(rds["body"][i + 1]["damping"].is<lua::Table>())
        {
            body_setup[i].damping[0] = rds["body"][i + 1]["damping"][1].toFloat();
            body_setup[i].damping[1] = rds["body"][i + 1]["damping"][2].toFloat();
        }
    }

    for(size_t i = 0; i < joint_setup.size(); i++)
    {
        joint_setup[i].body_index = rds["joint"][i + 1]["body_index"].to<animation::BoneId>();
        joint_setup[i].joint_type = static_cast<RagdollJointSetup::Type>(rds["joint"][i + 1]["joint_type"].toInt());
        if(rds["joint"][i + 1]["body1_offset"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body1_offset[j] = rds["joint"][i + 1]["body1_offset"][j + 1].toFloat();
        }
        if(rds["joint"][i + 1]["body2_offset"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body2_offset[j] = rds["joint"][i + 1]["body2_offset"][j + 1].toFloat();
        }
        if(rds["joint"][i + 1]["body1_angle"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body1_angle[j] = rds["joint"][i + 1]["body1_angle"][j + 1].toFloat();
        }
        if(rds["joint"][i + 1]["body2_angle"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].body2_angle[j] = rds["joint"][i + 1]["body2_angle"][j + 1].toFloat();
        }
        if(rds["joint"][i + 1]["joint_limit"].is<lua::Table>())
        {
            for(int j = 0; j < 3; ++j)
                joint_setup[i].joint_limit[j] = rds["joint"][i + 1]["joint_limit"][j + 1].toFloat();
        }
    }

    return true;
}

void RagdollSetup::clearSetup()
{
    body_setup.clear();
    joint_setup.clear();
    hit_func.clear();
}
} // namespace world