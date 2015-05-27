-- OPENTOMB LARA HAIR CONFIGURATION SCRIPT
-- by Lwmte, May 2015

--------------------------------------------------------------------------------
-- This script contains hair properties for all TR1-5 games.
--------------------------------------------------------------------------------

HAIR_TR1       = 0
HAIR_TR2       = 1
HAIR_TR3       = 2
HAIR_TR4_KID_1 = 3
HAIR_TR4_KID_2 = 4
HAIR_TR4_OLD   = 5
HAIR_TR5_KID_1 = 6
HAIR_TR5_KID_2 = 7
HAIR_TR5_OLD   = 8

M_PI = 3.141592653;   -- Needed for hair alignment


old_hair_props = {
    
    root_weight     = 1.0,
    tail_weight     = 10.0,
    
    hair_damping    = {0.15, 0.30},
    hair_inertia    = 120.0,
    hair_friction   = 20.0,
    hair_bouncing   = 0.0,
    
    joints_per_body = 3,
    joint_radius    = 0.4,
    joint_cfm       = 0.1,
    joint_erp       = 1.0,
    
    joint_overlap   = 0.8,
}                 

new_hair_old_props = {
    
    root_weight     = 1.0,
    tail_weight     = 10.0,
    
    hair_damping    = {0.15, 0.80},
    hair_inertia    = 50.0,
    hair_friction   = 10.0,
    hair_bouncing   = 0.0,
    
    joints_per_body = 3,
    joint_radius    = 0.4,
    joint_cfm       = 0.1,
    joint_erp       = 1.0,
    
    joint_overlap   = 0.9
}

new_hair_kid_props = {
    
    root_weight     = 1.0,
    tail_weight     = 8.0,
    
    hair_damping    = {0.15, 0.80},
    hair_inertia    = 80.0,
    hair_friction   = 10.0,
    hair_bouncing   = 0.0,
    
    joints_per_body = 3,
    joint_radius    = 0.5,
    joint_cfm       = 0.1,
    joint_erp       = 1.0,
    
    joint_overlap   = 0.8
}

hair = {}

-- Hair in TR1 is non-existent by default, but it can be re-implemented by
-- importing TR2/3 hair into TR1 levels with item ID 189.

hair[HAIR_TR1]       = { props = old_hair_props,     model = 189, link_body = 14, v_count = 3, v_index = {43, 44, 45    }, offset = {  0.0, -48.0, -32.0}, root_angle = {-M_PI/2, 0.0, -M_PI} }
hair[HAIR_TR2]       = { props = old_hair_props,     model = 2  , link_body = 14, v_count = 4, v_index = {52, 55, 51, 48}, offset = {  0.0, -48.0,  16.0}, root_angle = {-M_PI/2, 0.0, -M_PI} }
hair[HAIR_TR4_KID_1] = { props = new_hair_kid_props, model = 30 , link_body = 14, v_count = 4, v_index = {70, 71, 68, 69}, offset = { 32.0, -40.0,  44.0}, root_angle = {-M_PI/4, 0.0, ((M_PI/4)*5)} }
hair[HAIR_TR4_KID_2] = { props = new_hair_kid_props, model = 30 , link_body = 14, v_count = 4, v_index = {76, 77, 79, 78}, offset = {-36.0, -40.0,  44.0}, root_angle = {-M_PI/4, M_PI/4, (M_PI/4)*3} }
hair[HAIR_TR4_OLD]   = { props = new_hair_old_props, model = 30 , link_body = 14, v_count = 4, v_index = {40, 38, 37, 39}, offset = {  0.0, -36.0,   8.0}, root_angle = {-M_PI/2, 0.0, -M_PI} }

-- TR3 hair is similar to TR2, TR5 hair is similar to TR4.

hair[HAIR_TR3]       = hair[HAIR_TR2]
hair[HAIR_TR5_KID_1] = hair[HAIR_TR4_KID1]
hair[HAIR_TR5_KID_2] = hair[HAIR_TR4_KID2]
hair[HAIR_TR5_OLD]   = hair[HAIR_TR4_OLD]


-- Get hair info from corresponding property table.

function getHairSetup(id)
    if((hair == nil) or (hair[id] == nil)) then
        return nil;
    else
        return hair[id];
    end;
end;