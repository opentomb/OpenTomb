// GLSL fragment program for drawingtext

// Varying attribute for color
in vec4 varying_color;

// Varying attribute for position, used to form tex coordinate
in vec2 varying_texCoord;

// Texture
uniform sampler2D color_map;

// Color output
out vec4 color;

void main(void)
{
    vec4 texColor = texture(color_map, varying_texCoord);
    color = vec4(varying_color.rgb, texColor.r);
}
