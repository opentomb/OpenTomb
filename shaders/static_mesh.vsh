// GLSL vertex programm for color mult
attribute vec3 position;
attribute vec4 color;
attribute vec3 normal;
attribute vec2 texCoord;

uniform mat4 modelViewProjection;
uniform vec4 tintMult;

varying vec4 varying_color;
varying vec2 varying_texCoord;

void main(void)
{
    gl_Position = modelViewProjection * vec4(position, 1.0);
    varying_color = color * tintMult;
    varying_texCoord = texCoord;
}