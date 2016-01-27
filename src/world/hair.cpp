#include "hair.h"

#include <algorithm>

#include "LuaState.h"

#include "engine/bullet.h"
#include "render/render.h"
#include "script/script.h"
#include "world/core/basemesh.h"
#include "world/skeletalmodel.h"

#include <glm/gtc/type_ptr.hpp>

namespace world
{
bool Hair::create(HairSetup *setup, const Entity& parent_entity)
{
    // No setup or parent to link to - bypass function.

    if(   !setup
       || setup->m_linkBody >= parent_entity.m_skeleton.getBoneCount()
       || !parent_entity.m_skeleton.getBones()[setup->m_linkBody].bt_body)
    {
        return false;
    }

    auto model = getWorld()->getModelByID(setup->m_model);

    // No model to link to - bypass function.

    if(!model || !model->hasMeshReferences())
        return false;

    // Setup engine container. FIXME: DOESN'T WORK PROPERLY ATM.

    setRoom(parent_entity.getRoom());

    // Setup initial hair parameters.

    m_ownerChar = &parent_entity;       // Entity to refer to.
    m_ownerBody = setup->m_linkBody;    // Entity body to refer to.

    // Setup initial position / angles.

    glm::mat4 owner_body_transform = parent_entity.m_transform * parent_entity.m_skeleton.getBones()[m_ownerBody].full_transform;
    // Number of elements (bodies) is equal to number of hair meshes.

    m_elements.clear();
    m_elements.resize(model->getMeshReferenceCount());

    // Root index should be always zero, as it is how engine determines that it is
    // connected to head and renders it properly. Tail index should be always the
    // last element of the hair, as it indicates absence of "child" constraint.

    m_rootIndex = 0;
    m_tailIndex = m_elements.size() - 1;

    // Weight step is needed to determine the weight of each hair body.
    // It is derived from root body weight and tail body weight.

    glm::float_t weight_step = (setup->m_rootWeight - setup->m_tailWeight) / m_elements.size();
    glm::float_t current_weight = setup->m_rootWeight;

    for(size_t i = 0; i < m_elements.size(); i++)
    {
        // Point to corresponding mesh.

        m_elements[i].mesh = model->getMeshReference(i).mesh_base;

        // Begin creating ACTUAL physical hair mesh.

        btVector3   localInertia(0, 0, 0);

        // Make collision shape out of mesh.

        m_elements[i].shape.reset(BT_CSfromMesh(m_elements[i].mesh, true, true, false));
        m_elements[i].shape->calculateLocalInertia(current_weight * setup->m_hairInertia, localInertia);

        // Decrease next body weight to weight_step parameter.

        current_weight -= weight_step;

        // Initialize motion state for body.

        btTransform startTransform;
        startTransform.setFromOpenGLMatrix(glm::value_ptr(owner_body_transform));
        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

        // Make rigid body.

        m_elements[i].body.reset(new btRigidBody(current_weight, motionState, m_elements[i].shape.get(), localInertia));

        // Damping makes body stop in space by itself, to prevent it from continous movement.

        m_elements[i].body->setDamping(setup->m_hairDamping[0], setup->m_hairDamping[1]);

        // Restitution and friction parameters define "bounciness" and "dullness" of hair.

        m_elements[i].body->setRestitution(setup->m_hairRestitution);
        m_elements[i].body->setFriction(setup->m_hairFriction);

        // Since hair is always moving with Lara, even if she's in still state (like, hanging
        // on a ledge), hair bodies shouldn't deactivate over time.

        m_elements[i].body->forceActivationState(DISABLE_DEACTIVATION);

        // Hair bodies must not collide with each other, and also collide ONLY with kinematic
        // bodies (e. g. animated meshes), or else Lara's ghost object or anything else will be able to
        // collide with hair!

        m_elements[i].body->setUserPointer(this);
        getWorld()->m_engine->bullet.dynamicsWorld->addRigidBody(m_elements[i].body.get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_KINEMATIC);

        m_elements[i].body->activate();
    }

    // GENERATE CONSTRAINTS.
    // All constraints are generic 6-DOF type, as they seem perfect fit for hair.

    // Joint count is calculated from overall body amount multiplied by per-body constraint
    // count.

    m_joints.resize(m_elements.size());

    // If multiple joints per body is specified, joints are placed in circular manner,
    // with obvious step of (SIMD_2_PI) / joint count. It means that all joints will form
    // circle-like figure.

    int curr_joint = 0;

    for(size_t i = 0; i < m_elements.size(); i++)
    {
        glm::float_t body_length;
        btTransform localA; localA.setIdentity();
        btTransform localB; localB.setIdentity();

        glm::float_t joint_x = 0.0;
        glm::float_t joint_y = 0.0;

        std::shared_ptr<btRigidBody> prev_body;
        if(i == 0)  // First joint group
        {
            // Adjust pivot point A to parent body.

            localA.setOrigin(util::convert(setup->m_headOffset + glm::vec3(joint_x, 0.0, joint_y)));
            localA.getBasis().setEulerZYX(setup->m_rootAngle[0], setup->m_rootAngle[1], setup->m_rootAngle[2]);
            // Stealing this calculation because I need it for drawing
            localA.getOpenGLMatrix(glm::value_ptr(m_ownerBodyHairRoot));

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0, -SIMD_HALF_PI, 0);

            prev_body = parent_entity.m_skeleton.getBones()[m_ownerBody].bt_body;   // Previous body is parent body.
        }
        else
        {
            // Adjust pivot point A to previous mesh's length, considering mesh overlap multiplier.

            body_length = glm::abs(m_elements[i - 1].mesh->m_boundingBox.max[1] - m_elements[i - 1].mesh->m_boundingBox.min[1]) * setup->m_jointOverlap;

            localA.setOrigin(btVector3(joint_x, body_length, joint_y));
            localA.getBasis().setEulerZYX(0, SIMD_HALF_PI, 0);

            // Pivot point B is automatically adjusted by Bullet.

            localB.setOrigin(btVector3(joint_x, 0.0, joint_y));
            localB.getBasis().setEulerZYX(0, SIMD_HALF_PI, 0);

            prev_body = m_elements[i - 1].body;   // Previous body is preceiding hair mesh.
        }

        // Create 6DOF constraint.

        m_joints[curr_joint].reset(new btGeneric6DofConstraint(*prev_body, *m_elements[i].body, localA, localB, true));

        // CFM and ERP parameters are critical for making joint "hard" and link
        // to Lara's head. With wrong values, constraints may become "elastic".

        for(int axis = 0; axis <= 5; axis++)
        {
            m_joints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->m_jointCfm, axis);
            m_joints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->m_jointErp, axis);
        }

        if(i == 0)
        {
            // First joint group should be more limited in motion, as it is connected
            // right to the head. NB: Should we make it scriptable as well?

            m_joints[curr_joint]->setLinearLowerLimit(btVector3(0., 0., 0.));
            m_joints[curr_joint]->setLinearUpperLimit(btVector3(0., 0., 0.));
            m_joints[curr_joint]->setAngularLowerLimit(btVector3(-SIMD_HALF_PI, 0., -SIMD_HALF_PI*0.4));
            m_joints[curr_joint]->setAngularUpperLimit(btVector3(-SIMD_HALF_PI*0.3, 0., SIMD_HALF_PI*0.4));

            // Increased solver iterations make constraint even more stable.

            m_joints[curr_joint]->setOverrideNumSolverIterations(100);
        }
        else
        {
            // Normal joint with more movement freedom.

            m_joints[curr_joint]->setLinearLowerLimit(btVector3(0., 0., 0.));
            m_joints[curr_joint]->setLinearUpperLimit(btVector3(0., 0., 0.));
            m_joints[curr_joint]->setAngularLowerLimit(btVector3(-SIMD_HALF_PI*0.5, 0., -SIMD_HALF_PI*0.5));
            m_joints[curr_joint]->setAngularUpperLimit(btVector3(SIMD_HALF_PI*0.5, 0., SIMD_HALF_PI*0.5));
        }

        m_joints[curr_joint]->setDbgDrawSize(glm::float_t(5.0f));    // Draw constraint axes.

        // Add constraint to the world.

        getWorld()->m_engine->bullet.dynamicsWorld->addConstraint(m_joints[curr_joint].get(), true);

        curr_joint++;   // Point to the next joint.
    }

    createHairMesh(*model);

    return true;
}

