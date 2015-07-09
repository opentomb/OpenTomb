// GLSL vertex program for rendering entities

in vec3 position;
in vec4 color;
in vec3 normal;
in vec2 texCoord;

uniform mat4 modelViewProjection;
uniform mat4 modelView;

out vec4 varying_color;
out vec2 varying_texCoord;
out vec3 varying_normal;
out vec3 varying_position;

void main()
{
    // Copy ins to varyings
    varying_texCoord = texCoord;
    varying_color = color;
    
    // Transform model-space position, used for lighting by
    // fragment shader
    vec4 pos = modelView * vec4(position, 1);
    varying_position = pos.xyz / pos.w;
    
    // Transform normal; assuming only standard transforms
    // (Otherwise we'd need to have a special normal matrix)
    varying_normal = (modelView * vec4(normal, 0)).xyz;
    
    // Need projected position for transform
    gl_Position = modelViewProjection * vec4(position, 1);
}
