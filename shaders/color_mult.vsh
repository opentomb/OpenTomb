// GLSL vertex programm for color mult
uniform	vec4       tintMult;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    //gl_TexCoord[0] = gl_MultiTexCoord0;           // works correct too;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor[0] = gl_Color[0] * tintMult[0];
    gl_FrontColor[1] = gl_Color[1] * tintMult[1];
    gl_FrontColor[2] = gl_Color[2] * tintMult[2];
    gl_FrontColor[3] = gl_Color[3] * tintMult[3];
}