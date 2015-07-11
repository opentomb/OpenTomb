// GLSL vertex program for rendering skinned meshes in entities

in vec3 position;
in vec4 color;
in vec3 normal;
in vec2 texCoord;
in vec2 matrixIndex; // ivec2 sadly not allowed.

uniform mat4 modelView[10];
uniform mat4 projection;

out vec4 varying_color;
out vec2 varying_texCoord;
out vec3 varying_normal;
out vec3 varying_position;

void main()
{
    // Copy ins to varyings
    varying_texCoord = texCoord;
    varying_color = color;
    
    // Transform model-space position.
    // Interpolate between different skin matrices.
    mat4 matrix = modelView[int(matrixIndex[0])] * 0.5 + modelView[int(matrixIndex[1])] * 0.5;
    vec4 pos = matrix * vec4(position, 1);
    varying_position = pos.xyz / pos.w;
    
    // Transform normal; assuming only standard transforms
    // (Otherwise we'd need to have a special normal matrix)
    varying_normal = (matrix * vec4(normal, 0)).xyz;
    
    // Need projected position for transform
    gl_Position = projection * pos;
}
