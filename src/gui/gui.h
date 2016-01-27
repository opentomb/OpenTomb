#pragma once

#include "gl_font.h"
#include "render/render.h"

#include "console.h"
#include "fadermanager.h"
#include "itemnotifier.h"
#include "progressbarmanager.h"
#include "textline.h"

namespace gui
{
struct Gui
{
private:
    engine::Engine* m_engine;
    Console m_console;

public:
    explicit Gui(engine::Engine* engine);
    ~Gui();

    ProgressbarManager m_progressBars;
    FaderManager m_faders;
    ItemNotifier m_notifier;
    TextLineManager m_textlineManager;
    FontManager m_fontManager;
    glm::mat4 m_guiProjectionMatrix = glm::mat4(1.0f);

    Console& getConsole()
    {
        return m_console;
    }

    /**
     * Helper method to setup OpenGL state for console drawing.
     *
     * Either changes to 2D matrix state (is_gui = 1) or away from it (is_gui = 0). Does not do any drawing.
     */
    void switchGLMode(bool is_gui);

    /**
     * Draws wireframe of this frustum.
     *
     * Expected state:
     *  - Vertex array is enabled, color, tex coord, normal disabled
     *  - No vertex buffer object is bound
     *  - Texturing is disabled
     *  - Alpha test is disabled
     *  - Blending is enabled
     *  - Lighting is disabled
     * Ignored state:
     *  - Currently bound texture.
     *  - Currently bound element buffer.
     *  - Depth test enabled (disables it, then restores)
     *  - Vertex pointer (changes it)
     *  - Matrices (changes them, restores)
     *  - Line width (changes it, then restores)
     *  - Current color (changes it)
     * Changed state:
     *  - Current position will be arbitrary.
     *  - Vertex pointer will be arbitray.
     *  - Current color will be arbitray (set by console)
     *  - Blend mode will be SRC_ALPHA, ONE_MINUS_SRC_ALPHA (set by console)
     */
    void render();

    /**
     *  Draw simple rectangle.
     *  Only state it changes is the blend mode, according to blendMode value.
     */
    void drawRect(glm::float_t x, glm::float_t y,
                  glm::float_t width, glm::float_t height,
                  const glm::vec4& colorUpperLeft, const glm::vec4& colorUpperRight,
                  const glm::vec4& colorLowerLeft, const glm::vec4& colorLowerRight,
                  const loader::BlendingMode blendMode,
                  const GLuint texture = 0);

    /**
     * General GUI drawing routines.
     */
    void drawLoadScreen(int value);
    void drawInventory();

    /**
     * General GUI update routines.
     */
    bool update();
    void resize();  // Called every resize event.

private:

    GLuint m_rectanglePositionBuffer = 0;
    GLuint m_rectangleColorBuffer = 0;
    std::unique_ptr<render::VertexArray> m_rectangleArray = nullptr;
};
} // namespace gui
