// GLSL vertex program for rendering entities

uniform mat4 modelViewProjection;
uniform mat4 modelView;

varying vec4 varying_color;
varying vec2 varying_texCoord;
varying vec3 varying_normal;
varying vec3 varying_position;

void main()
{
    // Copy attributes to varyings
    varying_texCoord = gl_MultiTexCoord0.xy;
    varying_color = gl_Color;
    
    // Transform model-space position, used for lighting by
    // fragment shader
    vec4 position = modelView * gl_Vertex;
    varying_position = position.xyz / position.w;
    
    // Transform normal; assuming only standard transforms
    // (Otherwise we'd need to have a special normal matrix)
    varying_normal = (modelView * vec4(gl_Normal, 0)).xyz;
    
    // Need projected position for transform
    gl_Position = modelViewProjection * gl_Vertex;
}
