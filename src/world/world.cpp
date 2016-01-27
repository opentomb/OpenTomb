#include "world.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "engine/bullet.h"
#include "engine/engine.h"
#include "entity.h"
#include "gui/console.h"
#include "inventory.h"
#include "resource.h"
#include "script/script.h"
#include "world/character.h"
#include "world/core/sprite.h"
#include "world/room.h"

namespace engine
{
extern world::Object* last_object;
} // namespace engine

using gui::Console;

namespace world
{
World::World(engine::Engine* engine)
    : m_engine(engine)
    , m_audioEngine(engine)
{
    BOOST_LOG_TRIVIAL(info) << "Initializing World";
}

void World::prepare()
{
    m_meshes.clear();
    m_sprites.clear();
    m_rooms.clear();
    m_flipData.clear();
    m_textures.clear();
    m_entities.clear();
    m_items.clear();
    m_character.reset();

    m_audioEngine.clear();
    m_textureAnimations.clear();

    m_roomBoxes.clear();
    m_camerasAndSinks.clear();
    m_skeletalModels.clear();
    m_skyBox = nullptr;
    m_animCommands.clear();
}

void World::empty()
{
    engine::last_object = nullptr;
    m_engine->engine_lua.clearTasks();

    m_audioEngine.deInitAudio(); // De-initialize and destroy all audio objects.

    if(m_character)
    {
        m_character->inventory().disable();
        m_character->inventory().setItemsType(MenuItemType::Supply);  // see base items
    }

    if(m_character)
    {
        m_character->setRoom(nullptr);
        m_character->m_currentSector = nullptr;
    }

    m_entities.clear();  // Clearing up entities must happen before destroying rooms.

    // Destroy Bullet's MISC objects (debug spheres etc.)
    ///@FIXME: Hide it somewhere, it is nasty being here.

    if(m_engine->bullet.dynamicsWorld != nullptr)
    {
        for(int i = m_engine->bullet.dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = m_engine->bullet.dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if(body == nullptr)
                continue;

            Object* object = static_cast<Object*>(body->getUserPointer());
            body->setUserPointer(nullptr);

            if(dynamic_cast<BulletObject*>(object) == nullptr)
                continue;

            if(body->getMotionState())
            {
                delete body->getMotionState();
                body->setMotionState(nullptr);
            }

            body->setCollisionShape(nullptr);

            m_engine->bullet.dynamicsWorld->removeRigidBody(body);
            delete object;
            delete body;
        }
    }

    m_rooms.clear();

    m_flipData.clear();
    m_roomBoxes.clear();
    m_camerasAndSinks.clear();
    m_sprites.clear();
    m_items.clear();
    m_character.reset();
    m_skeletalModels.clear();
    m_meshes.clear();

    glDeleteTextures(static_cast<GLsizei>(m_textures.size()), m_textures.data());
    m_textures.clear();

    m_textureAtlas.reset();
    m_textureAnimations.clear();
}

bool World::deleteEntity(ObjectId id)
{
    if(m_character->getId() == id)
        return false;

    auto it = m_entities.find(id);
    if(it == m_entities.end())
    {
        return false;
    }
    else
    {
        m_entities.erase(it);
        return true;
    }
}

std::shared_ptr<Entity> World::spawnEntity(ModelId model_id, ObjectId room_id, const glm::vec3* pos, const glm::vec3* ang, boost::optional<ObjectId> id)
{
    std::shared_ptr<SkeletalModel> model = getModelByID(model_id);
    if(!model)
        return nullptr;

    std::shared_ptr<Entity> ent;
    if(id)
        ent = getEntityByID(*id);

    if(ent)
    {
        if(pos != nullptr)
        {
            ent->m_transform[3] = glm::vec4(*pos, 1.0f);
        }
        if(ang != nullptr)
        {
            ent->m_angles = *ang;
            ent->updateTransform();
        }
        if(room_id < m_rooms.size())
        {
            ent->setRoom(m_rooms[room_id].get());
            ent->m_currentSector = ent->getRoom()->getSectorRaw(glm::vec3(ent->m_transform[3]));
        }
        else
        {
            ent->setRoom(nullptr);
        }

        return ent;
    }

    if(!id)
    {
        ent = std::make_shared<Entity>(m_nextEntityId, this);
        m_entities[m_nextEntityId] = ent;
        ++m_nextEntityId;
    }
    else
    {
        ent = std::make_shared<Entity>(*id, this);
        if(*id + 1 > m_nextEntityId)
            m_nextEntityId = *id + 1;
    }

    if(pos != nullptr)
    {
        ent->m_transform[3] = glm::vec4(*pos, 1.0f);
    }
    if(ang != nullptr)
    {
        ent->m_angles = *ang;
        ent->updateTransform();
    }
    if(room_id < m_rooms.size())
    {
        ent->setRoom(m_rooms[room_id].get());
        ent->m_currentSector = ent->getRoom()->getSectorRaw(glm::vec3(ent->m_transform[3]));
    }
    else
    {
        ent->setRoom(nullptr);
    }

    ent->m_typeFlags = ENTITY_TYPE_SPAWNED;
    ent->m_active = ent->m_enabled = true;
    ent->m_triggerLayout = 0x00;
    ent->m_OCB = 0x00;
    ent->m_timer = 0.0;

    ent->m_moveType = MoveType::StaticPos;
    ent->m_inertiaLinear = 0.0;
    ent->m_inertiaAngular[0] = 0.0;
    ent->m_inertiaAngular[1] = 0.0;

    ent->m_skeleton.fromModel(model);

    ent->setAnimation(0, 0);                                     // Set zero animation and zero frame

    Res_SetEntityProperties(ent);
    ent->rebuildBoundingBox();
    ent->m_skeleton.genRigidBody(*ent);

    if(ent->getRoom() != nullptr)
    {
        ent->getRoom()->addEntity(ent.get());
    }
    addEntity(ent);
    Res_SetEntityFunction(ent);

    return ent;
}

std::shared_ptr<Entity> World::getEntityByID(ObjectId id) const
{
    if(m_character->getId() == id)
        return m_character;

    auto it = m_entities.find(id);
    if(it == m_entities.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Character> World::getCharacterByID(ObjectId id)
{
    return std::dynamic_pointer_cast<Character>(getEntityByID(id));
}

std::shared_ptr<BaseItem> World::getBaseItemByID(ObjectId id)
{
    auto it = m_items.find(id);
    if(it == m_items.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Room> World::findRoomByPosition(const glm::vec3& pos) const
{
    for(auto r : m_rooms)
    {
        if(r->isActive() && r->getBoundingBox().contains(pos))
        {
            return r;
        }
    }
    return nullptr;
}

Room* World::Room_FindPosCogerrence(const glm::vec3 &new_pos, Room* room) const
{
    if(room == nullptr)
    {
        return findRoomByPosition(new_pos).get();
    }

    if(room->isActive() &&
       new_pos[0] >= room->getBoundingBox().min[0] && new_pos[0] < room->getBoundingBox().max[0] &&
       new_pos[1] >= room->getBoundingBox().min[1] && new_pos[1] < room->getBoundingBox().max[1])
    {
        if(new_pos[2] >= room->getBoundingBox().min[2] && new_pos[2] < room->getBoundingBox().max[2])
        {
            return room;
        }
        else if(new_pos[2] >= room->getBoundingBox().max[2])
        {
            const RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_above != nullptr)
            {
                return orig_sector->sector_above->owner_room->checkFlip();
            }
        }
        else if(new_pos[2] < room->getBoundingBox().min[2])
        {
            const RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_below != nullptr)
            {
                return orig_sector->sector_below->owner_room->checkFlip();
            }
        }
    }

    const RoomSector* new_sector = room->getSectorRaw(new_pos);
    if(new_sector != nullptr && new_sector->portal_to_room)
    {
        return m_rooms[*new_sector->portal_to_room]->checkFlip();
    }

    for(Room* r : room->getNearRooms())
    {
        if(r->isActive() && r->getBoundingBox().contains(new_pos))
        {
            return r;
        }
    }

    return findRoomByPosition(new_pos).get();
}

std::shared_ptr<Room> World::getByID(ObjectId ID)
{
    for(auto r : m_rooms)
    {
        if(ID == r->getId())
        {
            return r;
        }
    }
    return nullptr;
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
    if(m_entities.find(entity->getId()) != m_entities.end())
        return;

    m_entities[entity->getId()] = entity;
    if(entity->getId() + 1 > m_nextEntityId)
        m_nextEntityId = entity->getId() + 1;
}

bool World::createItem(ModelId item_id, ModelId model_id, ModelId world_model_id, MenuItemType type, uint16_t count, const std::string& name)
{
    std::shared_ptr<SkeletalModel> model = getModelByID(model_id);
    if(!model)
    {
        return false;
    }

    std::unique_ptr<animation::Skeleton> bf(new animation::Skeleton(&m_engine->m_world));
    bf->fromModel(model);

    auto item = std::make_shared<BaseItem>();
    item->id = item_id;
    item->world_model_id = world_model_id;
    item->type = type;
    item->count = count;
    item->name[0] = 0;
    strncpy(item->name, name.c_str(), 64);
    item->bf = std::move(bf);

    m_items[item->id] = item;

    return true;
}

int World::deleteItem(ObjectId item_id)
{
    m_items.erase(m_items.find(item_id));
    return 1;
}

std::shared_ptr<SkeletalModel> World::getModelByID(ModelId id)
{
    auto it = m_skeletalModels.find(id);
    if(it == m_skeletalModels.end())
        return nullptr;
    else
        return it->second;
}

// Find sprite by ID.
// Not a binary search - sprites may be not sorted by ID.

core::Sprite* World::getSpriteByID(core::SpriteId ID)
{
    for(core::Sprite& sp : m_sprites)
    {
        if(sp.id == ID)
        {
            return &sp;
        }
    }

    return nullptr;
}

BaseItem::~BaseItem() = default;

void World::updateAnimTextures()                                                // This function is used for updating global animated texture frame
{
    for(animation::TextureAnimationSequence& seq : m_textureAnimations)
    {
        if(seq.frame_lock)
        {
            continue;
        }

        seq.frameTime += m_engine->getFrameTime();
        if(seq.frameTime < seq.timePerFrame)
            continue;

        seq.frameTime -= static_cast<int>(seq.frameTime / seq.timePerFrame) * seq.timePerFrame;

        switch(seq.textureType)
        {
            case animation::TextureAnimationType::Reverse:
                if(seq.reverse)
                {
                    if(seq.currentFrame == 0)
                    {
                        seq.currentFrame++;
                        seq.reverse = false;
                    }
                    else if(seq.currentFrame > 0)
                    {
                        seq.currentFrame--;
                    }
                }
                else
                {
                    if(seq.currentFrame == seq.keyFrames.size() - 1)
                    {
                        seq.currentFrame--;
                        seq.reverse = true;
                    }
                    else if(seq.currentFrame < seq.keyFrames.size() - 1)
                    {
                        seq.currentFrame++;
                    }
                    seq.currentFrame %= seq.keyFrames.size();                ///@PARANOID
                }
                break;

            case animation::TextureAnimationType::Forward:                                    // inversed in polygon anim. texture frames
            case animation::TextureAnimationType::Backward:
                seq.currentFrame++;
                seq.currentFrame %= seq.keyFrames.size();
                break;
        };
    }
}

glm::vec4 World::calculateWaterTint() const
{
    if(m_engineVersion < loader::Engine::TR4)  // If water room and level is TR1-3
    {
        if(m_engineVersion < loader::Engine::TR3)
        {
            // Placeholder, color very similar to TR1 PSX ver.
            return{ 0.585f, 0.9f, 0.9f, 1.0f };
        }
        else
        {
            // TOMB3 - closely matches TOMB3
            return{ 0.275f, 0.45f, 0.5f, 1.0f };
        }
    }
    else
    {
        return{ 1, 1, 1, 1 };
    }
}
} // namespace world
