// GLSL fragment program for drawing a screen-aligned square

// Varying attribute for color
in vec4 varying_color;

// Varying attribute for position, used to form tex coordinate
in vec2 varying_position;

// Texture
uniform sampler2D color_map;

// Color output
out vec4 color;

void main(void)
{
    vec2 coords = varying_position;
    coords.y = 1.0 - coords.y;
    vec4 c = varying_color * texture(color_map, coords);
    if(c.a < 0.5)
        discard;
    color = c;
}
