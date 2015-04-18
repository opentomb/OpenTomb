// GLSL main vertex programm
uniform	mat4       modelViewProjectionMat;
uniform	mat4       modelViewMat;
uniform	mat4       projectionMat;
uniform	mat4       transformMat;

void main(void)
{
    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position = modelViewProjectionMat * transformMat * gl_Vertex;
    //gl_TexCoord[0] = gl_MultiTexCoord0;           // works correct too;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}