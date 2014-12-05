// GLSL vertex programm for color mult
uniform	vec4       tintMult;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    //gl_TexCoord[0] = gl_MultiTexCoord0;           // works correct too;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color * tintMult;
}