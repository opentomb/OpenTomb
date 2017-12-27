// GLSL vertex program for rendering entities

uniform mat4 modelViewProjection;
uniform mat4 modelView;
uniform float distFog;

varying vec4 varying_color;
varying vec2 varying_texCoord;
varying vec3 varying_normal;
varying vec3 varying_position;

void main()
{
    // Transform model-space position, used for lighting by
    // fragment shader
    vec4 position = modelView * gl_Vertex;
    varying_position = position.xyz / position.w;
    
    // Transform normal; assuming only standard transforms
    // (Otherwise we'd need to have a special normal matrix)
    varying_normal = (modelView * vec4(gl_Normal, 0)).xyz;
    
    // Need projected position for transform
    gl_Position = modelViewProjection * gl_Vertex;

    // Copy attributes to varyings
    varying_texCoord = gl_MultiTexCoord0.xy;
    float dd = length(gl_Position);
    float d = clamp(2.0 * (1.0 - dd / distFog), 0.0, 1.0);
    varying_color = gl_Color * d;
}
