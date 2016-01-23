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

void World::prepare()
{
    meshes.clear();
    sprites.clear();
    rooms.clear();
    flip_data.clear();
    textures.clear();
    entity_tree.clear();
    items_tree.clear();
    character.reset();

    audioEngine.clear();
    textureAnimations.clear();

    room_boxes.clear();
    cameras_sinks.clear();
    skeletal_models.clear();
    sky_box = nullptr;
    anim_commands.clear();
}


void World::empty()
{
    engine::last_object = nullptr;
    engine_lua.clearTasks();

    audioEngine.deInitAudio(); // De-initialize and destroy all audio objects.

    if(gui::Gui::instance)
    {
        gui::Gui::instance->inventory.setInventory(nullptr);
        gui::Gui::instance->inventory.setItemsType(MenuItemType::Supply);  // see base items
    }

    if(character)
    {
        character->setRoom( nullptr );
        character->m_currentSector = nullptr;
    }

    entity_tree.clear();  // Clearing up entities must happen before destroying rooms.

    // Destroy Bullet's MISC objects (debug spheres etc.)
    ///@FIXME: Hide it somewhere, it is nasty being here.

    if(engine::BulletEngine::instance->dynamicsWorld != nullptr)
    {
        for(int i = engine::BulletEngine::instance->dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = engine::BulletEngine::instance->dynamicsWorld->getCollisionObjectArray()[i];
            if(btRigidBody* body = btRigidBody::upcast(obj))
            {
                Object* object = static_cast<Object*>(body->getUserPointer());
                body->setUserPointer(nullptr);

                if(dynamic_cast<BulletObject*>(object))
                {
                    if(body->getMotionState())
                    {
                        delete body->getMotionState();
                        body->setMotionState(nullptr);
                    }

                    body->setCollisionShape(nullptr);

                    engine::BulletEngine::instance->dynamicsWorld->removeRigidBody(body);
                    delete object;
                    delete body;
                }
            }
        }
    }

    rooms.clear();

    flip_data.clear();
    room_boxes.clear();
    cameras_sinks.clear();
    sprites.clear();
    items_tree.clear();
    character.reset();
    skeletal_models.clear();
    meshes.clear();

    glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
    textures.clear();

    tex_atlas.reset();
    textureAnimations.clear();
}

bool World::deleteEntity(ObjectId id)
{
    if(character->getId() == id)
        return false;

    auto it = entity_tree.find(id);
    if(it == entity_tree.end())
    {
        return false;
    }
    else
    {
        entity_tree.erase(it);
        return true;
    }
}

