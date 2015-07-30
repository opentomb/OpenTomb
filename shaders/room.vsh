// GLSL vertex program for various room effects (water/flicker) etc.

in vec3 position;
in vec4 color;
in vec3 normal;
in vec2 texCoord;

uniform mat4 modelViewProjection;
uniform	vec4 tintMult;
uniform float fCurrentTick;

out vec4 varying_color;
out vec2 varying_texCoord;

void main(void)
{
    //This is our vertex/vertex color
    vec3 vPos = position;
    vec4 vCol = color;

    gl_Position = modelViewProjection * vec4(position, 1.0);

    float fPerturb = 0.0;
    float fGlow = 0.0;
    float fFlicker = 0.0;

#if IS_WATER
    //Calculate sum and time
    float fSum = vPos.x + vPos.y + vPos.z;
    float fTime = fCurrentTick * 0.00157;

    //This room is a water room
    //Calculate peturb and glow values
    fPerturb = 0.5 * abs(sin(fSum * 8.0 + fTime)) + 0.5;
    fGlow = 0.5 * abs(sin(8.0 + fTime)) + 0.5;
    
    vCol *= tintMult * mix(1.0, fPerturb, 1.0) * mix(1.0, fGlow, 0.8);
    vCol.a = 1.0;
#endif

#if IS_FLICKER
    //The room has flickering flags set
    float fFlickerTime = fCurrentTick * 0.80;
    fFlicker = 0.4 * abs(sin(fFlickerTime)) + 0.6;
    vCol *=  mix(1.0, fFlicker, 0.6);
#endif

    //Set texture co-ord
    varying_texCoord = texCoord;

    //Set color
    varying_color = vCol;
}
