
#include "world.h"

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

#include "character_controller.h"
#include "anim_state_control.h"
#include "engine.h"
#include "entity.h"
#include "mesh.h"
#include "vmath.h"


#define CHARACTER_BOX_HALF_SIZE (128.0)
#define CHARACTER_BASE_RADIUS   (128.0)
#define CHARACTER_BASE_HEIGHT   (512.0)

void Character_Create(struct entity_s *ent, btScalar r, btScalar h)
{
    character_p ret;
    btTransform tr;

    if(ent == NULL || ent->character != NULL)
    {
        return;
    }

    ret = (character_p)malloc(sizeof(character_t));

    ret->ent = ent;
    ent->character = ret;

    ret->cmd.action = 0x00;
    ret->cmd.vertical_collide = 0x00;
    ret->cmd.horizontal_collide = 0x00;
    ret->cmd.crouch = 0x00;
    ret->cmd.flags = 0x00;
    ret->cmd.jump = 0x00;
    ret->cmd.kill = 0x00;
    ret->cmd.roll = 0x00;
    ret->cmd.shift = 0x00;
    ret->cmd.slide = 0x00;
    vec3_set_zero(ret->cmd.move);
    vec3_set_zero(ret->cmd.rot);
            
    ret->speed_mult = DEFAULT_CHARACTER_SPEED_MULT;
    ret->max_move_iterations = DEFAULT_MAX_MOVE_ITERATIONS;
    ret->min_step_up_height = DEFAULT_MIN_STEP_UP_HEIGHT;
    ret->max_step_up_height = DEFAULT_MAX_STEP_UP_HEIGHT;
    ret->fall_down_height = DEFAULT_FALL_DAWN_HEIGHT;
    ret->critical_slant_z_component = DEFAULT_CRITICAL_SLANT_Z_COMPONENT;
    ret->critical_wall_component = DEFAULT_CRITICAL_WALL_COMPONENT;
    ret->climb_r = DEFAULT_CHARACTER_CLIMB_R;
    
    vec3_set_zero(ret->speed.m_floats);

    ret->Radius = r;
    ret->Height = h;

    ret->shapeBox = new btBoxShape(btVector3(CHARACTER_BOX_HALF_SIZE, CHARACTER_BOX_HALF_SIZE, CHARACTER_BOX_HALF_SIZE));
    ret->shapeZ = new btCapsuleShapeZ(CHARACTER_BASE_RADIUS, CHARACTER_BASE_HEIGHT - 2.0 * CHARACTER_BASE_RADIUS);
    ret->shapeY = new btCapsuleShape(CHARACTER_BASE_RADIUS, CHARACTER_BASE_HEIGHT - 2.0 * CHARACTER_BASE_RADIUS);
    ret->sphere = new btSphereShape(CHARACTER_BASE_RADIUS);
    ret->climb_sensor = new btSphereShape(ent->character->climb_r);

    ret->manifoldArray = new btManifoldArray();

    tr.setFromOpenGLMatrix(ent->transform);
    ret->ghostObject = new btPairCachingGhostObject();
    ret->ghostObject->setWorldTransform(tr);
    ret->ghostObject->setCollisionFlags(ret->ghostObject->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
    ret->ghostObject->setUserPointer(ent->self);
    ent->character->ghostObject->setCollisionShape(ent->character->shapeZ);
    bt_engine_dynamicsWorld->addCollisionObject(ret->ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter | btBroadphaseProxy::DefaultFilter);

    ret->ray_cb = new bt_engine_ClosestRayResultCallback(ent->self);
    ret->ray_cb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    ret->convex_cb = new bt_engine_ClosestConvexResultCallback(ent->self);
    ret->convex_cb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;

    ret->height_info.cb = ret->ray_cb;
    ret->height_info.ccb = ret->convex_cb;
    ret->height_info.ceiling_hit = 0x00;
    ret->height_info.floor_hit = 0x00;
    ret->height_info.edge_hit = 0x00;
    ret->height_info.water = 0x00;
}


void Character_Clean(struct entity_s *ent)
{
    character_p actor = ent->character;

    if(actor == NULL)
    {
        return;
    }

    actor->ent = NULL;

    if(actor->ghostObject)
    {
        actor->ghostObject->setUserPointer(NULL);
        bt_engine_dynamicsWorld->removeCollisionObject(actor->ghostObject);
        delete actor->ghostObject;
        actor->ghostObject = NULL;
    }

    if(actor->shapeZ)
    {
        delete actor->shapeZ;
        actor->shapeZ = NULL;
    }

    if(actor->shapeY)
    {
        delete actor->shapeY;
        actor->shapeY = NULL;
    }

    if(actor->climb_sensor)
    {
        delete actor->climb_sensor;
        actor->climb_sensor = NULL;
    }

    if(actor->sphere)
    {
        delete actor->sphere;
        actor->sphere = NULL;
    }

    if(actor->ray_cb)
    {
        delete actor->ray_cb;
        actor->ray_cb = NULL;
    }
    if(actor->convex_cb)
    {
        delete actor->convex_cb;
        actor->convex_cb = NULL;
    }

    if(actor->manifoldArray)
    {
        delete actor->manifoldArray;
        actor->manifoldArray = NULL;
    }
    actor->height_info.cb = NULL;
    actor->height_info.ccb = NULL;
    actor->height_info.ceiling_hit = 0x00;
    actor->height_info.floor_hit = 0x00;
    actor->height_info.edge_hit = 0x00;
    actor->height_info.water = 0x00;
    
    free(ent->character);
    ent->character = NULL;
}

void Character_UpdateCollisionObject(struct entity_s *ent, btScalar z_factor)
{
    btVector3 tv;
    
    tv.m_floats[0] = 0.5 * (ent->bf.bb_max[0] - ent->bf.bb_min[0]) / CHARACTER_BOX_HALF_SIZE;
    tv.m_floats[1] = 0.5 * (ent->bf.bb_max[1] - ent->bf.bb_min[1]) / CHARACTER_BOX_HALF_SIZE;
    tv.m_floats[2] = 0.5 * (ent->bf.bb_max[2] - ent->bf.bb_min[2] - z_factor) / CHARACTER_BOX_HALF_SIZE;
    ent->character->shapeBox->setLocalScaling(tv); 
    
    if(ent->bf.bb_max[2] - ent->bf.bb_min[2] >= ent->bf.bb_max[1] - ent->bf.bb_min[1])
    {
        //Z_CAPSULE
        tv.m_floats[0] = ent->character->Radius / CHARACTER_BASE_RADIUS;
        tv.m_floats[1] = ent->character->Radius / CHARACTER_BASE_RADIUS;
        tv.m_floats[2] = (ent->bf.bb_max[2] - ent->bf.bb_min[2] - z_factor) / CHARACTER_BASE_HEIGHT;
        ent->character->shapeZ->setLocalScaling(tv);
        ent->character->ghostObject->setCollisionShape(ent->character->shapeZ);
        
        tv.m_floats[0] = 0.0;
        tv.m_floats[1] = 0.0;
        tv.m_floats[2] = 0.5 * (ent->bf.bb_max[2] + ent->bf.bb_min[2] - z_factor) + z_factor;
        ent->collision_offset = tv;
    }
    else
    {
        //Y_CAPSULE
        tv.m_floats[0] = ent->character->Radius / CHARACTER_BASE_RADIUS;
        tv.m_floats[1] = (ent->bf.bb_max[1] - ent->bf.bb_min[1]) / CHARACTER_BASE_HEIGHT;
        tv.m_floats[2] = ent->character->Radius / CHARACTER_BASE_RADIUS;
        ent->character->shapeY->setLocalScaling(tv);
        ent->character->ghostObject->setCollisionShape(ent->character->shapeY);
        
        tv.m_floats[0] = 0.0;
        tv.m_floats[1] = 0.5 * (ent->bf.bb_max[1] + ent->bf.bb_min[1]);
        tv.m_floats[2] = 0.5 * (ent->bf.bb_max[2] + ent->bf.bb_min[2]);
        //ent->character->collision_offset = tv;
        Mat4_vec3_rot_macro(ent->collision_offset.m_floats, ent->transform, tv.m_floats);
    }
}

void Character_UpdateCurrentRoom(struct entity_s *ent)
{
    btScalar pos[3];
    vec3_add(pos, ent->transform + 12, ent->collision_offset.m_floats);
    ent->self->room = Room_FindPosCogerrence(&engine_world, pos, ent->self->room);
}

/**
 * Start position are taken from ent->transform
 */
void Character_GetHeightInfo(btScalar pos[3], struct height_info_s *fc)
{
    btVector3 from, to;
    bt_engine_ClosestRayResultCallback *cb = fc->cb;
    room_p r = (cb->m_cont)?(cb->m_cont->room):(NULL);
    room_sector_p rs;
    
    fc->edge_hit = 0x00;
    fc->floor_hit = 0x00;
    fc->ceiling_hit = 0x00;
    fc->water = 0x00;
    fc->water_level = 32512.0;
    
    r = Room_FindPosCogerrence(&engine_world, pos, r);
    if(r)
    {
        rs = Room_GetSector(r, pos);                                            // if r != NULL then rs can not been NULL!!!
        if(r->flags & 0x01)                                                     // in water - go up
        {
            while(rs->sector_above)
            {
                rs = rs->sector_above;
                if((rs->owner_room->flags & 0x01) == 0x00)                      // find air
                {
                    fc->water_level = (btScalar)rs->floor;
                    fc->water = 0x01;
                    break;
                }
            }
        }
        else                                                                    // in air - go down
        {
            while(rs->sector_below)
            {
                rs = rs->sector_below;
                if((rs->owner_room->flags & 0x01) != 0x00)                      // find water
                {
                    fc->water_level = (btScalar)rs->ceiling;
                    fc->water = 0x01;
                    break;
                }
            }
        }
    }
    
    /*
     * GET HEIGHTS
     */
    vec3_copy(from.m_floats, pos);
    to = from;
    to.m_floats[2] -= 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
    cb->m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;// kF_KeepUnflippedNormal
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    fc->floor_hit = (int)cb->hasHit();
    if(fc->floor_hit)
    {
        fc->floor_normale = cb->m_hitNormalWorld;
        fc->floor_point.setInterpolate3(from, to, cb->m_closestHitFraction);
        fc->floor_obj = (engine_container_p)cb->m_collisionObject->getUserPointer();
    }
    
    if(fc->floor_hit)
    {
        from = fc->floor_point;
        from.m_floats[2] += 64.0;
    }
    to = from;
    to.m_floats[2] += 4096.0;
    cb->m_closestHitFraction = 1.0;
    cb->m_collisionObject = NULL;
    cb->m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;// kF_KeepUnflippedNormal
    bt_engine_dynamicsWorld->rayTest(from, to, *cb);
    fc->ceiling_hit = (int)cb->hasHit();
    if(fc->ceiling_hit)
    {
        fc->ceiling_normale = cb->m_hitNormalWorld;
        fc->ceiling_point.setInterpolate3(from, to, cb->m_closestHitFraction);
        fc->ceiling_obj = (engine_container_p)cb->m_collisionObject->getUserPointer();
    }
    
    if(!fc->floor_hit && fc->ceiling_hit)
    {
        from = fc->ceiling_point;
        from.m_floats[2] -= 64.0;
        to = from;
        to.m_floats[2] -= 4096.0;
        cb->m_closestHitFraction = 1.0;
        cb->m_collisionObject = NULL;
        cb->m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;// kF_KeepUnflippedNormal
        bt_engine_dynamicsWorld->rayTest(from, to, *cb);
        fc->floor_hit = (int)cb->hasHit();
        if(fc->floor_hit)
        {
            fc->floor_normale = cb->m_hitNormalWorld;
            fc->floor_point.setInterpolate3(from, to, cb->m_closestHitFraction);
            fc->floor_obj = (engine_container_p)cb->m_collisionObject->getUserPointer();
        }
    }    
}

/**
 * @function calculates next floor info + fantom filter + returns step info
 */
int Character_CheckNextStep(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc)
{
    
}

/**
 * @FIXME: MAGICK CONST!
 * @param ent - entity
 * @param offset - offset, when we check height
 * @param nfc - height info (floor / ceiling)
 */
int Character_CheckClimbability(struct entity_s *ent, btScalar offset[3], struct height_info_s *nfc, btScalar test_height)
{
    btVector3 from, to, tmp;
    btScalar d, *pos = ent->transform + 12;
    btScalar n0[4], n1[4], n2[4], *v;                                           // planes equations
    btTransform t1, t2;
    //extern GLfloat cast_ray[6];
    /*
     * init callbacks functions
     */
    nfc->cb = ent->character->ray_cb;
    nfc->ccb = ent->character->convex_cb;
    nfc->edge_hit = 0x00; 
    nfc->floor_hit = 0x00; 
    nfc->ceiling_hit = 0x00; 
    vec3_add(tmp.m_floats, pos, offset);                                        // tmp = native offset point
    /*
     * check max height
     */   
    vec3_add(from.m_floats, pos, ent->collision_offset.m_floats);               // from - centre of model
    to = from;
    to.m_floats[2] += ent->character->Height;
    nfc->cb->m_closestHitFraction = 1.0;
    nfc->cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *nfc->cb);
    if(nfc->cb->hasHit())
    {
        nfc->ceiling_hit = 0x01;
        nfc->ceiling_normale = nfc->cb->m_hitNormalWorld;
        nfc->ceiling_point.setInterpolate3(from, to, nfc->cb->m_closestHitFraction);
        nfc->ceiling_obj = (engine_container_p)nfc->cb->m_collisionObject->getUserPointer();
        if(tmp.m_floats[2] > nfc->ceiling_point.m_floats[2] - 1.0)              ///@FIXME: magick
        {
            tmp.m_floats[2] = nfc->ceiling_point.m_floats[2] - 1.0;
        }
    }
    // ok, we set ceiling height limit, now we must to prevent direct slant surface skip
    /*
    * Let us calculate EDGE
    */
    from = tmp;
    to = from;
    to.m_floats[2] -= ent->character->Height;
    nfc->cb->m_closestHitFraction = 1.0;
    nfc->cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *nfc->cb);
    // add three ray test here, but more correct is using no hole collision model!
    if(!nfc->cb->hasHit())
    {
        from.m_floats[0] = pos[0];
        from.m_floats[1] = pos[1];
        from.m_floats[2] = tmp.m_floats[2];
        to.m_floats[0] = tmp.m_floats[0] + ent->transform[4 + 0] * ent->character->climb_r * 2;
        to.m_floats[1] = tmp.m_floats[1] + ent->transform[4 + 1] * ent->character->climb_r * 2;
        to.m_floats[2] = from.m_floats[2];
        nfc->cb->m_closestHitFraction = 1.0;
        nfc->cb->m_collisionObject = NULL;
        bt_engine_dynamicsWorld->rayTest(from, to, *nfc->cb);
        if(!nfc->cb->hasHit() || nfc->cb->m_hitNormalWorld.m_floats[2] < 0.1)
        {
            return 0;
        }
    }
    
    nfc->floor_hit = 0x01;
    nfc->floor_point.setInterpolate3(from, to, nfc->cb->m_closestHitFraction);
    nfc->floor_normale = nfc->cb->m_hitNormalWorld;
    nfc->floor_obj = (engine_container_p)nfc->cb->m_collisionObject->getUserPointer();
    vec3_copy(n0, nfc->floor_normale.m_floats);
    n0[3] = -vec3_dot(n0, nfc->floor_point.m_floats);
    
    from = nfc->floor_point;
    from.m_floats[0] -= ent->transform[4 + 0] * ent->character->climb_r * 2.0;
    from.m_floats[1] -= ent->transform[4 + 1] * ent->character->climb_r * 2.0;
    to = from;
    to.m_floats[0] += ent->transform[4 + 0] * ent->character->climb_r * 2.0;
    to.m_floats[1] += ent->transform[4 + 1] * ent->character->climb_r * 2.0;
    
    t1.setIdentity();
    t2.setIdentity();
    
    do
    {
        t1.setOrigin(from);
        t2.setOrigin(to);
        nfc->ccb->m_closestHitFraction = 1.0;
        nfc->ccb->m_hitCollisionObject = NULL;
        bt_engine_dynamicsWorld->convexSweepTest(ent->character->climb_sensor, t1, t2, *nfc->ccb);
        if(nfc->ccb->hasHit())
        {
            vec3_copy(n1, nfc->ccb->m_hitNormalWorld.m_floats);
            n1[3] = -vec3_dot(n1, nfc->ccb->m_hitPointWorld.m_floats);
        }
        else
        {
            return 0;
        }

        // get the character plane equation
        vec3_copy(n2, ent->transform + 0);
        n2[3] = -vec3_dot(n2, pos);

        /*
         * Solve system of the linear equations by Kramer method!
         * I know - It may be slow, but it has a good precision!
         * The root is point of 3 planes intersection.
         */
        d =-n0[0] * (n1[1] * n2[2] - n1[2] * n2[1]) + 
            n1[0] * (n0[1] * n2[2] - n0[2] * n2[1]) - 
            n2[0] * (n0[1] * n1[2] - n0[2] * n1[1]);

        from.m_floats[2] -= ent->character->climb_r;
        to.m_floats[2] -= ent->character->climb_r;
    }
    while(fabs(d) < 0.001);
    
    nfc->edge_point.m_floats[0] = n0[3] * (n1[1] * n2[2] - n1[2] * n2[1]) - 
                                  n1[3] * (n0[1] * n2[2] - n0[2] * n2[1]) + 
                                  n2[3] * (n0[1] * n1[2] - n0[2] * n1[1]);
    nfc->edge_point.m_floats[0] /= d;

    nfc->edge_point.m_floats[1] = n0[0] * (n1[3] * n2[2] - n1[2] * n2[3]) - 
                                  n1[0] * (n0[3] * n2[2] - n0[2] * n2[3]) + 
                                  n2[0] * (n0[3] * n1[2] - n0[2] * n1[3]);
    nfc->edge_point.m_floats[1] /= d;

    nfc->edge_point.m_floats[2] = n0[0] * (n1[1] * n2[3] - n1[3] * n2[1]) - 
                                  n1[0] * (n0[1] * n2[3] - n0[3] * n2[1]) + 
                                  n2[0] * (n0[1] * n1[3] - n0[3] * n1[1]);
    nfc->edge_point.m_floats[2] /= d;    
    /*
     * let us filter phantom climbing!
     */
