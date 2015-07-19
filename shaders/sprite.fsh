// Fragment shader for sprites

// Per-pixel inputs
// - texture coordinate
in vec2 varying_texCoord;

// Global parameters
// - Texture
uniform sampler2D color_map;

// Color output
out vec4 color;

void main(void)
{
    vec4 c = texture(color_map, varying_texCoord);
    if(c.a < 0.5)
        discard;
    color = c;
}
