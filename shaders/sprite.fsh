// Fragment shader for sprites

// Per-pixel inputs
// - texture coordinate
varying vec2 varying_texCoord;

// Global parameters
// - Texture
uniform sampler2D color_map;

void main(void)
{
    gl_FragColor = texture2D(color_map, varying_texCoord);
}
