// GLSL fragment program for drawing a screen-aligned square

// Varying attribute for color
varying vec4 varying_color;

void main(void)
{
    gl_FragColor = varying_color;
}