#if 0
    from.m_floats[0] = pos[0];
    from.m_floats[1] = pos[1];
    from.m_floats[2] = ((nfc->edge_point.m_floats[2] > nfc->floor_point.m_floats[2])?(nfc->edge_point.m_floats[2]):(nfc->floor_point.m_floats[2])) + 1.0;
    to = nfc->floor_point;
    to.m_floats[2] = from.m_floats[2];
    nfc->cb->m_closestHitFraction = 1.0;
    nfc->cb->m_collisionObject = NULL;
    bt_engine_dynamicsWorld->rayTest(from, to, *nfc->cb);
    if(nfc->cb->hasHit() && (nfc->cb->m_closestHitFraction < 0.99))
    {
        nfc->floor_hit = 0x00;
        return 0;
    }
#endif
    /*
     * Now, let us calculate z_angle
     */
    nfc->edge_hit = 0x01;
    
    /*
     * unclimbable edge slant %)
     */
    vec3_cross(n2, n0, n1);
    d = ent->character->critical_slant_z_component;
    d *= d * (n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2]);
    if(n2[2] * n2[2] > d)
    {
        nfc->edge_hit = 0x00;
        return 0x00;
    }
    
    n2[2] = n2[0];
    n2[0] = n2[1];
    n2[1] =-n2[2];
    n2[2] = 0.0;
    if(n2[0] * ent->transform[4 + 0] + n2[1] * ent->transform[4 + 1] > 0)       // direction fixing
    {
        n2[0] = -n2[0];
        n2[1] = -n2[1];
    }

    nfc->edge_z_ang = 180.0 * atan2f(n2[0], -n2[1]) / M_PI;
    nfc->edge_tan_xy.m_floats[0] = -n2[1];
    nfc->edge_tan_xy.m_floats[1] = n2[0];
    nfc->edge_tan_xy.m_floats[2] = 0.0;
    nfc->edge_tan_xy /= btSqrt(n2[0] * n2[0] + n2[1] * n2[1]);
    
    if(!nfc->ceiling_hit || (nfc->ceiling_point.m_floats[2] - nfc->floor_point.m_floats[2] >= ent->character->Height))
    {
        return 0x03;
    }
    else if((test_height > 0.0) && (nfc->ceiling_point.m_floats[2] - nfc->floor_point.m_floats[2] >= test_height))
    {
        return 0x08;
    }
    
    return 0x01;
}

