// GLSL vertex program for various room effects (water/flicker) etc.

attribute vec3 vertex;
attribute vec4 color;
attribute vec2 texCoord;

uniform mat4 modelViewProjection;
uniform	vec4 tintMult;
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

	float dd = length(gl_Position);
	
#if IS_WATER
    //Calculate sum and time
    float fSum = vPos.x + vPos.y + vPos.z;
    float fTime = fCurrentTick * 0.00157;

    //This room is a water room
    //Calculate peturb and glow values
    fPerturb = 0.5 * abs(sin(fSum * 8.0 + fTime)) + 0.5;
    fGlow = 0.5 * abs(sin(8.0 + fTime)) + 0.5;
    vCol *= tintMult * mix(1.0, fPerturb, 1.0) * mix(1.0, fGlow, 0.8);
	
	/*
	Tint red. 
	TODO: find nice water colour.
	
	float d = clamp(((8192.0 - dd)/(4096.0)),0.0,1.0);
	vCol = (vCol * vec4(d,d,d,1.0)) + ((1.0 - d) * vec4(1.0,0.0,0.0,1.0));
	*/
#else
	float d = clamp(((32768.0 - dd)/(16384.0)),0.0,1.0);
	vCol *= vec4(d,d,d,1.0);
#endif

#if IS_FLICKER
    //The room has flickering flags set
    float fFlickerTime = fCurrentTick * 0.80;
    fFlicker = 0.4 * abs(sin(fFlickerTime)) + 0.6;
    vCol *=  mix(1.0, fFlicker, 0.6);
#endif

    //Set texture co-ord
    varying_texCoord = gl_MultiTexCoord0.xy;
		
    //Set color
    varying_color = vCol;
}
