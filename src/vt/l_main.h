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
    TR_Level()
    {
        this->game_version = TR_UNKNOWN;
        strncpy(this->sfx_path, "MAIN.SFX", 256);

        this->textile8_count = 0;
        this->textile16_count = 0;
        this->textile8 = nullptr;
        this->textile16 = nullptr;
        this->textile32.clear();

        this->floor_data.clear();
        this->mesh_indices.clear();

        this->animations.clear();
        this->state_changes.clear();
        this->anim_dispatches.clear();
        this->anim_commands.clear();

        this->moveables.clear();
        this->static_meshes.clear();
        this->object_textures.clear();
        this->animated_textures_uv_count = 0; // destroyed
        this->animated_textures.clear();
        this->sprite_textures.clear();
        this->sprite_sequences.clear();
        this->cameras.clear();
        this->flyby_cameras.clear();
        this->sound_sources.clear();

        this->boxes.clear();
        this->overlaps.clear();
        this->zones.clear();
        this->items.clear();
        this->ai_objects.clear();
        this->cinematic_frames.clear();

        this->demo_data.clear();
        this->soundmap.clear();
        this->sound_details.clear();
        this->sample_indices.clear();
        this->samples_count = 0;            // destroyed
        this->samples_data.clear();               // destroyed

        this->frame_data_size = 0;          // destroyed
        this->frame_data = nullptr;            // destroyed
        this->mesh_tree_data_size = 0;      // destroyed
        this->mesh_tree_data = nullptr;        // destroyed

        this->meshes.clear();
        this->rooms.clear();
    }

    ~TR_Level()
    {
        uint32_t i;

        /**destroy all textiles**/
        if(this->textile8_count)
        {
            this->textile8_count = 0;
            free(this->textile8);
            this->textile8 = nullptr;
        }

        if(this->textile16_count)
        {
            this->textile16_count = 0;
            free(this->textile16);
            this->textile16 = nullptr;
        }

        this->textile32.clear();

        /**destroy other data**/
        this->floor_data.clear();

        this->mesh_indices.clear();

        this->animations.clear();

        this->state_changes.clear();

        this->anim_dispatches.clear();

        this->anim_commands.clear();

        this->moveables.clear();

        this->static_meshes.clear();

        this->object_textures.clear();

        this->animated_textures.clear();

        this->sprite_textures.clear();

        this->sprite_sequences.clear();

        this->cameras.clear();

        this->flyby_cameras.clear();

        this->sound_sources.clear();

        this->boxes.clear();

        this->overlaps.clear();

        this->zones.clear();

        this->items.clear();

        this->ai_objects.clear();

        this->cinematic_frames.clear();

        this->demo_data.clear();

        this->soundmap.clear();

        this->sound_details.clear();

        if(!this->samples_data.empty())
        {
            this->samples_count = 0;
            this->samples_data.clear();
        }

        this->sample_indices.clear();

        if(this->frame_data_size)
        {
            this->frame_data_size = 0;
            free(this->frame_data);
            this->frame_data = nullptr;
        }

        if(this->mesh_tree_data_size)
        {
            this->mesh_tree_data_size = 0;
            free(this->mesh_tree_data);
            this->mesh_tree_data = nullptr;
        }

        if(!this->meshes.empty())
        {
            for(i = 0; i < this->meshes.size(); i++)
            {
                if(this->meshes[i].lights)
                {
                    free(this->meshes[i].lights);
                    this->meshes[i].lights = nullptr;
                }

                if(this->meshes[i].num_textured_triangles)
                {
                    free(this->meshes[i].textured_triangles);
                    this->meshes[i].textured_triangles = nullptr;
                    this->meshes[i].num_textured_triangles = 0;
                }

                if(this->meshes[i].num_textured_rectangles)
                {
                    free(this->meshes[i].textured_rectangles);
                    this->meshes[i].textured_rectangles = nullptr;
                    this->meshes[i].num_textured_rectangles = 0;
                }

                if(this->meshes[i].num_coloured_triangles)
                {
                    free(this->meshes[i].coloured_triangles);
                    this->meshes[i].coloured_triangles = nullptr;
                    this->meshes[i].num_coloured_triangles = 0;
                }

                if(this->meshes[i].num_coloured_rectangles)
                {
                    free(this->meshes[i].coloured_rectangles);
                    this->meshes[i].coloured_rectangles = nullptr;
                    this->meshes[i].num_coloured_rectangles = 0;
                }

                if(this->meshes[i].normals)
                {
                    free(this->meshes[i].normals);
                    this->meshes[i].normals = nullptr;
                }

                if(this->meshes[i].vertices)
                {
                    free(this->meshes[i].vertices);
                    this->meshes[i].vertices = nullptr;
                }
            }
            this->meshes.clear();
        }

        if(!this->rooms.empty())
        {
            for(i = 0; i < this->rooms.size(); i++)
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
            this->rooms.clear();
        }
    }

    int32_t game_version;                   ///< \brief game engine version.

    uint32_t textile8_count;
    uint32_t textile16_count;
    tr_textile8_t *textile8 = nullptr;                ///< \brief 8-bit 256x256 textiles(TR1-3).
    tr2_textile16_t *textile16 = nullptr;             ///< \brief 16-bit 256x256 textiles(TR2-5).
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
    uint32_t animated_textures_uv_count;
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
    uint32_t samples_count;
    std::vector<uint8_t> samples_data;                  ///< \brief samples.
    std::vector<uint32_t> sample_indices;               ///< \brief sample indices.

    uint32_t frame_data_size;               ///< \brief frame data array size
    uint16_t *frame_data = nullptr;                   ///< \brief frame data array
    uint32_t mesh_tree_data_size;
    uint32_t *mesh_tree_data = nullptr;

    char     sfx_path[256];

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
