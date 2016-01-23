#pragma once

#include "progressbar.h"

#include <map>

namespace gui
{
class ProgressbarManager
{
public:
    ProgressbarManager();
    void draw();
    void showLoading(int value);
    void resize();
private:
    std::map<BarType, ProgressBar> m_progressBars;
};
}
