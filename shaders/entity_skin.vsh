// GLSL vertex program for rendering skinned meshes in entities

attribute vec3 position;
attribute vec4 color;
attribute vec3 normal;
attribute vec2 texCoord;
attribute vec2 vertexWeight;

uniform mat4 modelView[2];
uniform mat4 projection;

varying vec4 varying_color;
varying vec2 varying_texCoord;
varying vec3 varying_normal;
varying vec3 varying_position;

void main()
{
    // Copy attributes to varyings
    varying_texCoord = texCoord;
    varying_color = color;
    
    // Transform model-space position.
    // Interpolate between different skin matrices.
    mat4 matrix = vertexWeight.x * modelView[0] + vertexWeight.y * modelView[1];
    vec4 pos = matrix * vec4(position, 1);
    varying_position = pos.xyz / pos.w;
    
    // Transform normal; assuming only standard transforms
    // (Otherwise we'd need to have a special normal matrix)
    varying_normal = (matrix * vec4(normal, 0)).xyz;
    
    // Need projected position for transform
    gl_Position = projection * pos;
}
