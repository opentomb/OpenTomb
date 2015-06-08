// GLSL vertex program for drawing a screen-aligned square

// Transformation matrix to get to the screen coordinates
uniform vec2 factor;
// Transformation matrix to get to the screen coordinates
uniform vec2 offset;

// Varying attribute for position, only used by textured fragment shader
varying vec2 varying_position;
// Varying attribute for color
varying vec4 varying_color;

void main(void)
{
    // Uses legacy built-in variables because changing that would require using a VBO.
    varying_position = gl_Vertex.xy;
    varying_color = gl_Color;
    gl_Position = vec4(gl_Vertex.xy * factor + offset, 0, 1);
}
