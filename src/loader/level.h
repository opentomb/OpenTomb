#pragma once

#include "datatypes.h"
#include "game.h"

#include <memory>
#include <vector>

namespace loader
{
/** \brief A complete TR level.
  *
  * This contains all necessary functions to load a TR level.
  * Some corrections to the data are done, like converting to OpenGLs coordinate system.
  * All indexes are converted, so they can be used directly.
  * Endian conversion is done at the lowest possible layer, most of the time this is in the read_bitxxx functions.
  */
class Level
{
public:
    Level(Game gameVersion, io::SDLReader&& reader)
        : m_gameVersion(gameVersion)
        , m_reader(std::move(reader))
    {
    }

    virtual ~Level() = default;

    const Game m_gameVersion;

    std::vector<DWordTexture> m_textures;
    std::unique_ptr<Palette> m_palette;
    std::vector<Room> m_rooms;
    FloorData m_floorData;
    std::vector<Mesh> m_meshes;
    std::vector<uint32_t> m_meshIndices;
    std::vector<Animation> m_animations;
    std::vector<StateChange> m_stateChanges;
    std::vector<AnimDispatch> m_animDispatches;
    std::vector<int16_t> m_animCommands;
    std::vector<std::unique_ptr<AnimatedModel>> m_animatedModels;
    std::vector<StaticMesh> m_staticMeshes;
    std::vector<ObjectTexture> m_objectTextures;
    std::vector<uint16_t> m_animatedTextures;
    size_t m_animatedTexturesUvCount = 0;
    std::vector<SpriteTexture> m_spriteTextures;
    std::vector<SpriteSequence> m_spriteSequences;
    std::vector<Camera> m_cameras;
    std::vector<FlybyCamera> m_flybyCameras;
    std::vector<SoundSource> m_soundSources;
    std::vector<Box> m_boxes;
    std::vector<uint16_t> m_overlaps;
    std::vector<Zone> m_zones;
    std::vector<Item> m_items;
    std::unique_ptr<LightMap> m_lightmap;
    std::vector<AIObject> m_aiObjects;
    std::vector<CinematicFrame> m_cinematicFrames;
    std::vector<uint8_t> m_demoData;
    std::vector<int16_t> m_soundmap;
    std::vector<SoundDetails> m_soundDetails;
    size_t m_samplesCount = 0;
    std::vector<uint8_t> m_samplesData;
    std::vector<uint32_t> m_sampleIndices;

    std::vector<int16_t> m_poseData;
    std::vector<int32_t> m_meshTreeData;

    std::string m_sfxPath = "MAIN.SFX";

    /*
     * 0 Normal
     * 3 Catsuit
     * 4 Divesuit
     * 6 Invisible
     */
    uint16_t m_laraType = 0;

    /*
     * 0 No weather
     * 1 Rain
     * 2 Snow (in title.trc these are red triangles falling from the sky).
     */
    uint16_t m_weatherType = 0;

    static std::unique_ptr<Level> createLoader(const std::string &filename, Game game_version);
    virtual void load() = 0;

    StaticMesh *findStaticMeshById(uint32_t object_id);
    Item *fineItemById(int32_t object_id);
    AnimatedModel *findMoveableById(uint32_t object_id);

protected:
    io::SDLReader m_reader;
    bool m_demoOrUb = false;

    void readMeshData(io::SDLReader& reader);
    void readFrameMoveableData(io::SDLReader& reader);

    static void convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst);
    static void convertTexture(WordTexture & tex, DWordTexture & dst);

private:
    static Game probeVersion(io::SDLReader& reader, const std::string &filename);
    static std::unique_ptr<Level> createLoader(io::SDLReader&& reader, Game game_version, const std::string& sfxPath);
};
}
