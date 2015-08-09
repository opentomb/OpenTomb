#pragma once

#include <SDL2/SDL_rwops.h>
#include <vector>
#include <string>
#include <memory>

#include "tr_types.h"
#include "tr_versions.h"

namespace loader
{
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
    TR_Level(int32_t gameVersion, SDL_RWops* rwOps)
        : m_gameVersion(gameVersion)
        , m_src(rwOps)
    {
    }

    virtual ~TR_Level()
    {
    }

    int32_t m_gameVersion = TR_UNKNOWN;                   ///< \brief game engine version.

    std::vector<ByteTexture> m_textile8;                ///< \brief 8-bit 256x256 textiles(TR1-3).
    std::vector<WordTexture> m_textile16;             ///< \brief 16-bit 256x256 textiles(TR2-5).
    std::vector<DWordTexture> m_textile32;             ///< \brief 32-bit 256x256 textiles(TR4-5).
    std::vector<Room> m_rooms;                      ///< \brief all rooms (normal and alternate).
    std::vector<uint16_t> m_floorData;                   ///< \brief the floor data.
    std::vector<Mesh> m_meshes;                     ///< \brief all meshes (static and moveables).
    std::vector<uint32_t> m_meshIndices;                 ///< \brief mesh index table.
    std::vector<Animation> m_animations;             ///< \brief animations for moveables.
    std::vector<StateChange> m_stateChanges;       ///< \brief state changes for moveables.
    std::vector<AnimDispatch> m_animDispatches;    ///< \brief animation dispatches for moveables.
    std::vector<int16_t> m_animCommands;                 ///< \brief animation commands for moveables.
    std::vector<Moveable> m_moveables;               ///< \brief data for the moveables.
    std::vector<StaticMesh> m_staticMeshes;         ///< \brief data for the static meshes.
    std::vector<ObjectTexture> m_objectTextures;  ///< \brief object texture definitions.
    std::vector<uint16_t> m_animatedTextures;            ///< \brief animated textures.
    uint32_t m_animatedTexturesUvCount = 0;
    std::vector<SpriteTexture> m_spriteTextures;   ///< \brief sprite texture definitions.
    std::vector<SpriteSequence> m_spriteSequences; ///< \brief sprite sequences for animation.
    std::vector<Camera> m_cameras;                   ///< \brief cameras.
    std::vector<FlybyCamera> m_flybyCameras;      ///< \brief flyby cameras.
    std::vector<SoundSource> m_soundSources;       ///< \brief sound sources.
    std::vector<Box> m_boxes;                        ///< \brief boxes.
    std::vector<uint16_t> m_overlaps;                     ///< \brief overlaps.
    std::vector<int16_t> m_zones;                         ///< \brief zones.
    std::vector<Item> m_items;                      ///< \brief items.
    LightMap m_lightmap;                 ///< \brief ligthmap (TR1-3).
    Palette m_palette;                  ///< \brief colour palette (TR1-3).
    Palette m_palette16;                ///< \brief colour palette (TR2-3).
    std::vector<AIObject> m_aiObjects;            ///< \brief ai objects (TR4-5).
    std::vector<CinematicFrame> m_cinematicFrames; ///< \brief cinematic frames (TR1-3).
    std::vector<uint8_t> m_demoData;                     ///< \brief demo data.
    std::vector<int16_t> m_soundmap;                      ///< \brief soundmap (TR: 256 values TR2-4: 370 values TR5: 450 values).
    std::vector<SoundDetails> m_soundDetails;      ///< \brief sound details.
    uint32_t m_samplesCount = 0;
    std::vector<uint8_t> m_samplesData;                  ///< \brief samples.
    std::vector<uint32_t> m_sampleIndices;               ///< \brief sample indices.

    std::vector<uint16_t> m_frameData;                   ///< \brief frame data array
    std::vector<uint32_t> m_meshTreeData;

    std::string m_sfxPath = "MAIN.SFX";

    static std::unique_ptr<TR_Level> createLoader(const std::string &filename, int32_t game_version);
    static std::unique_ptr<TR_Level> createLoader(SDL_RWops * const src, int32_t game_version, const std::string& sfxPath);
    virtual void load();

    void prepare_level();
    void dump_textures();
    StaticMesh *find_staticmesh_id(uint32_t object_id);
    Item *find_item_id(int32_t object_id);
    Moveable *find_moveable_id(uint32_t object_id);

protected:
    uint32_t m_numTextiles;          ///< \brief number of 256x256 textiles.
    uint32_t m_numRoomTextiles;     ///< \brief number of 256x256 room textiles (TR4-5).
    uint32_t m_numObjTextiles;      ///< \brief number of 256x256 object textiles (TR4-5).
    uint32_t m_numBumpTextiles;     ///< \brief number of 256x256 bump textiles (TR4-5).
    uint32_t m_numMiscTextiles;     ///< \brief number of 256x256 misc textiles (TR4-5).
    bool m_read32BitTextiles;       ///< \brief are other 32bit textiles than misc ones read?

    io::SDLReader m_src;
    bool m_demoOrUb = false;

    void read_mesh_data(io::SDLReader& reader);
    void read_frame_moveable_data(io::SDLReader& reader);

    void convert_textile8_to_textile32(ByteTexture & tex, Palette & pal, DWordTexture & dst);
    void convert_textile16_to_textile32(WordTexture & tex, DWordTexture & dst);
};

class TR_TR2Level : public TR_Level
{
public:
    TR_TR2Level(int32_t gameVersion, SDL_RWops* rwOps)
        : TR_Level(gameVersion, rwOps)
    {
    }

    void load() override;
};

class TR_TR3Level : public TR_Level
{
public:
    TR_TR3Level(int32_t gameVersion, SDL_RWops* rwOps)
        : TR_Level(gameVersion, rwOps)
    {
    }

    void load() override;
};

class TR_TR4Level : public TR_Level
{
public:
    TR_TR4Level(int32_t gameVersion, SDL_RWops* rwOps)
        : TR_Level(gameVersion, rwOps)
    {
    }

    void load() override;
};

class TR_TR5Level : public TR_Level
{
public:
    TR_TR5Level(int32_t gameVersion, SDL_RWops* rwOps)
        : TR_Level(gameVersion, rwOps)
    {
    }

    void load() override;
};
}
