#include "world.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "audio/audio.h"
#include "camera.h"
#include "engine/engine.h"
#include "entity.h"
#include "gui/console.h"
#include "inventory.h"
#include "render/render.h"
#include "resource.h"
#include "script/script.h"
#include "util/vmath.h"
#include "world/character.h"
#include "world/core/basemesh.h"
#include "world/core/mesh.h"
#include "world/core/orientedboundingbox.h"
#include "world/core/polygon.h"
#include "world/portal.h"
#include "world/room.h"
#include "world/staticmesh.h"

namespace engine
{
extern EngineContainer* last_cont;
} // namespace engine

namespace world
{

void World::prepare()
{
    id = 0;
    name = nullptr;
    type = 0x00;
    meshes.clear();
    sprites.clear();
    rooms.clear();
    flip_data.clear();
    textures.clear();
    entity_tree.clear();
    items_tree.clear();
    character.reset();

    audio_sources.clear();
    audio_buffers.clear();
    audio_effects.clear();
    anim_sequences.clear();
    stream_tracks.clear();
    stream_track_map.clear();

    room_boxes.clear();
    cameras_sinks.clear();
    skeletal_models.clear();
    sky_box = nullptr;
    anim_commands.clear();
}


void World::empty()
{
    engine::last_cont = nullptr;
    engine_lua.clearTasks();

    audio::deInit(); // De-initialize and destroy all audio objects.

    if(main_inventory_manager != nullptr)
    {
        main_inventory_manager->setInventory(nullptr);
        main_inventory_manager->setItemsType(MenuItemType::Supply);  // see base items
    }

    if(character)
    {
        character->m_self->room = nullptr;
        character->m_currentSector = nullptr;
    }

    entity_tree.clear();  // Clearing up entities must happen before destroying rooms.

    // Destroy Bullet's MISC objects (debug spheres etc.)
    ///@FIXME: Hide it somewhere, it is nasty being here.

    if(engine::bt_engine_dynamicsWorld != nullptr)
    {
        for(int i = engine::bt_engine_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = engine::bt_engine_dynamicsWorld->getCollisionObjectArray()[i];
            if(btRigidBody* body = btRigidBody::upcast(obj))
            {
                engine::EngineContainer* cont = static_cast<engine::EngineContainer*>(body->getUserPointer());
                body->setUserPointer(nullptr);

                if(cont && (cont->object_type == OBJECT_BULLET_MISC))
                {
                    if(body->getMotionState())
                    {
                        delete body->getMotionState();
                        body->setMotionState(nullptr);
                    }

                    body->setCollisionShape(nullptr);

                    engine::bt_engine_dynamicsWorld->removeRigidBody(body);
                    cont->room = nullptr;
                    delete cont;
                    delete body;
                }
            }
        }
    }

    for(auto room : rooms)
    { room->empty(); }
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
    anim_sequences.clear();
}

bool World::deleteEntity(uint32_t id)
{
    if(character->id() == id) return false;

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

uint32_t World::spawnEntity(uint32_t model_id, uint32_t room_id, const btVector3* pos, const btVector3* ang, int32_t id)
{
    if(SkeletalModel* model = getModelByID(model_id))
    {
        if(std::shared_ptr<Entity> ent = getEntityByID(id))
        {
            if(pos != nullptr)
            {
                ent->m_transform.getOrigin() = *pos;
            }
            if(ang != nullptr)
            {
                ent->m_angles = *ang;
                ent->updateTransform();
            }
            if(room_id < rooms.size())
            {
                ent->m_self->room = rooms[room_id].get();
                ent->m_currentSector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
            }
            else
            {
                ent->m_self->room = nullptr;
            }

            return ent->id();
        }

        std::shared_ptr<Entity> ent;
        if(id < 0)
        {
            ent = std::make_shared<Entity>(next_entity_id);
            entity_tree[next_entity_id] = ent;
            ++next_entity_id;
        }
        else
        {
            ent = std::make_shared<Entity>(id);
            if(static_cast<uint32_t>(id + 1) > next_entity_id)
                next_entity_id = id + 1;
        }

        if(pos != nullptr)
        {
            ent->m_transform.getOrigin() = *pos;
        }
        if(ang != nullptr)
        {
            ent->m_angles = *ang;
            ent->updateTransform();
        }
        if(room_id < rooms.size())
        {
            ent->m_self->room = rooms[room_id].get();
            ent->m_currentSector = ent->m_self->room->getSectorRaw(ent->m_transform.getOrigin());
        }
        else
        {
            ent->m_self->room = nullptr;
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

        ent->m_bf.fromModel(model);

        ent->setAnimation(0, 0);                                     // Set zero animation and zero frame

        Res_SetEntityProperties(ent);
        ent->rebuildBV();
        ent->genRigidBody();

        if(ent->m_self->room != nullptr)
        {
            ent->m_self->room->addEntity(ent.get());
        }
        addEntity(ent);
        Res_SetEntityFunction(ent);

        return ent->id();
    }

    return 0xFFFFFFFF;
}

std::shared_ptr<Entity> World::getEntityByID(uint32_t id)
{
    if(character->id() == id) return character;
    auto it = entity_tree.find(id);
    if(it == entity_tree.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Character> World::getCharacterByID(uint32_t id)
{
    return std::dynamic_pointer_cast<Character>(getEntityByID(id));
}

std::shared_ptr<BaseItem> World::getBaseItemByID(uint32_t id)
{
    auto it = items_tree.find(id);
    if(it == items_tree.end())
        return nullptr;
    else
        return it->second;
}

std::shared_ptr<Room> World::findRoomByPosition(const btVector3& pos)
{
    for(auto r : rooms)
    {
        if(r->active && r->boundingBox.contains(pos))
        {
            return r;
        }
    }
    return nullptr;
}

Room* Room_FindPosCogerrence(const btVector3 &new_pos, Room* room)
{
    if(room == nullptr)
    {
        return engine::engine_world.findRoomByPosition(new_pos).get();
    }

    if(room->active &&
       (new_pos[0] >= room->boundingBox.min[0]) && (new_pos[0] < room->boundingBox.max[0]) &&
       (new_pos[1] >= room->boundingBox.min[1]) && (new_pos[1] < room->boundingBox.max[1]))
    {
        if((new_pos[2] >= room->boundingBox.min[2]) && (new_pos[2] < room->boundingBox.max[2]))
        {
            return room;
        }
        else if(new_pos[2] >= room->boundingBox.max[2])
        {
            RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_above != nullptr)
            {
                return orig_sector->sector_above->owner_room->checkFlip();
            }
        }
        else if(new_pos[2] < room->boundingBox.min[2])
        {
            RoomSector* orig_sector = room->getSectorRaw(new_pos);
            if(orig_sector->sector_below != nullptr)
            {
                return orig_sector->sector_below->owner_room->checkFlip();
            }
        }
    }

    RoomSector* new_sector = room->getSectorRaw(new_pos);
    if((new_sector != nullptr) && (new_sector->portal_to_room >= 0))
    {
        return engine::engine_world.rooms[new_sector->portal_to_room]->checkFlip();
    }

    for(const std::shared_ptr<Room>& r : room->near_room_list)
    {
        if(r->active && r->boundingBox.contains(new_pos))
        {
            return r.get();
        }
    }

    return engine::engine_world.findRoomByPosition(new_pos).get();
}

std::shared_ptr<Room> World::getByID(unsigned int ID)
{
    for(auto r : rooms)
    {
        if(ID == r->id)
        {
            return r;
        }
    }
    return nullptr;
}

RoomSector* Room_GetSectorCheckFlip(std::shared_ptr<Room> room, btScalar pos[3])
{
    int x, y;
    RoomSector* ret;

    if(room != nullptr)
    {
        if(!room->active)
        {
            if((room->base_room != nullptr) && (room->base_room->active))
            {
                room = room->base_room;
            }
            else if((room->alternate_room != nullptr) && (room->alternate_room->active))
            {
                room = room->alternate_room;
            }
        }
    }
    else
    {
        return nullptr;
    }

    if(!room->active)
    {
        return nullptr;
    }

    x = static_cast<int>(pos[0] - room->transform.getOrigin()[0]) / 1024;
    y = static_cast<int>(pos[1] - room->transform.getOrigin()[1]) / 1024;
    if(x < 0 || x >= room->sectors_x || y < 0 || y >= room->sectors_y)
    {
        return nullptr;
    }

    // Column index system
    // X - column number, Y - string number

    ret = &room->sectors[x * room->sectors_y + y];
    return ret;
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
    if(entity_tree.find(entity->id()) != entity_tree.end())
        return;
    entity_tree[entity->id()] = entity;
    if(entity->id() + 1 > next_entity_id)
        next_entity_id = entity->id() + 1;
}

bool World::createItem(uint32_t item_id, uint32_t model_id, uint32_t world_model_id, MenuItemType type, uint16_t count, const std::string& name)
{
    SkeletalModel* model = getModelByID(model_id);
    if(!model)
    {
        return false;
    }

    std::unique_ptr<animation::SSBoneFrame> bf(new animation::SSBoneFrame());
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

int World::deleteItem(uint32_t item_id)
{
    items_tree.erase(items_tree.find(item_id));
    return 1;
}

SkeletalModel* World::getModelByID(uint32_t id)
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

core::Sprite* World::getSpriteByID(unsigned int ID)
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

BaseItem::~BaseItem()
{
    bf->bone_tags.clear();
}

void World::updateAnimTextures()                                                // This function is used for updating global animated texture frame
{
    for(animation::AnimSeq& seq : anim_sequences)
    {
        if(seq.frame_lock)
        {
            continue;
        }

        seq.frame_time += engine::engine_frame_time;
        if(seq.frame_time >= seq.frame_rate)
        {
            int j = static_cast<int>(seq.frame_time / seq.frame_rate);
            seq.frame_time -= static_cast<btScalar>(j) * seq.frame_rate;

            switch(seq.anim_type)
            {
                case animation::AnimTextureType::Reverse:
                    if(seq.reverse_direction)
                    {
                        if(seq.current_frame == 0)
                        {
                            seq.current_frame++;
                            seq.reverse_direction = false;
                        }
                        else if(seq.current_frame > 0)
                        {
                            seq.current_frame--;
                        }
                    }
                    else
                    {
                        if(seq.current_frame == seq.frames.size() - 1)
                        {
                            seq.current_frame--;
                            seq.reverse_direction = true;
                        }
                        else if(seq.current_frame < seq.frames.size() - 1)
                        {
                            seq.current_frame++;
                        }
                        seq.current_frame %= seq.frames.size();                ///@PARANOID
                    }
                    break;

                case animation::AnimTextureType::Forward:                                    // inversed in polygon anim. texture frames
                case animation::AnimTextureType::Backward:
                    seq.current_frame++;
                    seq.current_frame %= seq.frames.size();
                    break;
            };
        }
    }
}

void World::calculateWaterTint(float* tint, bool fixed_colour)
{
    if(engineVersion < loader::Engine::TR4)  // If water room and level is TR1-3
    {
        if(engineVersion < loader::Engine::TR3)
        {
            // Placeholder, color very similar to TR1 PSX ver.
            if(fixed_colour)
            {
                tint[0] = 0.585f;
                tint[1] = 0.9f;
                tint[2] = 0.9f;
                tint[3] = 1.0f;
            }
            else
            {
                tint[0] *= 0.585f;
                tint[1] *= 0.9f;
                tint[2] *= 0.9f;
            }
        }
        else
        {
            // TOMB3 - closely matches TOMB3
            if(fixed_colour)
            {
                tint[0] = 0.275f;
                tint[1] = 0.45f;
                tint[2] = 0.5f;
                tint[3] = 1.0f;
            }
            else
            {
                tint[0] *= 0.275f;
                tint[1] *= 0.45f;
                tint[2] *= 0.5f;
            }
        }
    }
    else
    {
        if(fixed_colour)
        {
            tint[0] = 1.0f;
            tint[1] = 1.0f;
            tint[2] = 1.0f;
            tint[3] = 1.0f;
        }
    }
}

} // namespace world
