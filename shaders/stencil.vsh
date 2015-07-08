// stencil.vsh
// Does the most simple drawing possible, because all information is discarded
// anyway and only the stencil buffer used.

in vec3 position;

uniform mat4 modelViewProjection;

void main(void)
{
    gl_Position = modelViewProjection * vec4(position, 1.0);
}
