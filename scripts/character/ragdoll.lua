-- OPENTOMB RAGDOLL CONFIGURATION SCRIPT
-- by Lwmte, May 2015

--------------------------------------------------------------------------------
-- This script contains ragdoll properties for all TR3-5 games.
--------------------------------------------------------------------------------
print("ragdoll->loaded !");

M_PI = 3.141592653;   -- Needed for alignment


-- Point constraint is usually used for multi-mesh dynamic entities, like
-- multiple boulders / barrels, etc. Hinge and cone constraints could be used
-- for actual character ragdolls. 

RD_CONSTRAINT_POINT = 0;
RD_CONSTRAINT_HINGE = 1;
RD_CONSTRAINT_CONE  = 2;

RD_TYPE_LARA        = 0;
RD_TYPE_T_REX       = 1;

RD_PARTS_LARA_PELVIS            = 0;          -- it is a root body
RD_PARTS_LARA_LEFT_UPPER_LEG    = 1;
RD_PARTS_LARA_LEFT_LOWER_LEG    = 2;
RD_PARTS_LARA_LEFT_FOOT         = 3;
RD_PARTS_LARA_RIGHT_UPPER_LEG   = 4;
RD_PARTS_LARA_RIGHT_LOWER_LEG   = 5;
RD_PARTS_LARA_RIGHT_FOOT        = 6;
RD_PARTS_LARA_SPINE             = 7;
RD_PARTS_LARA_RIGHT_UPPER_ARM   = 8;
RD_PARTS_LARA_RIGHT_LOWER_ARM   = 9;
RD_PARTS_LARA_RIGHT_PALM        = 10;
RD_PARTS_LARA_LEFT_UPPER_ARM    = 11;
RD_PARTS_LARA_LEFT_LOWER_ARM    = 12;
RD_PARTS_LARA_LEFT_PALM         = 13;
RD_PARTS_LARA_HEAD              = 14;

ragdoll                 = {};  -- Actual ragdoll array.
ragdoll_hit_callback    = {};  -- Not directly related - ragdoll body hit callbacks.

