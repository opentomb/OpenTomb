#pragma once

#include <SDL2/SDL_rwops.h>
#include <vector>
#include <string>

#include "tr_types.h"
#include "tr_versions.h"

/** \brief A complete TR level.
  *
  * This contains all necessary functions to load a TR level.
  * Some corrections to the data are done, like converting to OpenGLs coordinate system.
  * All indexes are converted, so they can be used directly.
  * Endian conversion is done at the lowest possible layer, most of the time this is in the read_bitxxx functions.
  */
class TR_Level
{
public:
    int32_t m_gameVersion = TR_UNKNOWN;                   ///< \brief game engine version.

    std::vector<tr_textile8_t> m_textile8;                ///< \brief 8-bit 256x256 textiles(TR1-3).
    std::vector<tr2_textile16_t> m_textile16;             ///< \brief 16-bit 256x256 textiles(TR2-5).
    std::vector<tr4_textile32_t> m_textile32;             ///< \brief 32-bit 256x256 textiles(TR4-5).
    std::vector<tr5_room_t> m_rooms;                      ///< \brief all rooms (normal and alternate).
    std::vector<uint16_t> m_floorData;                   ///< \brief the floor data.
    std::vector<tr4_mesh_t> m_meshes;                     ///< \brief all meshes (static and moveables).
    std::vector<uint32_t> m_meshIndices;                 ///< \brief mesh index table.
    std::vector<tr_animation_t> m_animations;             ///< \brief animations for moveables.
    std::vector<tr_state_change_t> m_stateChanges;       ///< \brief state changes for moveables.
    std::vector<tr_anim_dispatch_t> m_animDispatches;    ///< \brief animation dispatches for moveables.
    std::vector<int16_t> m_animCommands;                 ///< \brief animation commands for moveables.
    std::vector<tr_moveable_t> m_moveables;               ///< \brief data for the moveables.
    std::vector<tr_staticmesh_t> m_staticMeshes;         ///< \brief data for the static meshes.
    std::vector<tr4_object_texture_t> m_objectTextures;  ///< \brief object texture definitions.
    std::vector<uint16_t> m_animatedTextures;            ///< \brief animated textures.
    uint32_t m_animatedTexturesUvCount = 0;
    std::vector<tr_sprite_texture_t> m_spriteTextures;   ///< \brief sprite texture definitions.
    std::vector<tr_sprite_sequence_t> m_spriteSequences; ///< \brief sprite sequences for animation.
    std::vector<tr_camera_t> m_cameras;                   ///< \brief cameras.
    std::vector<tr4_flyby_camera_t> m_flybyCameras;      ///< \brief flyby cameras.
    std::vector<tr_sound_source_t> m_soundSources;       ///< \brief sound sources.
    std::vector<tr_box_t> m_boxes;                        ///< \brief boxes.
    std::vector<uint16_t> m_overlaps;                     ///< \brief overlaps.
    std::vector<int16_t> m_zones;                         ///< \brief zones.
    std::vector<tr2_item_t> m_items;                      ///< \brief items.
    tr_lightmap_t m_lightmap;                 ///< \brief ligthmap (TR1-3).
    tr2_palette_t m_palette;                  ///< \brief colour palette (TR1-3).
    tr2_palette_t m_palette16;                ///< \brief colour palette (TR2-3).
    std::vector<tr4_ai_object_t> m_aiObjects;            ///< \brief ai objects (TR4-5).
    std::vector<tr_cinematic_frame_t> m_cinematicFrames; ///< \brief cinematic frames (TR1-3).
    std::vector<uint8_t> m_demoData;                     ///< \brief demo data.
    std::vector<int16_t> m_soundmap;                      ///< \brief soundmap (TR: 256 values TR2-4: 370 values TR5: 450 values).
    std::vector<tr_sound_details_t> m_soundDetails;      ///< \brief sound details.
    uint32_t m_samplesCount = 0;
    std::vector<uint8_t> m_samplesData;                  ///< \brief samples.
    std::vector<uint32_t> m_sampleIndices;               ///< \brief sample indices.

    std::vector<uint16_t> m_frameData;                   ///< \brief frame data array
    std::vector<uint32_t> m_meshTreeData;

    std::string m_sfxPath = "MAIN.SFX";

    void read_level(const std::string &filename, int32_t game_version);
    void read_level(SDL_RWops * const src, int32_t game_version);

protected:
    uint32_t m_numTextiles;          ///< \brief number of 256x256 textiles.
    uint32_t m_numRoomTextiles;     ///< \brief number of 256x256 room textiles (TR4-5).
    uint32_t m_numObjTextiles;      ///< \brief number of 256x256 object textiles (TR4-5).
    uint32_t m_numBumpTextiles;     ///< \brief number of 256x256 bump textiles (TR4-5).
    uint32_t m_numMiscTextiles;     ///< \brief number of 256x256 misc textiles (TR4-5).
    bool m_read32BitTextiles;       ///< \brief are other 32bit textiles than misc ones read?

