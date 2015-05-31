// GLSL fragment program for drawingtext

// Varying attribute for color
varying vec4 varying_color;

// Varying attribute for position, used to form tex coordinate
varying vec2 varying_texCoord;

// Texture
uniform sampler2D color_map;

void main(void)
{
    vec4 texColor = texture2D(color_map, varying_texCoord);
    gl_FragColor = vec4(varying_color.rgb, texColor.a);
}