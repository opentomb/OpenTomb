// GLSL vertex program for various room effects (water/flicker) etc. 

uniform mat4 modelViewProjection;

varying vec4 varying_color;
varying vec2 varying_texCoord;

void main(void)
{
    varying_texCoord = gl_MultiTexCoord0.xy;
    varying_color = gl_Color;
    gl_Position = modelViewProjection * gl_Vertex;
}
