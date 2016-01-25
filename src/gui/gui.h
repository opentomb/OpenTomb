#pragma once

#include "gl_font.h"
#include "render/render.h"

#include "fadermanager.h"
#include "inventory.h"
#include "itemnotifier.h"
#include "progressbarmanager.h"

namespace gui
{
struct Gui
{
    // Notifier show time is a time notifier stays on screen (excluding slide
    // effect). Maybe it's better to move it to script later.

    Gui();
    ~Gui();

    ProgressbarManager progressBars;
    FaderManager faders;
    ItemNotifier notifier;

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

    glm::mat4 guiProjectionMatrix = glm::mat4(1.0f);

    static std::unique_ptr<Gui> instance;

private:
    GLuint rectanglePositionBuffer = 0;
    GLuint rectangleColorBuffer = 0;
    std::unique_ptr<render::VertexArray> rectangleArray = nullptr;
};
} // namespace gui
