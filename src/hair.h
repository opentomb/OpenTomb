#pragma once

#include <cassert>
#include <cstdint>

#include <lua.hpp>

#include <LinearMath/btScalar.h>
#include <LinearMath/btVector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "character_controller.h"
#include "engine.h"
#include "entity.h"
#include "game.h"
#include "mesh.h"
#include "script.h"
#include "vmath.h"
#include "world.h"

#define HAIR_TR1       0
#define HAIR_TR2       1
#define HAIR_TR3       2
#define HAIR_TR4_KID_1 3
#define HAIR_TR4_KID_2 4
#define HAIR_TR4_OLD   5
#define HAIR_TR5_KID_1 6
#define HAIR_TR5_KID_2 7
#define HAIR_TR5_OLD   8

// Since we apply TR4 hair scheme to TR2-3 as well, we need to discard
// polygons which are unused. These are 0 and 5 in both TR2 and TR3.

#define HAIR_DISCARD_ROOT_FACE 0
#define HAIR_DISCARD_TAIL_FACE 5

struct HairElement
{
    std::shared_ptr<BaseMesh> mesh;           // Pointer to rendered mesh.
    std::unique_ptr<btCollisionShape> shape;          // Pointer to collision shape.
    std::shared_ptr<btRigidBody> body;           // Pointer to dynamic body.
    btVector3 position;     // Position of this hair element
    // relative to the model (NOT the parent!). Should be a matrix in theory,
    // but since this never has a rotation part, we can save a few bytes here.
};

struct HairSetup;

struct Hair : public Object
{
    std::unique_ptr<EngineContainer> m_container;

    std::weak_ptr<Entity> m_ownerChar;         // Entity who owns this hair.
    uint32_t m_ownerBody;         // Owner entity's body ID.
    btTransform m_ownerBodyHairRoot; // transform from owner body to root of hair start

    uint8_t m_rootIndex;         // Index of "root" element.
    uint8_t m_tailIndex;         // Index of "tail" element.

    std::vector<HairElement> m_elements;           // Array of elements.

    std::vector<std::unique_ptr<btGeneric6DofConstraint>> m_joints;             // Array of joints.

    std::shared_ptr<BaseMesh> m_mesh;               // Mesh containing all vertices of all parts of this hair object.

    ~Hair();

    // Creates hair into allocated hair structure, using previously defined setup and
    // entity index.
    bool create(HairSetup* setup, std::shared_ptr<Entity> parent_entity);

private:
    void createHairMesh(const SkeletalModel *model);
};

struct HairSetup
{
    uint32_t     m_model;              // Hair model ID
    uint32_t     m_linkBody;          // Lara's head mesh index

    btScalar     m_rootWeight;        // Root and tail hair body weight. Intermediate body
    btScalar     m_tailWeight;        // weights are calculated from these two parameters

    btScalar     m_hairDamping[2];    // Damping affects hair "plasticity"
    btScalar     m_hairInertia;       // Inertia affects hair "responsiveness"
    btScalar     m_hairRestitution;   // "Bounciness" of the hair
    btScalar     m_hairFriction;      // How much other bodies will affect hair trajectory

    btScalar     m_jointOverlap;      // How much two hair bodies overlap each other

    btScalar     m_jointCfm;          // Constraint force mixing (joint softness)
    btScalar     m_jointErp;          // Error reduction parameter (joint "inertia")

    btVector3    m_headOffset;        // Linear offset to place hair to
    btVector3    m_rootAngle;      // First constraint set angle (to align hair angle)

    // Gets scripted hair set-up to specified hair set-up structure.
    void getSetup(uint32_t hair_entry_index);
};
