// GLSL fragment program for drawing a screen-aligned square

// Varying attribute for color
in vec4 varying_color;

// Color output
out vec4 color;

void main(void)
{
    color = varying_color;
}
