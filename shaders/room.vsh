// GLSL vertex program for various room effects (water/flicker) etc. 

uniform	vec4 tintMult;
uniform int iCurrentTick;
uniform int iRoomLightMode;
uniform int iRoomFlags;

void main(void)
{
	//???
	gl_Position = ftransform();
	
	//This is our vertex/vertex color
	vec4 vPos = gl_Vertex;
	vec4 vCol = gl_Color;
	
	float fPerturb = 0.0;
	float fGlow = 0.0;
	float fFlicker = 0.0;
	
	//Calculate sum and time
	float fSum = vPos.x + vPos.y + vPos.z;
	float fTime = float(iCurrentTick) * 0.00157;
	float fFlickerTime = float(iCurrentTick) * 0.80;
	
	//If this room is a water room
	if((iRoomFlags & 0x1) == 1)
	{
		//Calculate peturb and glow values
		fPerturb = 0.5 * abs(sin(fSum * 8.0 + fTime)) + 0.5;
		fGlow = 0.5 * abs(sin(8.0 + fTime)) + 0.5;
		vCol *= tintMult * mix(1.0, fPerturb, 1.0) * mix(1.0, fGlow, 0.8);
	}
	
	//If the room has flickering flags set
	if(iRoomLightMode == 0x1)
	{
		fFlicker = 0.4 * abs(sin(fFlickerTime)) + 0.6;
		vCol *=  mix(1.0, fFlicker, 0.6);
	}
	
	//Set texture co-ord
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	
	//Set color
	gl_FrontColor = vCol;
}