#pragma once

#include "level.h"

namespace loader
{

class TR1Level : public Level
{
public:
    TR1Level(Game gameVersion, io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
};

} // namespace loader
