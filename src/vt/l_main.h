#pragma once

#include <SDL2/SDL_rwops.h>
#include <vector>
#include <string>
#include <cstring>

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
    ~TR_Level()
    {
        for(size_t i = 0; i < this->rooms.size(); i++)
        {
            if(this->rooms[i].num_layers)
            {
                this->rooms[i].num_layers = 0;
                free(this->rooms[i].layers);
                this->rooms[i].layers = nullptr;
            }

            if(this->rooms[i].num_lights)
            {
                this->rooms[i].num_lights = 0;
                free(this->rooms[i].lights);
                this->rooms[i].lights = nullptr;
            }

            if(this->rooms[i].num_portals)
            {
                this->rooms[i].num_portals = 0;
                free(this->rooms[i].portals);
                this->rooms[i].portals = nullptr;
            }

            if(this->rooms[i].num_xsectors * this->rooms[i].num_zsectors)
            {
                this->rooms[i].num_xsectors = 0;
                this->rooms[i].num_zsectors = 0;
                free(this->rooms[i].sector_list);
                this->rooms[i].sector_list = nullptr;
            }

            if(this->rooms[i].num_sprites)
            {
                this->rooms[i].num_sprites = 0;
                free(this->rooms[i].sprites);
                this->rooms[i].sprites = nullptr;
            }

            if(this->rooms[i].num_static_meshes)
            {
                this->rooms[i].num_static_meshes = 0;
                free(this->rooms[i].static_meshes);
                this->rooms[i].static_meshes = nullptr;
            }

            if(this->rooms[i].num_triangles)
            {
                this->rooms[i].num_triangles = 0;
                free(this->rooms[i].triangles);
                this->rooms[i].triangles = nullptr;
            }

            if(this->rooms[i].num_rectangles)
            {
                this->rooms[i].num_rectangles = 0;
                free(this->rooms[i].rectangles);
                this->rooms[i].rectangles = nullptr;
            }

            if(this->rooms[i].num_vertices)
            {
                this->rooms[i].num_vertices = 0;
                free(this->rooms[i].vertices);
                this->rooms[i].vertices = nullptr;
            }
        }
    }

    int32_t game_version = TR_UNKNOWN;                   ///< \brief game engine version.

    std::vector<tr_textile8_t> textile8;                ///< \brief 8-bit 256x256 textiles(TR1-3).
    std::vector<tr2_textile16_t> textile16;             ///< \brief 16-bit 256x256 textiles(TR2-5).
    std::vector<tr4_textile32_t> textile32;             ///< \brief 32-bit 256x256 textiles(TR4-5).
    std::vector<tr5_room_t> rooms;                      ///< \brief all rooms (normal and alternate).
    std::vector<uint16_t> floor_data;                   ///< \brief the floor data.
    std::vector<tr4_mesh_t> meshes;                     ///< \brief all meshes (static and moveables).
    std::vector<uint32_t> mesh_indices;                 ///< \brief mesh index table.
    std::vector<tr_animation_t> animations;             ///< \brief animations for moveables.
    std::vector<tr_state_change_t> state_changes;       ///< \brief state changes for moveables.
    std::vector<tr_anim_dispatch_t> anim_dispatches;    ///< \brief animation dispatches for moveables.
    std::vector<int16_t> anim_commands;                 ///< \brief animation commands for moveables.
    std::vector<tr_moveable_t> moveables;               ///< \brief data for the moveables.
    std::vector<tr_staticmesh_t> static_meshes;         ///< \brief data for the static meshes.
    std::vector<tr4_object_texture_t> object_textures;  ///< \brief object texture definitions.
    std::vector<uint16_t> animated_textures;            ///< \brief animated textures.
    uint32_t animated_textures_uv_count = 0;
    std::vector<tr_sprite_texture_t> sprite_textures;   ///< \brief sprite texture definitions.
    std::vector<tr_sprite_sequence_t> sprite_sequences; ///< \brief sprite sequences for animation.
    std::vector<tr_camera_t> cameras;                   ///< \brief cameras.
    std::vector<tr4_flyby_camera_t> flyby_cameras;      ///< \brief flyby cameras.
    std::vector<tr_sound_source_t> sound_sources;       ///< \brief sound sources.
    std::vector<tr_box_t> boxes;                        ///< \brief boxes.
    std::vector<uint16_t> overlaps;                     ///< \brief overlaps.
    std::vector<int16_t> zones;                         ///< \brief zones.
    std::vector<tr2_item_t> items;                      ///< \brief items.
    tr_lightmap_t lightmap;                 ///< \brief ligthmap (TR1-3).
    tr2_palette_t palette;                  ///< \brief colour palette (TR1-3).
    tr2_palette_t palette16;                ///< \brief colour palette (TR2-3).
    std::vector<tr4_ai_object_t> ai_objects;            ///< \brief ai objects (TR4-5).
    std::vector<tr_cinematic_frame_t> cinematic_frames; ///< \brief cinematic frames (TR1-3).
    std::vector<uint8_t> demo_data;                     ///< \brief demo data.
    std::vector<int16_t> soundmap;                      ///< \brief soundmap (TR: 256 values TR2-4: 370 values TR5: 450 values).
    std::vector<tr_sound_details_t> sound_details;      ///< \brief sound details.
    uint32_t samples_count = 0;
    std::vector<uint8_t> samples_data;                  ///< \brief samples.
    std::vector<uint32_t> sample_indices;               ///< \brief sample indices.

    std::vector<uint16_t> frame_data;                   ///< \brief frame data array
    std::vector<uint32_t> mesh_tree_data;

    std::string sfx_path = "MAIN.SFX";

    void read_level(const std::string &filename, int32_t game_version);
    void read_level(SDL_RWops * const src, int32_t game_version);

protected:
    uint32_t num_textiles;          ///< \brief number of 256x256 textiles.
    uint32_t num_room_textiles;     ///< \brief number of 256x256 room textiles (TR4-5).
    uint32_t num_obj_textiles;      ///< \brief number of 256x256 object textiles (TR4-5).
    uint32_t num_bump_textiles;     ///< \brief number of 256x256 bump textiles (TR4-5).
    uint32_t num_misc_textiles;     ///< \brief number of 256x256 misc textiles (TR4-5).
    bool read_32bit_textiles;       ///< \brief are other 32bit textiles than misc ones read?

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
