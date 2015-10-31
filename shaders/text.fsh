// GLSL fragment program for drawingtext

// Varying attribute for color
varying vec4 varying_color;

// Varying attribute for position, used to form tex coordinate
varying vec2 varying_texCoord;

// Texture
uniform sampler2D color_map;
// Color replace coefficient [0..1]
uniform float colorReplace;

void main(void)
{
    vec4 texColor = texture2D(color_map, varying_texCoord);
    texColor.rgb = texColor.rgb * (1 - colorReplace) + vec3(colorReplace, colorReplace, colorReplace);
    gl_FragColor = varying_color * texColor;
}