/**
 * It is from bullet_character_controller
 */
int Character_RecoverFromPenetration(btPairCachingGhostObject *ghost, btManifoldArray *manifoldArray, btScalar react[3])
{
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    int i, j, k, ret = 0;
    int num_pairs, manifolds_size;
    const btCollisionShape *cs = ghost->getCollisionShape();
    btBroadphasePairArray &pairArray = ghost->getOverlappingPairCache()->getOverlappingPairArray();
    btVector3 aabb_min, aabb_max, pos = ghost->getWorldTransform().getOrigin();
    btScalar koef;
    
    cs->getAabb(ghost->getWorldTransform(), aabb_min, aabb_max);
    bt_engine_dynamicsWorld->getBroadphase()->setAabb(ghost->getBroadphaseHandle(), aabb_min, aabb_max, bt_engine_dynamicsWorld->getDispatcher());
    bt_engine_dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), bt_engine_dynamicsWorld->getDispatchInfo(), bt_engine_dynamicsWorld->getDispatcher());

    vec3_set_zero(react);    
    num_pairs = ghost->getOverlappingPairCache()->getNumOverlappingPairs();
    for(i=0;i<num_pairs;i++)
    {
        manifoldArray->clear();
        // do not use commented code: it prevents to collision skips. 
        //btBroadphasePair &pair = pairArray[i];
        //btBroadphasePair* collisionPair = bt_engine_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
        btBroadphasePair *collisionPair = &pairArray[i];

        if(!collisionPair)
        {
            continue;
        }

        if (collisionPair->m_algorithm)
        {
            collisionPair->m_algorithm->getAllContactManifolds(*manifoldArray);
        }

        manifolds_size = manifoldArray->size();
        for(j=0;j<manifolds_size;j++)
        {
            btPersistentManifold* manifold = (*manifoldArray)[j];
            btScalar directionSign = manifold->getBody0() == ghost ? btScalar(-1.0) : btScalar(1.0);
            for(k=0;k<manifold->getNumContacts();k++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(k);
                btScalar dist = pt.getDistance();

                if (dist < 0.0)
                {
                    koef = directionSign * btScalar(0.2);
                    pos += pt.m_normalWorldOnB * koef * dist;
                    react[0] += pt.m_normalWorldOnB.m_floats[0] * koef;
                    react[1] += pt.m_normalWorldOnB.m_floats[1] * koef;
                    react[2] += pt.m_normalWorldOnB.m_floats[2] * koef;
                    ret = 1;
                }
            }
        }
    }

    if(ret)
    {
        btTransform newTrans = ghost->getWorldTransform();
        newTrans.setOrigin(pos);
        ghost->setWorldTransform(newTrans);
    }

    return ret;
}