boost::optional<ObjectId> World::spawnEntity(ModelId model_id, ObjectId room_id, const glm::vec3* pos, const glm::vec3* ang, boost::optional<ObjectId> id)
{
    SkeletalModel* model = getModelByID(model_id);
    if(!model)
        return boost::none;

    std::shared_ptr<Entity> ent;
    if(id)
        ent = getEntityByID(*id);

    if(ent)
    {
        if(pos != nullptr)
        {
            ent->m_transform[3] = glm::vec4(*pos,1.0f);
        }
        if(ang != nullptr)
        {
            ent->m_angles = *ang;
            ent->updateTransform();
        }
        if(room_id < rooms.size())
        {
            ent->setRoom( rooms[room_id].get() );
            ent->m_currentSector = ent->getRoom()->getSectorRaw(glm::vec3(ent->m_transform[3]));
        }
        else
        {
            ent->setRoom( nullptr );
        }

        return ent->getId();
    }

    if(!id)
    {
        ent = std::make_shared<Entity>(next_entity_id);
        entity_tree[next_entity_id] = ent;
        ++next_entity_id;
    }
    else
    {
        ent = std::make_shared<Entity>(*id);
        if(*id + 1 > next_entity_id)
            next_entity_id = *id + 1;
    }

    if(pos != nullptr)
    {
        ent->m_transform[3] = glm::vec4(*pos,1.0f);
    }
    if(ang != nullptr)
    {
        ent->m_angles = *ang;
        ent->updateTransform();
    }
    if(room_id < rooms.size())
    {
        ent->setRoom( rooms[room_id].get() );
        ent->m_currentSector = ent->getRoom()->getSectorRaw(glm::vec3(ent->m_transform[3]));
    }
    else
    {
        ent->setRoom( nullptr );
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

    return ent->getId();
}

std::shared_ptr<Entity> World::getEntityByID(ObjectId id)
{
    if(character->getId() == id)
        return character;

    auto it = entity_tree.find(id);
    if(it == entity_tree.end())
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
    auto it = items_tree.find(id);
    if(it == items_tree.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Room> World::findRoomByPosition(const glm::vec3& pos)
{
    for(auto r : rooms)
    {
        if(r->m_active && r->m_boundingBox.contains(pos))
        {
            return r;
        }
    }
    return nullptr;
}

Room* Room_FindPosCogerrence(const glm::vec3 &new_pos, Room* room)
{
    if(room == nullptr)
    {
        return engine::Engine::instance.m_world.findRoomByPosition(new_pos).get();
    }

    if(room->m_active &&
       new_pos[0] >= room->m_boundingBox.min[0] && new_pos[0] < room->m_boundingBox.max[0] &&
       new_pos[1] >= room->m_boundingBox.min[1] && new_pos[1] < room->m_boundingBox.max[1])
    {
        if(new_pos[2] >= room->m_boundingBox.min[2] && new_pos[2] < room->m_boundingBox.max[2])
        {
            return room;
        }
        else if(new_pos[2] >= room->m_boundingBox.max[2])
        {
            RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_above != nullptr)
            {
                return orig_sector->sector_above->owner_room->checkFlip();
            }
        }
        else if(new_pos[2] < room->m_boundingBox.min[2])
        {
            RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_below != nullptr)
            {
                return orig_sector->sector_below->owner_room->checkFlip();
            }
        }
    }

    RoomSector* new_sector = room->getSectorRaw(new_pos);
    if(new_sector != nullptr && new_sector->portal_to_room)
    {
        return engine::Engine::instance.m_world.rooms[*new_sector->portal_to_room]->checkFlip();
    }

    for(Room* r : room->m_nearRooms)
    {
        if(r->m_active && r->m_boundingBox.contains(new_pos))
        {
            return r;
        }
    }

    return engine::Engine::instance.m_world.findRoomByPosition(new_pos).get();
}

std::shared_ptr<Room> World::getByID(ObjectId ID)
{
    for(auto r : rooms)
    {
        if(ID == r->getId())
        {
            return r;
        }
    }
    return nullptr;
}

RoomSector* Room_GetSectorCheckFlip(std::shared_ptr<Room> room, const glm::vec3& pos)
{
    int x, y;
    RoomSector* ret;

    if(room != nullptr)
    {
        if(!room->m_active)
        {
            if(room->m_baseRoom != nullptr && room->m_baseRoom->m_active)
            {
                room = room->m_baseRoom;
            }
            else if(room->m_alternateRoom != nullptr && room->m_alternateRoom->m_active)
            {
                room = room->m_alternateRoom;
            }
        }
    }
    else
    {
        return nullptr;
    }

    if(!room->m_active)
    {
        return nullptr;
    }

    x = static_cast<int>(pos[0] - room->m_modelMatrix[3][0]) / 1024;
    y = static_cast<int>(pos[1] - room->m_modelMatrix[3][1]) / 1024;
    if(x < 0 || static_cast<size_t>(x) >= room->m_sectors.shape()[0] || y < 0 || static_cast<size_t>(y) >= room->m_sectors.shape()[1])
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    ret = &room->m_sectors[x][y];
    return ret;
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
    if(entity_tree.find(entity->getId()) != entity_tree.end())
        return;

    entity_tree[entity->getId()] = entity;
    if(entity->getId() + 1 > next_entity_id)
        next_entity_id = entity->getId() + 1;
}

bool World::createItem(ModelId item_id, ModelId model_id, ModelId world_model_id, MenuItemType type, uint16_t count, const std::string& name)
{
    SkeletalModel* model = getModelByID(model_id);
    if(!model)
    {
        return false;
    }

    std::unique_ptr<animation::Skeleton> bf(new animation::Skeleton());
    bf->fromModel(model);

    auto item = std::make_shared<BaseItem>();
    item->id = item_id;
    item->world_model_id = world_model_id;
    item->type = type;
    item->count = count;
    item->name[0] = 0;
    strncpy(item->name, name.c_str(), 64);
    item->bf = std::move(bf);

    items_tree[item->id] = item;

    return true;
}

int World::deleteItem(ObjectId item_id)
{
    items_tree.erase(items_tree.find(item_id));
    return 1;
}

SkeletalModel* World::getModelByID(ModelId id)
{
    if(skeletal_models.front().id == id)
    {
        return &skeletal_models.front();
    }
    if(skeletal_models.back().id == id)
    {
        return &skeletal_models.back();
    }

    size_t min = 0;
    size_t max = skeletal_models.size() - 1;
    do
    {
        auto i = (min + max) / 2;
        if(skeletal_models[i].id == id)
        {
            return &skeletal_models[i];
        }

        if(skeletal_models[i].id < id)
            min = i;
        else
            max = i;
    } while(min < max - 1);

    return nullptr;
}

// Find sprite by ID.
// Not a binary search - sprites may be not sorted by ID.

core::Sprite* World::getSpriteByID(core::SpriteId ID)
{
    for(core::Sprite& sp : sprites)
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
    for(animation::TextureAnimationSequence& seq : textureAnimations)
    {
        if(seq.frame_lock)
        {
            continue;
        }

        seq.frameTime += engine::Engine::instance.m_frameTime;
        if(seq.frameTime >= seq.timePerFrame)
        {
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
}

glm::vec4 World::calculateWaterTint() const
{
    if(engineVersion < loader::Engine::TR4)  // If water room and level is TR1-3
    {
        if(engineVersion < loader::Engine::TR3)
        {
            // Placeholder, color very similar to TR1 PSX ver.
            return { 0.585f, 0.9f, 0.9f, 1.0f };
        }
        else
        {
            // TOMB3 - closely matches TOMB3
            return { 0.275f, 0.45f, 0.5f, 1.0f };
        }
    }
    else
    {
        return { 1, 1, 1, 1 };
    }
}

} // namespace world
