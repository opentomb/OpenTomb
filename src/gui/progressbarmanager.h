#pragma once

#include "progressbar.h"

namespace gui
{
class ProgressbarManager
{
public:
    ProgressbarManager();
    void draw();
    void showLoading(int value);
    void resize();

    static std::unique_ptr<ProgressbarManager> instance;
private:
    std::map<BarType, ProgressBar> m_progressBars;
};
}
