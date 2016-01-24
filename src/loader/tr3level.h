#pragma once

#include "level.h"

namespace loader
{
class TR3Level : public Level
{
public:
    TR3Level(Game gameVersion, io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
};
} // namespace loader