void Character_FixPenetrations(struct entity_s *ent, character_command_p cmd/*, struct height_info_s *fc*/, btScalar move[3])
{
    int numPenetrationLoops = 0;
    btVector3 pos;
    btScalar t1, t2, reaction[3], tmp[3];
    
    vec3_set_zero(reaction);
    ent->character->ghostObject->getWorldTransform().setFromOpenGLMatrix(ent->transform);
    ent->character->ghostObject->getWorldTransform().getOrigin() += ent->collision_offset;
    cmd->horizontal_collide = 0x00;
    
    while(Character_RecoverFromPenetration(ent->character->ghostObject, ent->character->manifoldArray, tmp))
    {
        numPenetrationLoops++;
        vec3_add(reaction, reaction, tmp);
        
        if(numPenetrationLoops > 4)                                             ///@FIXME: magic
        {
            break;
        }
    }
    
    if(move && numPenetrationLoops > 0)
    {
        t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        t2 = move[0] * move[0] + move[1] * move[1];
        if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))    
        {
            t2 *= t1;
            t1 = reaction[0] * move[0] + reaction[1] * move[1];
            if(t1 > 0.0)
            {
                t1 = t1 * t1 / t2;

                if(t1 > ent->character->critical_wall_component * ent->character->critical_wall_component)
                {
                    cmd->horizontal_collide |= 0x01;
                }
            }
        }
    }
    
    pos = ent->character->ghostObject->getWorldTransform().getOrigin();    
    if(ent->character->height_info.ceiling_hit && (pos.m_floats[2] > ent->character->height_info.ceiling_point.m_floats[2]))
    {
        pos.m_floats[2] = ent->character->height_info.ceiling_point.m_floats[2] - ent->character->Radius;
        ent->character->cmd.vertical_collide |= 0x02;
    }

    pos -= ent->collision_offset;
    if(ent->character->height_info.floor_hit && pos.m_floats[2] < ent->character->height_info.floor_point.m_floats[2])
    {
        pos.m_floats[2] = ent->character->height_info.floor_point.m_floats[2];
        ent->character->cmd.vertical_collide |= 0x01;
    }
    
    vec3_copy(ent->transform+12, pos.m_floats);
}

