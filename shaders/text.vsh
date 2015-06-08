// GLSL vertex program for drawing text

// Transformation data to get to the screen coordinates
uniform vec2 screenSize;

// Varying attribute for color
varying vec4 varying_color;
// Varying attribute for texture coordinate
varying vec2 varying_texCoord;

void main(void)
{
    // Uses legacy built-in variables because changing that would require using a VBO.
    varying_color = gl_Color;
    varying_texCoord = gl_MultiTexCoord0.xy;
    gl_Position = vec4(vec2(2) * (gl_Vertex.xy / screenSize) - vec2(1), 0, 1);
}
