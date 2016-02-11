#pragma once

#include "fontmanager.h"
#include "gui/common.h"
#include "util/helpers.h"

#include <boost/format.hpp>

namespace gui
{
struct TextLine
{
    std::string text;

    FontType  fontType = gui::FontType::Primary;
    FontStyle fontStyle = gui::FontStyle::MenuTitle;

    glm::vec2 position = { 10.0, 10.0 };
    mutable glm::vec2 offset;

    HorizontalAnchor Xanchor = gui::HorizontalAnchor::Right;
    VerticalAnchor   Yanchor = gui::VerticalAnchor::Bottom;

    mutable glm::vec2 topLeft;
    mutable glm::vec2 bottomRight;

    bool show = true;

    void move(float scaleFactor);
};

class TextLineManager
{
    TRACK_LIFETIME();

    engine::Engine* m_engine;
    std::list<const TextLine*> m_baseLines;
    std::list<TextLine> m_tempLines;

public:
    explicit TextLineManager(engine::Engine* engine);

    void add(const TextLine *line)
    {
        m_baseLines.push_back(line);
    }

    void erase(const TextLine* line)
    {
        m_baseLines.erase(std::find(m_baseLines.begin(), m_baseLines.end(), line));
    }

    void renderLine(const TextLine& line);
    void renderStrings();

    /**
     * Draws text using a FontType::Secondary.
     */
    TextLine* drawText(glm::float_t x, glm::float_t y, const std::string& str);

    TextLine* drawText(glm::float_t x, glm::float_t y, const boost::format& str)
    {
        return drawText(x, y, str.str());
    }

    void resizeTextLines();
};
} // namespace gui
