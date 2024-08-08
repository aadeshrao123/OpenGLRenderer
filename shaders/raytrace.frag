#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse;
uniform samplerCube skybox;

struct Light {
    vec3 position;
    vec3 color;
};

uniform Light light1;
uniform Light light2;

void main()
{
    vec3 ambient = 0.1 * texture(texture_diffuse, TexCoords).rgb;

    // Light 1
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light1.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light1.color * texture(texture_diffuse, TexCoords).rgb;

    // Light 2
    lightDir = normalize(light2.position - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    diffuse += diff * light2.color * texture(texture_diffuse, TexCoords).rgb;

    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
