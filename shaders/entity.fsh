// GLSL fragment program for rendering entities
// Must be created with a define for NUMBER_OF_LIGHTS first.

uniform sampler2D color_map;

#if NUMBER_OF_LIGHTS > 0
uniform vec3 light_position[NUMBER_OF_LIGHTS];
uniform vec4 light_color[NUMBER_OF_LIGHTS];
uniform float light_falloff[NUMBER_OF_LIGHTS];
#endif
uniform vec4 light_ambient;

varying vec4 varying_color;
varying vec2 varying_texCoord;
varying vec3 varying_normal;
varying vec3 varying_position;

void main()
{
    // Find color from lights
    vec4 lightColor = light_ambient;
    vec3 normal = normalize(varying_normal);
    
#if NUMBER_OF_LIGHTS > 0
    for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
    {
        // Geometry
        vec3 toLight = light_position[i] - varying_position;
        float lengthToLight = length(toLight);
        float intensity = 1.0 / (1.0 + light_falloff[i] * lengthToLight);
        
        // Diffuse term
        vec4 diffuse = light_color[i] * max(dot(normal, toLight / lengthToLight), 0.0);
        lightColor += diffuse * intensity;
        
        // Specular currently term is not used (only TR4 and up, was not yet implemented. Maybe later.)
    }
#endif
    
    // Combine with color from texture and vertex
    vec4 texColor = texture2D(color_map, varying_texCoord);
    gl_FragColor = texColor * lightColor * varying_color;
}