// Internal utility function:
// Creates a single mesh out of all the parts of the given model.
// This assumes that Mesh_GenFaces was already called on the parts of model.
void Hair::createHairMesh(const SkeletalModel& model)
{
    m_mesh = std::make_shared<core::BaseMesh>();
    m_mesh->m_elementsPerTexture.resize(getWorld()->m_textures.size(), 0);
    size_t totalElements = 0;

    // Gather size information
    for(size_t i = 0; i < model.getMeshReferenceCount(); i++)
    {
        const std::shared_ptr<core::BaseMesh> original = model.getMeshReference(i).mesh_base;

        m_mesh->m_texturePageCount = std::max(m_mesh->m_texturePageCount, original->m_texturePageCount);

        for(size_t j = 0; j < original->m_texturePageCount; j++)
        {
            m_mesh->m_elementsPerTexture[j] += original->m_elementsPerTexture[j];
            totalElements += original->m_elementsPerTexture[j];
        }
    }

    // Create arrays
    m_mesh->m_elements.resize(totalElements, 0);

    // - with matrix index information
    m_mesh->m_matrixIndices.resize(m_mesh->m_vertices.size());

    // Copy information
    std::vector<size_t> elementsStartPerTexture(m_mesh->m_texturePageCount);
    m_mesh->m_vertices.clear();
    for(size_t i = 0; i < model.getMeshReferenceCount(); i++)
    {
        const std::shared_ptr<core::BaseMesh> original = model.getMeshReference(i).mesh_base;

        // Copy vertices
        const size_t verticesStart = m_mesh->m_vertices.size();
        m_mesh->m_vertices.insert(m_mesh->m_vertices.end(), original->m_vertices.begin(), original->m_vertices.end());

        // Copy elements
        size_t originalElementsStart = 0;
        for(size_t page = 0; page < original->m_texturePageCount; page++)
        {
            if(original->m_elementsPerTexture[page] == 0)
                continue;

            BOOST_ASSERT(originalElementsStart < original->m_elements.size());
            BOOST_ASSERT(originalElementsStart + original->m_elementsPerTexture[page] <= original->m_elements.size());

            BOOST_ASSERT(elementsStartPerTexture[page] < m_mesh->m_elements.size());
            BOOST_ASSERT(elementsStartPerTexture[page] + original->m_elementsPerTexture[page] <= m_mesh->m_elements.size());

            std::copy_n(&original->m_elements[originalElementsStart], original->m_elementsPerTexture[page], &m_mesh->m_elements[elementsStartPerTexture[page]]);

            for(size_t j = 0; j < original->m_elementsPerTexture[page]; j++)
            {
                m_mesh->m_elements[elementsStartPerTexture[page]] = static_cast<GLuint>(verticesStart + original->m_elements[originalElementsStart]);
                originalElementsStart += 1;
                elementsStartPerTexture[page] += 1;
            }
        }

        /*
         * Apply total offset from parent.
         * The resulting mesh will have all the hair in default position
         * (i.e. as one big rope). The shader and matrix then transform it
         * correctly.
         */
        m_elements[i].position = model.getMeshReference(i).offset;
        if(i > 0)
        {
            // TODO: This assumes the parent is always the preceding mesh.
            // True for hair, obviously wrong for everything else. Can stay
            // here, but must go when we start generalizing the whole thing.
            m_elements[i].position += m_elements[i - 1].position;
        }

        // And create vertex data (including matrix indices)
        for(size_t j = 0; j < original->m_vertices.size(); j++)
        {
            m_mesh->m_matrixIndices.emplace_back();
            BOOST_ASSERT(m_mesh->m_matrixIndices.size() > verticesStart + j);
            if(original->m_vertices[j].position[1] <= 0)
            {
                m_mesh->m_matrixIndices[verticesStart + j].i = i;
                m_mesh->m_matrixIndices[verticesStart + j].j = i + 1;
            }
            else
            {
                m_mesh->m_matrixIndices[verticesStart + j].i = i + 1;
                m_mesh->m_matrixIndices[verticesStart + j].j = std::min(static_cast<int8_t>(i + 2), static_cast<int8_t>(model.getMeshReferenceCount()));
            }

            // Now move all the hair vertices
            m_mesh->m_vertices[verticesStart + j].position += m_elements[i].position;

            // If the normal isn't fully in y direction, cancel its y component
            // This is perhaps a bit dubious.
            if(m_mesh->m_vertices[verticesStart + j].normal[0] != 0 || m_mesh->m_vertices[verticesStart + j].normal[2] != 0)
            {
                m_mesh->m_vertices[verticesStart + j].normal[1] = 0;
                m_mesh->m_vertices[verticesStart + j].normal = glm::normalize(m_mesh->m_vertices[verticesStart + j].normal);
            }
        }
    }

    m_mesh->genVBO();
}

