// GLSL vertex programm for color mult
uniform mat4 modelViewProjection;
uniform vec4 tintMult;

varying vec4 varying_color;
varying vec2 varying_texCoord;

void main(void)
{
    gl_Position = modelViewProjection * gl_Vertex;
    varying_color = gl_Color * tintMult;
    varying_texCoord = gl_MultiTexCoord0.xy;
}