/**
 * we check walls and other collision objects reaction. if reaction more then critacal 
 * then cmd->horizontal_collide |= 0x01;
 * @param ent - cheked entity
 * @param cmd - here we fill cmd->horizontal_collide field
 * @param move - absolute 3d move vector
 */
void Character_CheckNextPenetration(struct entity_s *ent, character_command_p cmd, btScalar move[3])
{
    btVector3 pos;
    btScalar t1, t2, reaction[3];
    
    vec3_add(pos.m_floats, ent->collision_offset.m_floats, move);
    ent->character->ghostObject->getWorldTransform().setFromOpenGLMatrix(ent->transform);
    ent->character->ghostObject->getWorldTransform().getOrigin() += pos;
    cmd->horizontal_collide = 0x00;
    
    if(Character_RecoverFromPenetration(ent->character->ghostObject, ent->character->manifoldArray, reaction))
    {
        t1 = reaction[0] * reaction[0] + reaction[1] * reaction[1];
        t2 = move[0] * move[0] + move[1] * move[1];
        if((reaction[2] * reaction[2] < t1) && (move[2] * move[2] < t2))    
        {
            t2 *= t1;
            t1 = reaction[0] * move[0] + reaction[1] * move[1];
            t1 = t1 * t1 / t2;

            if(t1 > ent->character->critical_wall_component * ent->character->critical_wall_component)
            {
                cmd->horizontal_collide |= 0x01;
            }
        }
    }
}


void Character_UpdateCurrentSpeed(struct entity_s *ent, int zeroVz)
{
    btScalar t, vz;

    t = ent->current_speed * ent->character->speed_mult;
    vz = (zeroVz)?(0.0):(ent->character->speed.m_floats[2]);
    
    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(ent->character->speed.m_floats, ent->transform+4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(ent->character->speed.m_floats, ent->transform+4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(ent->character->speed.m_floats, ent->transform+0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(ent->character->speed.m_floats, ent->transform+0, t);
    }
    else
    {
        //ent->dir_flag = ENT_MOVE_FORWARD;
    }
    
    ent->character->speed.m_floats[2] = vz;
}

int Character_SetToJump(struct entity_s *ent, character_command_p cmd, btScalar vz)
{
    btScalar t;
    btVector3 spd(0.0, 0.0, 0.0);

    if(!ent->character)
    {
        return 0;
    }

    //ent->angles[0] += cmd->rot[0];
    //Entity_UpdateRotation(ent);
    t = ent->current_speed * ent->character->speed_mult;

    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+0, t);
    }
    else
    {
        ent->dir_flag = ENT_MOVE_FORWARD;
    }

    cmd->vertical_collide = 0x00;
    cmd->slide = 0x00;
    
    ent->character->speed += spd;
    ent->character->speed.m_floats[2] = vz;
    ent->move_type = MOVE_FREE_FALLING;

    return 0;
}


/*
 * MOVE IN DIFFERENCE CONDITIONS
 */
