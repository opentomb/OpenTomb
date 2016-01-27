#pragma once

#include "progressbar.h"

#include <map>

namespace world
{
class World;
}

namespace gui
{
class ProgressbarManager
{
public:
    explicit ProgressbarManager(engine::Engine* engine);
    void draw(const world::World& world);
    void showLoading(int value);
    void resize();
private:
    engine::Engine* m_engine;
    std::map<BarType, ProgressBar> m_progressBars;

    ProgressBar& getProgressBar(BarType t)
    {
        auto it = m_progressBars.find(t);
        if(it != m_progressBars.end())
            return it->second;
        return m_progressBars.insert(std::make_pair(t, ProgressBar(m_engine))).first->second;
    }
};
}
