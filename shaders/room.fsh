// GLSL fragment program for drawing a screen-aligned square

// Varying attribute for color
varying vec4 varying_color;

// Varying attribute for tex coord
varying vec2 varying_texCoord;

// Texture
uniform sampler2D color_map;

void main(void)
{
    gl_FragColor = varying_color * texture2D(color_map, varying_texCoord);
}
