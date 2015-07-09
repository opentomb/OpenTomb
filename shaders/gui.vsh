// GLSL vertex program for drawing a screen-aligned square

in vec2 position;
in vec4 color;

// Transformation matrix to get to the screen coordinates
uniform vec2 factor;
// Transformation matrix to get to the screen coordinates
uniform vec2 offset;

// Varying in for position, only used by textured fragment shader
out vec2 varying_position;
// Varying in for color
out vec4 varying_color;

void main(void)
{
    // Uses legacy built-in variables because changing that would require using a VBO.
    varying_position = position;
    varying_color = color;
    gl_Position = vec4(position * factor + offset, 0, 1);
}
