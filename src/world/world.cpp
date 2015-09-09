#include "world.h"

#include <algorithm>
#include <chrono>
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
#include "engine/system.h"
#include "util/vmath.h"
#include "world/character.h"
#include "world/core/basemesh.h"
#include "world/core/orientedboundingbox.h"
#include "world/core/polygon.h"
#include "world/core/sprite.h"
#include "world/portal.h"
#include "world/room.h"
#include "world/staticmesh.h"

namespace engine
{
extern EngineContainer* last_cont;
} // namespace engine

using gui::Console;

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

void World::pauseAllSources()
{
    for(audio::Source& source : audio_sources)
    {
        if(source.isActive())
        {
            source.pause();
        }
    }
}

void World::stopAllSources()
{
    for(audio::Source& source : audio_sources)
    {
        source.stop();
    }
}

void World::resumeAllSources()
{
    for(audio::Source& source : audio_sources)
    {
        if(source.isActive())
        {
            source.play();
        }
    }
}

int World::getFreeSource() const  ///@FIXME: add condition (compare max_dist with new source dist)
{
    for(size_t i = 0; i < audio_sources.size(); i++)
    {
        if(!audio_sources[i].isActive())
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

bool World::endStreams(audio::StreamType stream_type)
{
    bool result = false;

    for(audio::StreamTrack& track : stream_tracks)
    {
        if((stream_type == audio::StreamType::Any) ||                              // End ALL streams at once.
                (track.isPlaying() &&
                 track.isType(stream_type)))
        {
            result = true;
            track.end();
        }
    }

    return result;
}

bool World::stopStreams(audio::StreamType stream_type)
{
    bool result = false;

    for(audio::StreamTrack& track : stream_tracks)
    {
        if(track.isPlaying() &&
                (track.isType(stream_type) ||
                 stream_type == audio::StreamType::Any)) // Stop ALL streams at once.
        {
            result = true;
            track.stop();
        }
    }

    return result;
}

bool World::isTrackPlaying(int32_t track_index) const
{
    for(const audio::StreamTrack& track : stream_tracks)
    {
        if((track_index == -1 || track.isTrack(track_index)) && track.isPlaying())
        {
            return true;
        }
    }

    return false;
}

int World::findSource(int effect_ID, audio::EmitterType entity_type, int entity_ID) const
{
    for(uint32_t i = 0; i < audio_sources.size(); i++)
    {
        if((entity_type == audio::EmitterType::Any || audio_sources[i].m_emitterType == entity_type) &&
                (entity_ID == -1                        || audio_sources[i].m_emitterID == static_cast<int32_t>(entity_ID)) &&
                (effect_ID == -1                        || audio_sources[i].m_effectIndex == static_cast<uint32_t>(effect_ID)))
        {
            if(audio_sources[i].isPlaying())
                return i;
        }
    }

    return -1;
}

int World::getFreeStream() const
{
    for(uint32_t i = 0; i < stream_tracks.size(); i++)
    {
        if(!stream_tracks[i].isPlaying() && !stream_tracks[i].isActive())
        {
            return i;
        }
    }

    return -1;  // If no free source, return error.
}

void World::updateStreams()
{
    audio::updateStreamsDamping();

    for(audio::StreamTrack& track : stream_tracks)
    {
        track.update();
    }
}

bool World::trackAlreadyPlayed(uint32_t track_index, int8_t mask)
{
    if(!mask)
    {
        return false;   // No mask, play in any case.
    }

    if(track_index >= stream_track_map.size())
    {
        return true;    // No such track, hence "already" played.
    }

    mask &= 0x3F;   // Clamp mask just in case.

    if(stream_track_map[track_index] == mask)
    {
        return true;    // Immediately return true, if flags are directly equal.
    }

    int8_t played = stream_track_map[track_index] & mask;
    if(played == mask)
    {
        return true;    // Bits were set, hence already played.
    }

    stream_track_map[track_index] |= mask;
    return false;   // Not yet played, set bits and return false.
}

void World::updateSources()
{
    if(audio_sources.empty())
    {
        return;
    }

    alGetListenerfv(AL_POSITION, listener_position);

    for(uint32_t i = 0; i < audio_emitters.size(); i++)
    {
        send(audio_emitters[i].sound_index, audio::EmitterType::SoundSource, i);
    }

    for(audio::Source& src : audio_sources)
    {
        src.update();
    }
}

void World::updateAudio()
{
    updateSources();
    updateStreams();

    if(audio::audio_settings.listener_is_player)
    {
        audio::updateListenerByEntity(character);
    }
    else
    {
        audio::updateListenerByCamera(render::renderer.camera());
    }
}

audio::StreamError World::streamPlay(const uint32_t track_index, const uint8_t mask)
{
    int    target_stream = -1;
    bool   do_fade_in = false;
    audio::StreamMethod load_method = audio::StreamMethod::Any;
    audio::StreamType stream_type = audio::StreamType::Any;

    char   file_path[256];          // Should be enough, and this is not the full path...

    // Don't even try to do anything with track, if its index is greater than overall amount of
    // soundtracks specified in a stream track map count (which is derived from script).

    if(track_index >= stream_track_map.size())
    {
        Console::instance().warning(SYSWARN_TRACK_OUT_OF_BOUNDS, track_index);
        return audio::StreamError::WrongTrack;
    }

    // Don't play track, if it is already playing.
    // This should become useless option, once proper one-shot trigger functionality is implemented.

    if(isTrackPlaying(track_index))
    {
        Console::instance().warning(SYSWARN_TRACK_ALREADY_PLAYING, track_index);
        return audio::StreamError::Ignored;
    }

    // lua_GetSoundtrack returns stream type, file path and load method in last three
    // provided arguments. That is, after calling this function we receive stream type
    // in "stream_type" argument, file path into "file_path" argument and load method into
    // "load_method" argument. Function itself returns false, if script wasn't found or
    // request was broken; in this case, we quit.

    if(!engine_lua.getSoundtrack(track_index, file_path, &load_method, &stream_type))
    {
        Console::instance().warning(SYSWARN_TRACK_WRONG_INDEX, track_index);
        return audio::StreamError::WrongTrack;
    }

    // Don't try to play track, if it was already played by specified bit mask.
    // Additionally, TrackAlreadyPlayed function applies specified bit mask to track map.
    // Also, bit mask is valid only for non-looped tracks, since looped tracks are played
    // in any way.

    if((stream_type != audio::StreamType::Background) &&
            trackAlreadyPlayed(track_index, mask))
    {
        return audio::StreamError::Ignored;
    }

    // Entry found, now process to actual track loading.

    target_stream = getFreeStream();            // At first, we need to get free stream.

    if(target_stream == -1)
    {
        do_fade_in = stopStreams(stream_type);  // If no free track found, hardly stop all tracks.
        target_stream = getFreeStream();        // Try again to assign free stream.

        if(target_stream == -1)
        {
            Console::instance().warning(SYSWARN_NO_FREE_STREAM);
            return audio::StreamError::NoFreeStream;  // No success, exit and don't play anything.
        }
    }
    else
    {
        do_fade_in = endStreams(stream_type);   // End all streams of this type with fadeout.

        // Additionally check if track type is looped. If it is, force fade in in any case.
        // This is needed to smooth out possible pop with gapless looped track at a start-up.

        do_fade_in = (stream_type == audio::StreamType::Background);
    }

    // Finally - load our track.

    if(!stream_tracks[target_stream].load(file_path, track_index, stream_type, load_method))
    {
        Console::instance().warning(SYSWARN_STREAM_LOAD_ERROR);
        return audio::StreamError::LoadError;
    }

    // Try to play newly assigned and loaded track.

    if(!stream_tracks[target_stream].play(do_fade_in))
    {
        Console::instance().warning(SYSWARN_STREAM_PLAY_ERROR);
        return audio::StreamError::PlayError;
    }

    return audio::StreamError::Processed;   // Everything is OK!
}

bool World::deInitDelay()
{
    const std::chrono::high_resolution_clock::time_point begin_time = std::chrono::high_resolution_clock::now();

    while(isTrackPlaying() || (findSource() >= 0))
    {
        auto curr_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin_time).count() / 1.0e6;

        if(curr_time > audio::AudioDeinitDelay)
        {
            engine::Sys_DebugLog(LOG_FILENAME, "Audio deinit timeout reached! Something is wrong with audio driver.");
            break;
        }
    }

    return true;
}

void World::deInitAudio()
{
    stopAllSources();
    stopStreams();

    deInitDelay();

    audio_sources.clear();
    stream_tracks.clear();
    stream_track_map.clear();

    ///@CRITICAL: You must delete all sources before deleting buffers!

    alDeleteBuffers(static_cast<ALsizei>(audio_buffers.size()), audio_buffers.data());
    audio_buffers.clear();

    audio_effects.clear();
    audio_map.clear();
}

audio::Error World::kill(int effect_ID, audio::EmitterType entity_type, int entity_ID)
{
    int playing_sound = findSource(effect_ID, entity_type, entity_ID);

    if(playing_sound != -1)
    {
        audio_sources[playing_sound].stop();
        return audio::Error::Processed;
    }

    return audio::Error::Ignored;
}

bool World::isInRange(audio::EmitterType entity_type, int entity_ID, float range, float gain)
{
    btVector3 vec{ 0,0,0 };

    switch(entity_type)
    {
    case audio::EmitterType::Entity:
        if(std::shared_ptr<world::Entity> ent = getEntityByID(entity_ID))
        {
            vec = ent->m_transform.getOrigin();
        }
        else
        {
            return false;
        }
        break;

    case audio::EmitterType::SoundSource:
        if(static_cast<uint32_t>(entity_ID) + 1 > audio_emitters.size())
        {
            return false;
        }
        vec = audio_emitters[entity_ID].position;
        break;

    case audio::EmitterType::Global:
        return true;

    default:
        return false;
    }

    auto dist = (listener_position - vec).length2();

    // We add 1/4 of overall distance to fix up some issues with
    // pseudo-looped sounds that are called at certain frames in animations.

    dist /= (gain + 1.25f);

    return dist < range * range;
}

audio::Error World::send(int effect_ID, audio::EmitterType entity_type, int entity_ID)
{
    int32_t         source_number;
    uint16_t        random_value;
    ALfloat         random_float;
    audio::Effect* effect = nullptr;

    // If there are no audio buffers or effect index is wrong, don't process.

    if(audio_buffers.empty() || effect_ID < 0)
        return audio::Error::Ignored;

    // Remap global engine effect ID to local effect ID.

    if(static_cast<uint32_t>(effect_ID) >= audio_map.size())
    {
        return audio::Error::NoSample;  // Sound is out of bounds; stop.
    }

    int real_ID = static_cast<int>(audio_map[effect_ID]);

    // Pre-step 1: if there is no effect associated with this ID, bypass audio send.

    if(real_ID == -1)
    {
        return audio::Error::Ignored;
    }
    else
    {
        effect = &audio_effects[real_ID];
    }

    // Pre-step 2: check if sound non-looped and chance to play isn't zero,
    // then randomly select if it should be played or not.

    if((effect->loop != loader::LoopType::Forward) && (effect->chance > 0))
    {
        random_value = rand() % 0x7FFF;
        if(effect->chance < random_value)
        {
            // Bypass audio send, if chance test is not passed.
            return audio::Error::Ignored;
        }
    }

    // Pre-step 3: Calculate if effect's hearing sphere intersect listener's hearing sphere.
    // If it's not, bypass audio send (cause we don't want it to occupy channel, if it's not
    // heard).

    if(isInRange(entity_type, entity_ID, effect->range, effect->gain) == false)
    {
        return audio::Error::Ignored;
    }

    // Pre-step 4: check if R (Rewind) flag is set for this effect, if so,
    // find any effect with similar ID playing for this entity, and stop it.
    // Otherwise, if W (Wait) or L (Looped) flag is set, and same effect is
    // playing for current entity, don't send it and exit function.

    source_number = findSource(effect_ID, entity_type, entity_ID);

    if(source_number != -1)
    {
        if(effect->loop == loader::LoopType::PingPong)
        {
            audio_sources[source_number].stop();
        }
        else if(effect->loop != loader::LoopType::None) // Any other looping case (Wait / Loop).
        {
            return audio::Error::Ignored;
        }
    }
    else
    {
        source_number = getFreeSource();  // Get free source.
    }

    if(source_number != -1)  // Everything is OK, we're sending audio to channel.
    {
        int buffer_index;

        // Step 1. Assign buffer to source.

        if(effect->sample_count > 1)
        {
            // Select random buffer, if effect info contains more than 1 assigned samples.
            random_value = rand() % (effect->sample_count);
            buffer_index = random_value + effect->sample_index;
        }
        else
        {
            // Just assign buffer to source, if there is only one assigned sample.
            buffer_index = effect->sample_index;
        }

        audio::Source *source = &audio_sources[source_number];

        source->setBuffer(buffer_index);

        // Step 2. Check looped flag, and if so, set source type to looped.

        if(effect->loop == loader::LoopType::Forward)
        {
            source->setLooping(AL_TRUE);
        }
        else
        {
            source->setLooping(AL_FALSE);
        }

        // Step 3. Apply internal sound parameters.

        source->m_emitterID = entity_ID;
        source->m_emitterType = entity_type;
        source->m_effectIndex = effect_ID;

        // Step 4. Apply sound effect properties.

        if(effect->rand_pitch)  // Vary pitch, if flag is set.
        {
            random_float = static_cast<ALfloat>( rand() % effect->rand_pitch_var );
            random_float = effect->pitch + ((random_float - 25.0f) / 200.0f);
            source->setPitch(random_float);
        }
        else
        {
            source->setPitch(effect->pitch);
        }

        if(effect->rand_gain)   // Vary gain, if flag is set.
        {
            random_float = static_cast<ALfloat>( rand() % effect->rand_gain_var );
            random_float = effect->gain + (random_float - 25.0f) / 200.0f;
            source->setGain(random_float);
        }
        else
        {
            source->setGain(effect->gain);
        }

        source->setRange(effect->range);    // Set audible range.

        source->play();                     // Everything is OK, play sound now!

        return audio::Error::Processed;
    }
    else
    {
        return audio::Error::NoChannel;
    }
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
