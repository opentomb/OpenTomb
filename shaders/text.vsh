// GLSL vertex program for drawing text

in vec4 color;
in vec2 texCoord;
in vec2 position;

// Transformation data to get to the screen coordinates
uniform vec2 screenSize;

// Varying attribute for color
out vec4 varying_color;
// Varying attribute for texture coordinate
out vec2 varying_texCoord;

void main(void)
{
    varying_color = color;
    varying_texCoord = texCoord;
    gl_Position = vec4(vec2(2) * (position / screenSize) - vec2(1), 0, 1);
}
