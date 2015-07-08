// Vertex shader for sprites

// Per-Vertex inputs
// - Position of sprite
in vec3 position;
// - Tex coord (pass-through)
in vec2 texCoord;
// - Relative position of corner
in vec2 cornerOffset;

// Per-Vertex outputs
// - Tex coord (pass-through
out vec2 varying_texCoord;

// Global parameters
uniform mat4 modelView;
uniform mat4 projection;

void main()
{
    vec4 eyeSpacePosition = modelView * vec4(position, 1);
    vec4 cornerPosition = eyeSpacePosition + vec4(cornerOffset, 0, 0);
    gl_Position = projection * cornerPosition;
    
    varying_texCoord = texCoord;
}
