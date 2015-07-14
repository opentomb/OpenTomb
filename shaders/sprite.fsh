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
    color = texture(color_map, varying_texCoord);
}
