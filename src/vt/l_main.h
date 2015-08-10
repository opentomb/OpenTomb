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
class TR1Level
{
public:
    TR1Level(Game gameVersion, io::SDLReader&& reader)
        : m_gameVersion(gameVersion)
        , m_reader(std::move(reader))
    {
    }

    virtual ~TR1Level() = default;

    const Game m_gameVersion;                   ///< \brief game engine version.

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
    size_t m_samplesCount = 0;
    std::vector<uint8_t> m_samplesData;                  ///< \brief samples.
    std::vector<uint32_t> m_sampleIndices;               ///< \brief sample indices.

    std::vector<uint16_t> m_frameData;                   ///< \brief frame data array
    std::vector<uint32_t> m_meshTreeData;

    std::string m_sfxPath = "MAIN.SFX";

    static std::unique_ptr<TR1Level> createLoader(const std::string &filename, Game game_version);
    virtual void load();

    virtual void prepareLevel();
    void dumpTextures();
    StaticMesh *findStaticMeshById(uint32_t object_id);
    Item *fineItemById(int32_t object_id);
    Moveable *findMoveableById(uint32_t object_id);

protected:
    io::SDLReader m_reader;
    bool m_demoOrUb = false;

    void readMeshData(io::SDLReader& reader);
    void readFrameMoveableData(io::SDLReader& reader);

    static void convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst);
    static void convertTexture(WordTexture & tex, DWordTexture & dst);

private:
    static Game probeVersion(io::SDLReader& reader, const std::string &filename);
    static std::unique_ptr<TR1Level> createLoader(io::SDLReader&& reader, Game game_version, const std::string& sfxPath);
};

class TR2Level : public TR1Level
{
public:
    TR2Level(Game gameVersion, io::SDLReader&& reader)
        : TR1Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
    virtual void prepareLevel() override;
};

class TR3Level : public TR1Level
{
public:
    TR3Level(Game gameVersion, io::SDLReader&& reader)
        : TR1Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
    virtual void prepareLevel() override;
};

class TR4Level : public TR1Level
{
public:
    TR4Level(Game gameVersion, io::SDLReader&& reader)
        : TR1Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
    virtual void prepareLevel() override;
};

class TR5Level : public TR1Level
{
public:
    TR5Level(Game gameVersion, io::SDLReader&& reader)
        : TR1Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
    virtual void prepareLevel() override;
};
}
