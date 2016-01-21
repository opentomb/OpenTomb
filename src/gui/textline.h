#pragma once

#include "fontmanager.h"
#include "gui.h"

#include <boost/format.hpp>

namespace gui
{

struct TextLine
{
    std::string                 text;

    FontType                    fontType;
    FontStyle                   fontStyle;

    glm::vec2 position;
    mutable glm::vec2 offset;

    HorizontalAnchor            Xanchor;
    VerticalAnchor              Yanchor;

    mutable glm::vec2 topLeft;
    mutable glm::vec2 bottomRight;

    bool                        show;

    void move();
};

class TextLineManager
{
private:
    std::list<const TextLine*> m_baseLines;
    std::list<TextLine> m_tempLines;

public:
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

    static std::unique_ptr<TextLineManager> instance;
};

} // namespace gui
