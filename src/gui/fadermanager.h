#pragma once

#include "fader.h"

#include <map>

namespace gui
{
class FaderManager
{
public:
    explicit FaderManager(engine::Engine* engine);
    ~FaderManager();

    void drawFaders();
    void showLoadScreenFader();

    bool start(FaderType fader, FaderDir fade_direction);
    bool stop(FaderType fader);
    bool assignPicture(FaderType fader, const std::string &pic_name);
    FaderStatus getStatus(FaderType fader);
    void setup(FaderType fader,
               uint8_t alpha, uint8_t R, uint8_t G, uint8_t B, loader::BlendingMode blending_mode,
               util::Duration fadein_speed, util::Duration fadeout_speed);
private:
    engine::Engine* m_engine;
    std::map<FaderType, Fader> m_faders;
};
} // namespace gui
