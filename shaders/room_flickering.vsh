// GLSL vertex program for various room effects (water/flicker) etc.

uniform mat4 modelViewProjection;
uniform float fCurrentTick;

varying vec4 varying_color;
varying vec2 varying_texCoord;

void main(void)
{
    //This is our vertex/vertex color
    vec4 vPos = gl_Vertex;
    vec4 vCol = gl_Color;
    
    gl_Position = modelViewProjection * gl_Vertex;
    
    float fPerturb = 0.0;
    float fGlow = 0.0;
    float fFlicker = 0.0;
    
    //Calculate sum and time
    float fSum = vPos.x + vPos.y + vPos.z;
    float fTime = fCurrentTick * 0.00157;
    float fFlickerTime = fCurrentTick * 0.80;
    
    //This room has flickering flags set
    fFlicker = 0.4 * abs(sin(fFlickerTime)) + 0.6;
    vCol *=  mix(1.0, fFlicker, 0.6);
    
    //Set texture co-ord
    varying_texCoord = gl_MultiTexCoord0.xy;
    
    //Set color
    varying_color = vCol;
}
