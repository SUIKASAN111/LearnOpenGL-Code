#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{    
    float eta1 = 1.00f;
    float eta2 = 1.52f;
    float ratio = eta1 / eta2;
    float R0 = pow(((eta1 - eta2) / (eta1 + eta2)), 2);

    vec3 I = normalize(Position - cameraPos);
    vec3 N = normalize(Normal);

    float fresnel = R0 + (1 - R0) * pow((1 - dot(N, -I)), 5);

    vec3 reflectDir = reflect(I, N);
    vec3 refractDir = refract(I, N, ratio);

    FragColor = vec4(texture(skybox, reflectDir).rgb, 1.0) * fresnel + vec4(texture(skybox, refractDir).rgb, 1.0) * (1 - fresnel);
}