void HairSetup::getSetup(world::World& world, int hair_entry_index)
{
    lua::Value res = world.m_engine->engine_lua["getHairSetup"](hair_entry_index);
    if(!res.is<lua::Table>())
        return;

    m_model = res["model"].to<ModelId>();
    m_linkBody = res["link_body"].to<animation::BoneId>();
    m_rootWeight = res["props"]["root_weight"].to<glm::float_t>();
    m_tailWeight = res["props"]["tail_weight"].to<glm::float_t>();
    m_hairInertia = res["props"]["hair_inertia"].to<glm::float_t>();
    m_hairFriction = res["props"]["hair_friction"].to<glm::float_t>();
    m_hairRestitution = res["props"]["hair_bouncing"].to<glm::float_t>();
    m_jointOverlap = res["props"]["joint_overlap"].to<glm::float_t>();
    m_jointCfm = res["props"]["joint_cfm"].to<glm::float_t>();
    m_jointErp = res["props"]["joint_erp"].to<glm::float_t>();
    m_hairDamping[0] = res["props"]["hair_damping"][1].to<glm::float_t>();
    m_hairDamping[1] = res["props"]["hair_damping"][2].to<glm::float_t>();
    m_headOffset = { res["offset"][1].toFloat(), res["offset"][2].toFloat(), res["offset"][3].toFloat() };
    m_rootAngle = { res["root_angle"][1].toFloat(), res["root_angle"][2].toFloat(), res["root_angle"][3].toFloat() };
}

Hair::~Hair()
{
    for(auto& joint : m_joints)
    {
        if(joint)
            getWorld()->m_engine->bullet.dynamicsWorld->removeConstraint(joint.get());
    }

    for(auto& element : m_elements)
    {
        if(element.body)
        {
            element.body->setUserPointer(nullptr);
            getWorld()->m_engine->bullet.dynamicsWorld->removeRigidBody(element.body.get());
        }
    }
}
} // namespace world
