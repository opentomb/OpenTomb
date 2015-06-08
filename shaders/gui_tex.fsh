// GLSL fragment program for drawing a screen-aligned square

// Varying attribute for color
varying vec4 varying_color;

// Varying attribute for position, used to form tex coordinate
varying vec2 varying_position;

// Texture
uniform sampler2D color_map;

void main(void)
{
    vec2 coords = varying_position;
    coords.y = 1.0 - coords.y;
    gl_FragColor = varying_color * texture2D(color_map, coords);
}
