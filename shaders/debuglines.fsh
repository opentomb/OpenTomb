// debuglines.fsh
// Simple shading for the debug lines. No texture, only color

varying vec3 varying_color;

void main(void)
{
    gl_FragColor = vec4(varying_color, 1);
}
