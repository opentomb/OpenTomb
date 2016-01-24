#pragma once

#include "level.h"

namespace loader
{
class TR2Level : public Level
{
public:
    TR2Level(Game gameVersion, io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
};
} // namespace loader
