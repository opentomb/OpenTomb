// static_mesh.fsh

// Varying attribute for color
in vec4 varying_color;

// Varying attribute for tex coord
in vec2 varying_texCoord;

// Texture
uniform sampler2D color_map;

// Color output
out vec4 color;

void main(void)
{
    vec4 c = varying_color * texture(color_map, varying_texCoord);
    if( c.a < 0.5 )
        discard;
    color = c;
}