int Character_MoveOnFloor(struct entity_s *ent, character_command_p cmd)
{
    int i, iter;
    btVector3 tv, norm_move_xy, move, spd(0.0, 0.0, 0.0);
    btScalar fc_pos[3], norm_move_xy_len, t, ang, *pos = ent->transform + 12;
    height_info_t nfc;
    
    if(!ent->character)
    {
        return 0;
    }

    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    
    /*
     * resize collision model
     */
    Character_UpdateCollisionObject(ent, 0.5 * (ent->character->max_step_up_height + ent->character->min_step_up_height));

    /*
     * init height info structure
     */
    nfc.cb = ent->character->ray_cb;
    nfc.ccb = ent->character->convex_cb;
    cmd->horizontal_collide = 0x00;
    cmd->vertical_collide = 0x00;
    // First of all - get information about floor and ceiling!!!
    vec3_copy(fc_pos, pos);
    fc_pos[2] += ent->collision_offset.m_floats[2];
    Character_GetHeightInfo(fc_pos, &ent->character->height_info);

    /*
     * check move type
     */
    if(ent->character->height_info.floor_hit)
    {
        if(ent->character->height_info.floor_point.m_floats[2] + ent->character->fall_down_height < pos[2])
        {
            ent->move_type = MOVE_FREE_FALLING;
            ent->character->speed.m_floats[2] = 0.0;
            return -1;                                                          // nothing to do here
        }
        else
        {
            cmd->vertical_collide |= 0x01;
        }
        
        tv = ent->character->height_info.floor_normale;
        if(tv.m_floats[2] > 0.02 && tv.m_floats[2] < ent->character->critical_slant_z_component)
        {
            tv.m_floats[2] = -tv.m_floats[2];
            spd = tv * ent->character->speed_mult * DEFAULT_CHARACTER_SLIDE_SPEED_MULT; // slide down direction
            ang = 180.0 * atan2f(tv.m_floats[0], -tv.m_floats[1]) / M_PI;        // from -180 deg to +180 deg
            //ang = (ang < 0.0)?(ang + 360.0):(ang);
            t = tv.m_floats[0] * ent->transform[4] + tv.m_floats[1] * ent->transform[5];
            if(t >= 0.0)
            {
                cmd->slide = CHARACTER_SLIDE_FRONT;
                ent->angles[0] = ang + 180.0;
                // front forward sly down
            }
            else
            {
                cmd->slide = CHARACTER_SLIDE_BACK;
                ent->angles[0] = ang;
                // back forward sly down
            }
            Entity_UpdateRotation(ent);
            cmd->vertical_collide |= 0x01;
        }
        else                                                                    // no slide - free to walk
        {
            t = ent->current_speed * ent->character->speed_mult;
            t = (t < 0.0)?(0.0):(t);                                            /// stick or feature: that is a serious question!
            cmd->vertical_collide |= 0x01;
            ent->angles[0] += cmd->rot[0];
            Entity_UpdateRotation(ent);                                         // apply rotations

            if(ent->dir_flag & ENT_MOVE_FORWARD)
            {
                vec3_mul_scalar(spd.m_floats, ent->transform+4, t);
            }
            else if(ent->dir_flag & ENT_MOVE_BACKWARD)
            {
                vec3_mul_scalar(spd.m_floats, ent->transform+4,-t);
            }
            else if(ent->dir_flag & ENT_MOVE_LEFT)
            {
                vec3_mul_scalar(spd.m_floats, ent->transform+0,-t);
            }
            else if(ent->dir_flag & ENT_MOVE_RIGHT)
            {
                vec3_mul_scalar(spd.m_floats, ent->transform+0, t);
            }
            else
            {
                //ent->dir_flag = ENT_MOVE_FORWARD;
            }
            cmd->slide = 0x00;
        }
    }
    else                                                                        // no hit to the floor
    {
        cmd->slide = 0x00;
        cmd->vertical_collide = 0x00;
        ent->move_type = MOVE_FREE_FALLING;
        ent->character->speed.m_floats[2] = 0.0;
        return -1;                                                              // nothing to do here
    }

    /*
     * now move normally
     */
    ent->character->speed = spd;
    move = spd * engine_frame_time;
    t = move.length();
    iter = 2.0 * t / ent->character->Radius + 1;
    if(iter < 1)
    {
        iter = 1;
    }
    move /= (btScalar)iter;
    norm_move_xy.m_floats[0] = move.m_floats[0];
    norm_move_xy.m_floats[1] = move.m_floats[1];
    norm_move_xy.m_floats[2] = 0.0;
    norm_move_xy_len = norm_move_xy.length();
    if(norm_move_xy_len * iter > 0.2 * t)
    {
        norm_move_xy /= norm_move_xy_len;
    }
    else
    {
        norm_move_xy_len = 32512.0;
        vec3_set_zero(norm_move_xy.m_floats);
    }
    
    for(i=0;i<iter && cmd->horizontal_collide==0x00;i++)
    {
        vec3_copy(fc_pos, pos);
        fc_pos[2] += ent->collision_offset.m_floats[2];
        Character_GetHeightInfo(fc_pos, &ent->character->height_info);
        vec3_add(pos, pos, move.m_floats);
        Character_FixPenetrations(ent, cmd, move.m_floats);                     // get horizontal collide

        if(ent->character->height_info.floor_hit)
        {
            if(ent->character->height_info.floor_point.m_floats[2] + ent->character->fall_down_height > pos[2])
            {
                if(pos[2] > ent->character->height_info.floor_point.m_floats[2])
                {
                    pos[2] -= engine_frame_time * 2400.0;                       ///@FIXME: magick
                }
            }
            else
            {
                ent->move_type = MOVE_FREE_FALLING;
                ent->character->speed.m_floats[2] = 0.0;
                vec3_copy(tv.m_floats, pos);
                tv += ent->collision_offset;
                ent->self->room = Room_FindPosCogerrence(&engine_world, tv.m_floats, ent->self->room);
                return 2;
            }            
            if(pos[2] < ent->character->height_info.floor_point.m_floats[2])
            {
                pos[2] = ent->character->height_info.floor_point.m_floats[2];
                cmd->vertical_collide |= 0x01;
            }
        }
        else
        {
            ent->move_type = MOVE_FREE_FALLING;
            ent->character->speed.m_floats[2] = 0.0;
            Character_UpdateCurrentRoom(ent);
            return 2;
        }

        Character_UpdateCurrentRoom(ent);
    }

    return iter;
}


