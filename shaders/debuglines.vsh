// debuglines.vsh
// Simple shading for the debug lines. No texture, only color

attribute vec3 position;
attribute vec3 color;

uniform mat4 modelViewProjection;

varying vec3 varying_color;

void main(void)
{
    gl_Position = modelViewProjection * vec4(position, 1.0);
    varying_color = color;
}
