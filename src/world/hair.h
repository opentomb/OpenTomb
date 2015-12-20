#pragma once

#include <cstdint>

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "engine/engine.h"
#include "world/entity.h"
#include "world/world.h"

namespace world
{

#define HAIR_TR1       0
#define HAIR_TR2       1
#define HAIR_TR3       2
#define HAIR_TR4_KID_1 3
#define HAIR_TR4_KID_2 4
#define HAIR_TR4_OLD   5
#define HAIR_TR5_KID_1 6
#define HAIR_TR5_KID_2 7
#define HAIR_TR5_OLD   8

struct HairElement
{
    std::shared_ptr<core::BaseMesh> mesh;           // Pointer to rendered mesh.
    std::unique_ptr<btCollisionShape> shape;          // Pointer to collision shape.
    std::shared_ptr<btRigidBody> body;           // Pointer to dynamic body.
    glm::vec3 position;     // Position of this hair element
    // relative to the model (NOT the parent!). Should be a matrix in theory,
    // but since this never has a rotation part, we can save a few bytes here.
};

struct HairSetup;

struct Hair : public Object
{
    std::weak_ptr<Entity> m_ownerChar;         // Entity who owns this hair.
    uint32_t m_ownerBody = 0;         // Owner entity's body ID.
    glm::mat4 m_ownerBodyHairRoot{ 1.0f }; // transform from owner body to root of hair start

    uint8_t m_rootIndex = 0;         // Index of "root" element.
    uint8_t m_tailIndex = 0;         // Index of "tail" element.

    std::vector<HairElement> m_elements;           // Array of elements.

    std::vector<std::unique_ptr<btGeneric6DofConstraint>> m_joints;             // Array of joints.

    std::shared_ptr<core::BaseMesh> m_mesh = nullptr;               // Mesh containing all vertices of all parts of this hair object.

    explicit Hair(Room* room = nullptr)
        : Object(0, room)
    {
    }

    ~Hair();

    // Creates hair into allocated hair structure, using previously defined setup and
    // entity index.
    bool create(HairSetup* setup, std::shared_ptr<Entity> parent_entity);

private:
    void createHairMesh(const SkeletalModel& model);
};

struct HairSetup
{
    ModelId      m_model;              // Hair model ID
    uint32_t     m_linkBody;          // Lara's head mesh index

    glm::float_t     m_rootWeight;        // Root and tail hair body weight. Intermediate body
    glm::float_t     m_tailWeight;        // weights are calculated from these two parameters

    glm::float_t     m_hairDamping[2];    // Damping affects hair "plasticity"
    glm::float_t     m_hairInertia;       // Inertia affects hair "responsiveness"
    glm::float_t     m_hairRestitution;   // "Bounciness" of the hair
    glm::float_t     m_hairFriction;      // How much other bodies will affect hair trajectory

    glm::float_t     m_jointOverlap;      // How much two hair bodies overlap each other

    glm::float_t     m_jointCfm;          // Constraint force mixing (joint softness)
    glm::float_t     m_jointErp;          // Error reduction parameter (joint "inertia")

    glm::vec3    m_headOffset;        // Linear offset to place hair to
    glm::vec3    m_rootAngle;      // First constraint set angle (to align hair angle)

    // Gets scripted hair set-up to specified hair set-up structure.
    void getSetup(int hair_entry_index);
};

} // namespace world