int Character_FreeFalling(struct entity_s *ent, character_command_p cmd)
{
    int i, iter;
    btVector3 move;
    btScalar fc_pos[3], t, *pos = ent->transform + 12;

    if(!ent->character)
    {
        return 0;
    }

    /*
     * resize collision model
     */
    Character_UpdateCollisionObject(ent, 0.0);

    /*
     * init height info structure
     */
    
    cmd->slide = 0x00;
    cmd->horizontal_collide = 0x00;
    cmd->vertical_collide = 0x00;
    ent->angles[0] += cmd->rot[0] * 0.5;                                        ///@FIXME magic const
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateRotation(ent);                                                 // apply rotations
    
    move = ent->character->speed + bt_engine_dynamicsWorld->getGravity() * engine_frame_time * 0.5;
    move *= engine_frame_time;
    ent->character->speed += bt_engine_dynamicsWorld->getGravity() * engine_frame_time;
    vec3_RotateZ(ent->character->speed.m_floats, ent->character->speed.m_floats, cmd->rot[0] * 0.5); ///@FIXME magic const
    
    t = move.length();
    iter = 2.0 * t / ent->character->Radius + 1;
    if(iter < 1)
    {
        iter = 1;
    }
    move /= (btScalar)iter;
    
    vec3_copy(fc_pos, pos);
    fc_pos[2] += ent->collision_offset.m_floats[2];
    Character_GetHeightInfo(fc_pos, &ent->character->height_info);
    
    if(ent->self->room && (ent->self->room->flags & 0x01))
    {
        if(ent->character->speed.m_floats[2] < 0.0)
        {
            ent->current_speed = 0.0;
            ent->character->speed.m_floats[0] = 0.0;
            ent->character->speed.m_floats[1] = 0.0;
        } 
        
        if(!ent->character->height_info.water || (pos[2] + ent->character->Height < ent->character->height_info.water_level))
        {
            ent->move_type = MOVE_UNDER_WATER;
            return 2;
        }
    }
    
    if(ent->character->height_info.ceiling_hit && ent->character->speed.m_floats[2] > 0.0)
    {
        if(ent->character->height_info.ceiling_point.m_floats[2] < ent->bf.bb_max[2] + pos[2])
        {
            pos[2] = ent->character->height_info.ceiling_point.m_floats[2] - ent->bf.bb_max[2];
            ent->character->speed.m_floats[2] = 0.0;
            cmd->vertical_collide |= 0x02;
            vec3_copy(fc_pos, pos);
            fc_pos[2] += ent->collision_offset.m_floats[2];
            Character_GetHeightInfo(fc_pos, &ent->character->height_info);
            Character_FixPenetrations(ent, cmd, move);
        }
    }
    if(ent->character->height_info.floor_hit && ent->character->speed.m_floats[2] < 0.0)                 // move down
    {
        if(ent->character->height_info.floor_point.m_floats[2] >= pos[2] + ent->bf.bb_min[2] + move.m_floats[2])
        {
            pos[2] = ent->character->height_info.floor_point.m_floats[2];
            //ent->character->speed.m_floats[2] = 0.0;
            ent->move_type = MOVE_ON_FLOOR;
            cmd->vertical_collide |= 0x01;
            Character_UpdateCurrentRoom(ent);
            vec3_copy(fc_pos, pos);
            fc_pos[2] += ent->collision_offset.m_floats[2];
            Character_GetHeightInfo(fc_pos, &ent->character->height_info);
            Character_FixPenetrations(ent, cmd, move);
            return 2;
        }
    }

    for(i=0;i<iter && cmd->horizontal_collide==0x00;i++)
    {
        vec3_copy(fc_pos, pos);
        fc_pos[2] += ent->collision_offset.m_floats[2];
        Character_GetHeightInfo(fc_pos, &ent->character->height_info);
        vec3_add(pos, pos, move.m_floats);
        Character_FixPenetrations(ent, cmd, move.m_floats);                // get horizontal collide

        if(ent->character->height_info.ceiling_hit && ent->character->speed.m_floats[2] > 0.0)
        {
            if(ent->character->height_info.ceiling_point.m_floats[2] < ent->bf.bb_max[2] + pos[2])
            {
                pos[2] = ent->character->height_info.ceiling_point.m_floats[2] - ent->bf.bb_max[2];
                ent->character->speed.m_floats[2] = 0.0;
                cmd->vertical_collide |= 0x02;
            }
        }
        if(ent->character->height_info.floor_hit && ent->character->speed.m_floats[2] < 0.0)             // move down
        {
            if(ent->character->height_info.floor_point.m_floats[2] >= pos[2] + ent->bf.bb_min[2] + move.m_floats[2])
            {
                pos[2] = ent->character->height_info.floor_point.m_floats[2];
                //ent->character->speed.m_floats[2] = 0.0;
                ent->move_type = MOVE_ON_FLOOR;
                cmd->vertical_collide |= 0x01;
                Character_UpdateCurrentRoom(ent);
                vec3_copy(fc_pos, pos);
                fc_pos[2] += ent->collision_offset.m_floats[2];
                Character_GetHeightInfo(fc_pos, &ent->character->height_info);
                Character_FixPenetrations(ent, cmd, move);
                return 2;
            }
        }
        
        Character_UpdateCurrentRoom(ent);
    }

    return iter;
}


/*
 * CLIMBING - MOVE NO Z LANDING
 */
int Character_Climbing(struct entity_s *ent, character_command_p cmd)
{
    int i, iter;
    btVector3 move, spd(0.0, 0.0, 0.0);
    btScalar fc_pos[3], t, *pos = ent->transform + 12;
    btScalar z = pos[2];
    
    /*
     * resize collision model
     */
    Character_UpdateCollisionObject(ent, 0.0);

    cmd->slide = 0x00;
    cmd->horizontal_collide = 0x00;
    cmd->vertical_collide = 0x00;
    
    t = ent->current_speed * ent->character->speed_mult;
    cmd->vertical_collide |= 0x01;
    ent->angles[0] += cmd->rot[0];
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateRotation(ent);                                                 // apply rotations

    if(ent->dir_flag & ENT_MOVE_FORWARD)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+4, t);
    }
    else if(ent->dir_flag & ENT_MOVE_BACKWARD)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+4,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_LEFT)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+0,-t);
    }
    else if(ent->dir_flag & ENT_MOVE_RIGHT)
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+0, t);
    }
    else
    {
        //ent->dir_flag = ENT_MOVE_FORWARD;
    }
    cmd->slide = 0x00;
    
    ent->character->speed = spd;
    move = spd * engine_frame_time;
    t = move.length();
    iter = 2.0 * t / ent->character->Radius + 1;
    if(iter < 1)
    {
        iter = 1;
    }
    move /= (btScalar)iter;
            
    for(i=0;i<iter && cmd->horizontal_collide==0x00;i++)
    {
        vec3_copy(fc_pos, pos);
        fc_pos[2] += ent->collision_offset.m_floats[2];
        Character_GetHeightInfo(fc_pos, &ent->character->height_info);
        vec3_add(pos, pos, move.m_floats);
        if(ent->character->no_fix == 0)
        {
            Character_FixPenetrations(ent, cmd, move.m_floats);                 // get horizontal collide
        }

        Character_UpdateCurrentRoom(ent);
    }
    
    pos[2] = z;
    ent->character->no_fix = 0;
    
    return 1;
}

