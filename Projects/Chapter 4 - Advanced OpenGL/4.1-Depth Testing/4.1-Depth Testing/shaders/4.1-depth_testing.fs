#version 330 core
out vec4 FragColor;

float near = 0.1;
float far = 100;

float LinearizeDepth(float depth){
    return near / (near - far + far / depth);
}

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    // FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    
    // float depth = LinearizeDepth(gl_FragCoord.z);
    // FragColor = vec4(vec3(depth), 1.0);

    FragColor = texture(texture1, TexCoords);
}