#pragma once

#include "fader.h"

namespace gui
{
class FaderManager
{
public:
    FaderManager();
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
    std::map<FaderType, Fader> m_faders;
};
} // namespace gui
