// GLSL vertex programm for color mult
in vec3 position;
in vec4 color;
in vec3 normal;
in vec2 texCoord;

uniform mat4 modelViewProjection;
uniform vec4 tintMult;

out vec4 varying_color;
out vec2 varying_texCoord;

void main(void)
{
    gl_Position = modelViewProjection * vec4(position, 1.0);
    varying_color = color * tintMult;
    varying_texCoord = texCoord;
}