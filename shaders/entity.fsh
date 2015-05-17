// GLSL fragment program for rendering entities

uniform sampler2D color_map;

uniform int number_of_lights;
uniform vec3 light_position[8];
uniform vec4 light_color[8];
uniform float light_falloff[8];
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
    
    for (int i = 0; i < number_of_lights; i++)
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
    
    // Combine with color from texture and vertex
    vec4 texColor = texture2D(color_map, varying_texCoord);
    gl_FragColor = texColor * lightColor * varying_color;
}
