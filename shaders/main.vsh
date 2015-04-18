// GLSL main vertex programm

attribute vec4 vPosition;

uniform vec4       uTintMult;
//uniform mat4       uViewProjMat;
uniform mat4       uViewMat;
uniform mat4       uProjectionMat;
uniform mat4       uTransformMat;

void main(void)
{
    // gl_Position = uProjectionMat * uViewMat * uTransformMat * gl_Vertex;
    gl_Position = uProjectionMat * uViewMat * uTransformMat * vPosition;
    gl_TexCoord[0] = gl_MultiTexCoord0;           // works correct too;
    //gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_FrontColor = gl_Color * uTintMult;
}