#version 330 core

// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 EyeDirection_worldspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler3D radianceDistribution;

void main(){
    
    vec3 radiance = vec3(0,0,0);
    vec3 p = Position_worldspace + vec3(10,10,10);
    vec3 px;
    px.x = round(p.x);
    px.y = round(p.y);
    px.z = round(p.z);
    
    for (int i = 0; i <= 3; i++) {
        radiance += texture(radianceDistribution, p).rgb;
        p = p + normalize(EyeDirection_worldspace);
        px.x = round(p.x);
        px.y = round(p.y);
        px.z = round(p.z);
    }
    
    color = radiance;
    
}
