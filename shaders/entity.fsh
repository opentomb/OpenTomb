// GLSL fragment program for rendering entities
// Must be created with a define for NUMBER_OF_LIGHTS first.

uniform sampler2D color_map;

#if NUMBER_OF_LIGHTS > 0
uniform vec3 light_position[NUMBER_OF_LIGHTS];
uniform vec4 light_color[NUMBER_OF_LIGHTS];
uniform float light_innerRadius[NUMBER_OF_LIGHTS];
uniform float light_outerRadius[NUMBER_OF_LIGHTS];
#endif
uniform vec4 light_ambient;

varying vec4 varying_color;
varying vec2 varying_texCoord;
varying vec3 varying_normal;
varying vec3 varying_position;

float shading(vec3 n, vec3 l) { return 0.5 + 0.5 * max(dot(n,normalize(l)), 0.0); }
float sumc(vec4 c){ return c.r + c.g + c.b; }

void main()
{
    // Find color from lights
    vec4 lightAmbient;
    vec4 lightColor = vec4(0.0);
    vec3 lightDirection = vec3(0.0,2.0,-1.0);
    vec3 normal = normalize(varying_normal);
    
#if NUMBER_OF_LIGHTS > 0
    vec3 current_light_position;//Or direction
    
    float current_light_intensity;
    float current_light_distance;
    
    for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
    {
        //'Classic' lightning
        //current_light_intensity = clamp(((light_outerRadius[i] - current_light_distance)/(light_outerRadius[i]-light_innerRadius[i])),0.0,1.0);
        
        //Soft lightning
        current_light_intensity = clamp(((light_outerRadius[i] - current_light_distance)/light_outerRadius[i]),0.0,1.0);
        
		//Skip light if its intensity is 0
        if(current_light_intensity == 0.0)
            continue;//Skip zero intensity lights
        
		// Geometry
        current_light_position = light_position[i] - varying_position;
        current_light_distance = length(current_light_position);
		
		lightDirection += current_light_position;
		
        // Diffuse term
        lightColor = max(lightColor, light_color[i] * current_light_intensity * shading(normal, current_light_position) );
        
        // Specular currently term is not used (only TR4 and up, was not yet implemented. Maybe later.)
    }
#endif
    lightAmbient = light_ambient * shading(normal,lightDirection);
    
    //Get texture (pixel).
    vec4 texColor = texture2D(color_map, varying_texCoord);
    
    // Combine lights and apply them to texture pixel.
    vec4 tmp = (lightAmbient + lightColor) * texColor * varying_color;
    
    //Use texture and lightning, but preserves alpha (transparency).
    gl_FragColor = vec4(tmp.rgb,texColor.a);
}