ragdoll[RD_TYPE_LARA] = {

  hit_callback = "lara",    -- Passed to character structure on ragdoll creation.
  body_count   = 15,        -- Bodies above body count won't be modified.
  joint_count  = 14,        -- Joints above joint count won't be added and won't be processed.
  
  joint_cfm    = 0.7,
  joint_erp    = 0.1,
  
  -- Body properties array.
  
  body = {
            {mass = 15.0, restitution = 0.8, friction =  8.0, damping = {0.2, 0.2}},    -- 00 = Pelvis
            {mass = 20.0, restitution = 0.0, friction =  4.0, damping = {0.2, 0.2}},    -- 01 = Left upper leg
            {mass = 15.0, restitution = 0.0, friction =  2.0, damping = {0.6, 0.6}},    -- 02 = Left lower leg
            {mass = 10.0, restitution = 0.3, friction =  5.0, damping = {0.8, 0.8}},    -- 03 = Left foot
            {mass = 20.0, restitution = 0.0, friction =  8.0, damping = {0.2, 0.2}},    -- 04 = Right upper leg
            {mass = 15.0, restitution = 0.0, friction =  4.0, damping = {0.6, 0.6}},    -- 05 = Right lower leg
            {mass = 10.0, restitution = 0.3, friction =  2.0, damping = {0.8, 0.8}},    -- 06 = Right foot
            {mass = 25.0, restitution = 0.5, friction = 10.0, damping = {0.9, 0.9}},    -- 07 = Spine
            {mass = 20.0, restitution = 0.0, friction =  6.5, damping = {0.3, 0.3}},    -- 11 = Right upper arm
            {mass = 15.0, restitution = 0.0, friction =  4.0, damping = {0.6, 0.6}},    -- 12 = Right lower arm
            {mass = 10.0, restitution = 0.3, friction =  1.0, damping = {0.8, 0.8}},    -- 13 = Right palm
            {mass = 20.0, restitution = 0.0, friction =  6.5, damping = {0.2, 0.5}},    -- 08 = Left upper arm
            {mass = 15.0, restitution = 0.0, friction =  4.0, damping = {0.6, 0.6}},    -- 09 = Left lower arm
            {mass = 10.0, restitution = 0.3, friction =  1.0, damping = {0.8, 0.8}},    -- 10 = Left palm
            {mass = 20.0, restitution = 0.0, friction =  8.0, damping = {0.8, 0.8}}     -- 14 = Head
         },
  
  -- Actual joint array.

  joint = {
  
              -- PELVIS-SPINE
  
      [1] = { body_index = RD_PARTS_LARA_SPINE,
          
              body1_offset = {0.0, 0.0, 40.0},
              body2_offset = {0.0, 0.0, -10.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.25, M_PI*0.5} },
              
              -- SPINE-HEAD
              
      [2] = { body_index = RD_PARTS_LARA_HEAD,
          
              body1_offset = {0.0, 0.0, 172.0},
              body2_offset = {0.0, 0.0, -32.0},
          
              body1_angle  = {0.0, 0.0, M_PI*0.5},
              body2_angle  = {0.0, 0.0, M_PI*0.5},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.25, M_PI*0.25, M_PI*0.25} },
              
              -- LEFT HIP
              
      [3] = { body_index = RD_PARTS_LARA_LEFT_UPPER_LEG,
          
              body1_offset = {-40.0, 0.0, -32.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, -M_PI*1.25},
              body2_angle  = {0.0, 0.0, -M_PI*1.25},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.5, M_PI*0.25, 0} },
              
              -- LEFT KNEE
              
      [4] = { body_index = RD_PARTS_LARA_LEFT_LOWER_LEG,
          
              body1_offset = {8.0, -8.0, -192.0},
              body2_offset = {0.0, -8.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.5} },
              
              -- LEFT ANKLE
              
      [5] = { body_index = RD_PARTS_LARA_LEFT_FOOT,
          
              body1_offset = {0.0, -8.0, -192.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.25} },
              
              -- RIGHT HIP
              
      [6] = { body_index = RD_PARTS_LARA_RIGHT_UPPER_LEG,
          
              body1_offset = {40.0, 0.0, -32.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, 0.0, M_PI*0.25},
              body2_angle  = {0.0, 0.0, M_PI*0.25},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.5, M_PI*0.25, 0.0} },
              
              -- RIGHT KNEE
              
      [7] = { body_index = RD_PARTS_LARA_RIGHT_LOWER_LEG,
          
              body1_offset = {-8.0, -8.0, -192.0},
              body2_offset = {0.0, -8.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.5} },
              
              -- RIGHT ANKLE
              
      [8] = { body_index = RD_PARTS_LARA_RIGHT_FOOT,
          
              body1_offset = {0.0, -8.0, -192.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {0.0, M_PI*0.25} },
              
              -- LEFT SHOULDER
              
      [9] = { body_index = RD_PARTS_LARA_LEFT_UPPER_ARM,
          
              body1_offset = {-50.0, -16.0, 138.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {-M_PI*1.5, M_PI*0.5, 0.0},
              body2_angle  = {-M_PI*1.5, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.5, M_PI*0.25, M_PI*0.15} },
              
              -- LEFT ELBOW
              
     [10] = { body_index = RD_PARTS_LARA_LEFT_LOWER_ARM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {8.0, 0.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.25, 0.0} },
              
              -- LEFT WRIST
              
     [11] = { body_index = RD_PARTS_LARA_LEFT_PALM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, M_PI*0.5},
              body2_angle  = {0.0, M_PI*0.5, M_PI*0.5},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.25, M_PI*0.25} },
              
              -- RIGHT SHOULDER
              
     [12] = { body_index = RD_PARTS_LARA_RIGHT_UPPER_ARM,
          
              body1_offset = {50.0, -16.0, 138.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {-M_PI*1.5, M_PI*0.5, 0.0},
              body2_angle  = {-M_PI*1.5, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_CONE,
              joint_limit  = {M_PI*0.5, M_PI*0.25, M_PI*0.15} },
              
              -- RIGHT ELBOW
              
     [13] = { body_index = RD_PARTS_LARA_RIGHT_LOWER_ARM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {-8.0, 0.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, 0.0},
              body2_angle  = {0.0, M_PI*0.5, 0.0},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.25, 0.0} },
              
              -- RIGHT WRIST
              
     [14] = { body_index = RD_PARTS_LARA_RIGHT_PALM,
          
              body1_offset = {0.0, 0.0, -100.0},
              body2_offset = {0.0, 0.0, 0.0},
          
              body1_angle  = {0.0, M_PI*0.5, M_PI*0.5},
              body2_angle  = {0.0, M_PI*0.5, M_PI*0.5},
          
              joint_type   = RD_CONSTRAINT_HINGE,
              joint_limit  = {-M_PI*0.25, M_PI*0.25} },
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