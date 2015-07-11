// debuglines.vsh
// Simple shading for the debug lines. No texture, only color

in vec3 position;
in vec3 color;

uniform mat4 modelViewProjection;

out vec3 varying_color;

void main(void)
{
    gl_Position = modelViewProjection * vec4(position, 1.0);
    varying_color = color;
}