    int8_t read_bit8(SDL_RWops * const src);
    uint8_t read_bitu8(SDL_RWops * const src);
    int16_t read_bit16(SDL_RWops * const src);
    uint16_t read_bitu16(SDL_RWops * const src);
    int32_t read_bit32(SDL_RWops * const src);
    uint32_t read_bitu32(SDL_RWops * const src);
    float read_float(SDL_RWops * const src);
    float read_mixfloat(SDL_RWops * const src);

    void read_mesh_data(SDL_RWops * const src);
    void read_frame_moveable_data(SDL_RWops * const src);

    void read_tr_colour(SDL_RWops * const src, tr2_colour_t & colour);
    void read_tr_vertex16(SDL_RWops * const src, tr5_vertex_t & vertex);
    void read_tr_vertex32(SDL_RWops * const src, tr5_vertex_t & vertex);
    void read_tr_face3(SDL_RWops * const src, tr4_face3_t & face);
    void read_tr_face4(SDL_RWops * const src, tr4_face4_t & face);
    void read_tr_textile8(SDL_RWops * const src, tr_textile8_t & textile);
    void read_tr_lightmap(SDL_RWops * const src, tr_lightmap_t & lightmap);
    void read_tr_palette(SDL_RWops * const src, tr2_palette_t & palette);
    void read_tr_box(SDL_RWops * const src, tr_box_t & box);
    void read_tr_room_sprite(SDL_RWops * const src, tr_room_Sprite & room_sprite);
    void read_tr_room_portal(SDL_RWops * const src, tr_room_portal_t & portal);
    void read_tr_room_sector(SDL_RWops * const src, tr_room_sector_t & room_sector);
    void read_tr_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert);
    void read_tr_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture);
    void read_tr_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture);
    void read_tr_sprite_sequence(SDL_RWops * const src, tr_sprite_sequence_t & sprite_sequence);
    void read_tr_mesh(SDL_RWops * const src, tr4_mesh_t & mesh);
    void read_tr_state_changes(SDL_RWops * const src, tr_state_change_t & state_change);
    void read_tr_anim_dispatches(SDL_RWops * const src, tr_anim_dispatch_t & anim_dispatch);
    void read_tr_animation(SDL_RWops * const src, tr_animation_t & animation);
    void read_tr_moveable(SDL_RWops * const src, tr_moveable_t & moveable);
    void read_tr_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr_cinematic_frame(SDL_RWops * const src, tr_cinematic_frame_t & cf);
    void read_tr_staticmesh(SDL_RWops * const src, tr_staticmesh_t & mesh);
    void read_tr_level(SDL_RWops * const src, bool demo_or_ub);

    void read_tr2_colour4(SDL_RWops * const src, tr2_colour_t & colour);
    void read_tr2_palette16(SDL_RWops * const src, tr2_palette_t & palette16);
    void read_tr2_textile16(SDL_RWops * const src, tr2_textile16_t & textile);
    void read_tr2_box(SDL_RWops * const src, tr_box_t & box);
    void read_tr2_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr2_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr2_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr2_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr2_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr2_level(SDL_RWops * const src, bool demo);

    void read_tr3_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr3_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr3_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr3_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr3_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr3_level(SDL_RWops * const src);

    void read_tr4_vertex_float(SDL_RWops * const src, tr5_vertex_t & vertex);
    void read_tr4_textile32(SDL_RWops * const src, tr4_textile32_t & textile);
    void read_tr4_face3(SDL_RWops * const src, tr4_face3_t & meshface);
    void read_tr4_face4(SDL_RWops * const src, tr4_face4_t & meshface);
    void read_tr4_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr4_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr4_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr4_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr4_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr4_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert);
    void read_tr4_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture);
    void read_tr4_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture);
    void read_tr4_mesh(SDL_RWops * const src, tr4_mesh_t & mesh);
    void read_tr4_animation(SDL_RWops * const src, tr_animation_t & animation);
    void read_tr4_level(SDL_RWops * const _src);

    void read_tr5_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr5_room_layer(SDL_RWops * const src, tr5_room_layer_t & layer);
    void read_tr5_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & vert);
    void read_tr5_room(SDL_RWops * const orgsrc, tr5_room_t & room);
    void read_tr5_moveable(SDL_RWops * const src, tr_moveable_t & moveable);
    void read_tr5_level(SDL_RWops * const src);
};
