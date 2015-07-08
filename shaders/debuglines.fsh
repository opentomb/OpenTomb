// debuglines.fsh
// Simple shading for the debug lines. No texture, only color

in vec3 varying_color;

// Color output
out vec4 color;

void main(void)
{
    color = vec4(varying_color, 1);
}
