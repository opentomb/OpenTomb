// GLSL vertex programm for color mult
uniform mat4 modelViewProjection;
uniform vec4 tintMult;
uniform float distFog;

varying vec4 varying_color;
varying vec2 varying_texCoord;

void main(void)
{
    gl_Position = modelViewProjection * gl_Vertex;
    float dd = length(gl_Position);
    float d = clamp((distFog - dd) / (distFog * 0.4), 0.0, 1.0);
    varying_color = gl_Color * tintMult * d;
    varying_texCoord = gl_MultiTexCoord0.xy;
}
