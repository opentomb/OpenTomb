-- OPENTOMB RAGDOLL CONFIGURATION SCRIPT
-- by Lwmte, May 2015

--------------------------------------------------------------------------------
-- This script contains ragdoll properties for all TR3-5 games.
--------------------------------------------------------------------------------


M_PI = 3.141592653;   -- Needed for alignment


-- Point constraint is usually used for multi-mesh dynamic entities, like
-- multiple boulders / barrels, etc. Hinge and cone constraints could be used
-- for actual character ragdolls. 

RD_CONSTRAINT_POINT = 0;
RD_CONSTRAINT_HINGE = 1;
RD_CONSTRAINT_CONE  = 2;

RD_TYPE_LARA        = 0;
RD_TYPE_TR2_BALLS   = 1;
RD_TYPE_TR2_BARRELS = 2;

RD_TYPE_TR3_TIGER   = 3;

RD_PARTS_LARA_PELVIS = 0;
RD_PARTS_LARA_RIGHT_UPPER_LEG = 1;
RD_PARTS_LARA_RIGHT_LOWER_LEG = 2;
RD_PARTS_LARA_RIGHT_FOOT = 3;
RD_PARTS_LARA_LEFT_UPPER_LEG = 4;
RD_PARTS_LARA_LEFT_LOWER_LEG = 5;
RD_PARTS_LARA_LEFT_FOOT = 6;
RD_PARTS_LARA_SPINE = 7;
RD_PARTS_LARA_LEFT_UPPER_ARM = 8;
RD_PARTS_LARA_LEFT_LOWER_ARM = 9;
RD_PARTS_LARA_LEFT_PALM = 10;
RD_PARTS_LARA_RIGHT_UPPER_ARM = 11;
RD_PARTS_LARA_RIGHT_LOWER_ARM = 12;
RD_PARTS_LARA_RIGHT_PALM = 13;
RD_PARTS_LARA_HEAD = 14;

ragdoll                 = {};  -- Actual ragdoll array.
ragdoll_hit_callback    = {};                       -- Not directly related - ragdoll body hit callbacks.

ragdoll[RD_TYPE_LARA] = {

  hit_callback = "lara",    -- Passed to character structure on ragdoll creation.
  body_count   = 15,        -- Bodies above body count won't be modified.
  joint_count  = 14,        -- Joints above joint count won't be added and won't be processed.
  
  -- Body properties array.
  
  body = {
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 00 = Pelvis
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 01 = Left upper leg
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 02 = Left lower leg
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 03 = Left foot
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 04 = Right upper leg
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 05 = Right lower leg
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 06 = Right foot
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 07 = Spine
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 08 = Left upper arm
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 09 = Left lower arm
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 10 = Left palm
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 11 = Right upper arm
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 12 = Right lower arm
            {weight = 1.0, restitution = 0.0, friction = 0.0, damping = {0.5, 0.5}},    -- 13 = Right palm
            {weight = 1.0, restitution = 0.2, friction = 0.0, damping = {0.5, 0.5}}     -- 14 = Head
         },
  
  -- Actual joint array.

  joint = {
  
              -- PELVIS-SPINE
  
            { body1_index = RD_PARTS_LARA_PELVIS,
              body2_index = RD_PARTS_LARA_SPINE,
          
              body1_offset = {0.0, 0.0, 40.0},
              body2_offset = {0.0, 0.0, -10.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.25, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- SPINE-HEAD
              
            { body1_index = RD_PARTS_LARA_SPINE,
              body2_index = RD_PARTS_LARA_HEAD,
          
              body1_offset = {0.0, 0.0, 172.0},
              body2_offset = {0.0, 0.0, -32.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.25, M_PI*0.25, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- LEFT HIP
              
            { body1_index = RD_PARTS_LARA_PELVIS,
              body2_index = RD_PARTS_LARA_LEFT_UPPER_LEG,
          
              body1_offset = {40.0, 0.0, -32.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.25, M_PI*0.25, 0},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- LEFT KNEE
              
            { body1_index = RD_PARTS_LARA_LEFT_UPPER_LEG,
              body2_index = RD_PARTS_LARA_LEFT_LOWER_LEG,
          
              body1_offset = {0.0, 5.0, -192.0},
              body2_offset = {0.0, -8.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- LEFT ANKLE
              
            { body1_index = RD_PARTS_LARA_LEFT_LOWER_LEG,
              body2_index = RD_PARTS_LARA_LEFT_FOOT,
          
              body1_offset = {0.0, 0.0, -192.0},
              body2_offset = {0.0, -12.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, M_PI},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.5, 0.0},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- RIGHT HIP
              
            { body1_index = RD_PARTS_LARA_PELVIS,
              body2_index = RD_PARTS_LARA_RIGHT_UPPER_LEG,
          
              body1_offset = {-40.0, 0.0, -32.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.25, M_PI*0.25, 0.0},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- RIGHT KNEE
              
            { body1_index = RD_PARTS_LARA_RIGHT_UPPER_LEG,
              body2_index = RD_PARTS_LARA_RIGHT_LOWER_LEG,
          
              body1_offset = {0.0, 5.0, -192.0},
              body2_offset = {0.0, -8.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- RIGHT ANKLE
              
            { body1_index = RD_PARTS_LARA_RIGHT_LOWER_LEG,
              body2_index = RD_PARTS_LARA_RIGHT_FOOT,
          
              body1_offset = {0.0, 0.0, -192.0},
              body2_offset = {0.0, -8.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, M_PI},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.5, 0.0},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- LEFT SHOULDER
              
            { body1_index = RD_PARTS_LARA_SPINE,
              body2_index = RD_PARTS_LARA_LEFT_UPPER_ARM,
          
              body1_offset = {50.0, -16.0, 138.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.5, M_PI*0.5, 0.0},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- LEFT ELBOW
              
            { body1_index = RD_PARTS_LARA_LEFT_UPPER_ARM,
              body2_index = RD_PARTS_LARA_LEFT_LOWER_ARM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.5, 0.0, 0.0, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- LEFT WRIST
              
            { body1_index = RD_PARTS_LARA_LEFT_LOWER_ARM,
              body2_index = RD_PARTS_LARA_LEFT_PALM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- RIGHT SHOULDER
              
            { body1_index = RD_PARTS_LARA_SPINE,
              body2_index = RD_PARTS_LARA_RIGHT_UPPER_ARM,
          
              body1_offset = {-50.0, -16.0, 138.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.5, M_PI*0.5, 0.0},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- RIGHT ELBOW
              
            { body1_index = RD_PARTS_LARA_RIGHT_UPPER_ARM,
              body2_index = RD_PARTS_LARA_RIGHT_LOWER_ARM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.5, 0.0, 0.0, M_PI*0.5},
    
              joint_cfm    = 0.3,
              joint_erp    = 0.5 },
              
              -- RIGHT WRIST
              
            { body1_index = RD_PARTS_LARA_RIGHT_LOWER_ARM,
              body2_index = RD_PARTS_LARA_RIGHT_PALM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, 0.0},
              body2_angle  = {0.0, 0.0, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.5},
    
              joint_cfm    = 0.1,
              joint_erp    = 0.8 }
          }
}


ragdoll_hit_callback["lara"] = function(id)
    if(math.random(100000) > 99500) then
        playSound(4, id)                    -- Replace 4 with something more appropriate!
    end
end;


-- Get ragdoll info from corresponding property table.

function getRagdollSetup(id)
    if((ragdoll == nil) or (ragdoll[id] == nil)) then
        return nil;
    else
        return ragdoll[id];
    end;
end;