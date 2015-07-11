// stencil.fsh
// Does the most simple drawing possible, because all information is discarded
// anyway and only the stencil buffer used.

// Color output TODO: Can we remove this for no color drawing?
out vec4 color;

void main(void)
{
    color = vec4(1);
}