/*
 * underwater and onwater swimming has a big trouble:
 * the speed and acceleration information is absent...
 * I add some sticks to make it work for testing.
 * I thought to make export anim information to LUA script...
 */
int Character_MoveUnderWater(struct entity_s *ent, character_command_p cmd)
{
    int i, iter;
    btVector3 move, spd(0.0, 0.0, 0.0);
    btScalar fc_pos[3], t, *pos = ent->transform + 12;
    
    /*
     * check current place
     */
    if(ent->self->room && !(ent->self->room->flags & 0x01))
    {
        ent->move_type = MOVE_FREE_FALLING;
        return 2;
    }
    /*
     * resize collision model
     */
    Character_UpdateCollisionObject(ent, 0.0);

    cmd->slide = 0x00;
    cmd->horizontal_collide = 0x00;
    cmd->vertical_collide = 0x00;
    
    t = 64.0 * ent->character->speed_mult * cmd->jump;                          ///@FIXME: magick!
    
    ent->angles[0] += cmd->rot[0];
    ent->angles[1] -= cmd->rot[1];
    ent->angles[2] = 0.0;
    if((ent->angles[1] > 70.0) && (ent->angles[1] < 180.0))                     // Underwater angle limiter.
    {
       ent->angles[1] = 70.0;
    }
    else if((ent->angles[1] > 180.0) && (ent->angles[1] < 270.0))
    {
        ent->angles[1] = 270.0;
    }
    Entity_UpdateRotation(ent);                                                 // apply rotations

    vec3_mul_scalar(spd.m_floats, ent->transform+4, t);
    ent->character->speed = spd;
    move = spd * engine_frame_time;
    t = move.length();
    iter = 2.0 * t / ent->character->Radius + 1;
    if(iter < 1)
    {
        iter = 1;
    }
    move /= (btScalar)iter;
            
    for(i=0;i<iter && cmd->horizontal_collide==0x00;i++)
    {
        vec3_copy(fc_pos, pos);
        fc_pos[2] += ent->collision_offset.m_floats[2];
        Character_GetHeightInfo(fc_pos, &ent->character->height_info);
        vec3_add(pos, pos, move.m_floats);
        Character_FixPenetrations(ent, cmd, move.m_floats);                     // get horizontal collide

        Character_UpdateCurrentRoom(ent);
        if(ent->character->height_info.water && (pos[2] + ent->bf.bb_max[2] >= ent->character->height_info.water_level))
        {
            if(/*(spd.m_floats[2] > 0.0)*/ent->transform[4 + 2] > 0.67)         ///@FIXME: magick!
            {
                ent->move_type = MOVE_ON_WATER;
                //pos[2] = fc.water_level;
                return 2;
            }
            if(!ent->character->height_info.floor_hit || (ent->character->height_info.water_level - ent->character->height_info.floor_point.m_floats[2] >= ent->character->Height))
            {
                pos[2] = ent->character->height_info.water_level - ent->bf.bb_max[2];
            }
        }
    }
    
    return 1;
}


int Character_MoveOnWater(struct entity_s *ent, character_command_p cmd)
{
    int i, iter;
    btVector3 move, spd(0.0, 0.0, 0.0);
    btScalar fc_pos[3], t, *pos = ent->transform + 12;
       
    /*
     * resize collision model
     */
    Character_UpdateCollisionObject(ent, 0.0);

    cmd->slide = 0x00;
    cmd->horizontal_collide = 0x00;
    cmd->vertical_collide = 0x00;
    
    ent->angles[0] += cmd->rot[0];
    ent->angles[1] = 0.0;
    ent->angles[2] = 0.0;
    Entity_UpdateRotation(ent);                                                 // apply rotations
    
    /*
     * Find speed
     */
    //t = ent->current_speed * ent->character->speed_mult;
    t = 24.0 * ent->character->speed_mult;                                      ///@FIXME: magick!
    t = (t < 0.0)?(0.0):(t);                                                    /// stick or feature: that is a serious question!
    if((ent->dir_flag & ENT_MOVE_FORWARD) && (cmd->move[0] == 1))
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+4, t);
    }
    else if((ent->dir_flag & ENT_MOVE_BACKWARD) && (cmd->move[0] == -1))
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+4,-t);
    }
    else if((ent->dir_flag & ENT_MOVE_LEFT) && (cmd->move[1] == -1))
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+0,-t);
    }
    else if((ent->dir_flag & ENT_MOVE_RIGHT) && (cmd->move[1] == 1))
    {
        vec3_mul_scalar(spd.m_floats, ent->transform+0, t);
    }
    else
    {
        //ent->dir_flag = ENT_MOVE_FORWARD;
    }
    
    /*
     * Prepare to moving
     */    
    ent->character->speed = spd;
    move = spd * engine_frame_time;
    t = move.length();
    iter = 2.0 * t / ent->character->Radius + 1;
    if(iter < 1)
    {
        iter = 1;
    }
    move /= (btScalar)iter;
            
    for(i=0;i<iter && cmd->horizontal_collide==0x00;i++)
    {
        vec3_copy(fc_pos, pos);
        fc_pos[2] += ent->collision_offset.m_floats[2];
        Character_GetHeightInfo(fc_pos, &ent->character->height_info);
        vec3_add(pos, pos, move.m_floats);
        Character_FixPenetrations(ent, cmd, move.m_floats);                     // get horizontal collide

        Character_UpdateCurrentRoom(ent);
        if(ent->character->height_info.water)
        {
            pos[2] = ent->character->height_info.water_level;
        }
        else
        {
            ent->move_type = MOVE_ON_FLOOR;
            return 2;
        }
    }
    
    return 1;
}


