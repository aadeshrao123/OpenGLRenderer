#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform samplerCube skybox;
uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform vec3 lightPos2;
uniform vec3 lightColor2;
uniform vec3 viewPos;

const float PI = 3.14159265359;

// Calculate normal from normal map
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal, TexCoord).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Geometry function for Smith's method
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry function for Smith's method
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Geometry function for Smith's method
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// PBR lighting calculation
vec3 calculatePBR(vec3 N, vec3 V, vec3 L, vec3 albedo, vec3 F0, float metallic, float roughness, vec3 lightColor)
{
    vec3 H = normalize(V + L);
    float distance = length(L - V);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    // Add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
    // Material properties
    vec3 albedo = texture(texture_diffuse, TexCoord).rgb;
    float metallic = 0.5;
    float roughness = 0.5;
    vec3 F0 = vec3(0.04); // Default F0 value for non-metals
    F0 = mix(F0, albedo, metallic);

    // Normal from normal map
    vec3 N = getNormalFromMap();

    // View and light directions
    vec3 V = normalize(viewPos - FragPos);
    vec3 L1 = normalize(lightPos1 - FragPos);
    vec3 L2 = normalize(lightPos2 - FragPos);

    // PBR lighting
    vec3 Lo = calculatePBR(N, V, L1, albedo, F0, metallic, roughness, lightColor1);
    Lo += calculatePBR(N, V, L2, albedo, F0, metallic, roughness, lightColor2);

    // Ambient lighting (using environment map for IBL)
    vec3 ambient = vec3(0.1) * albedo; // Increase ambient light
    vec3 envColor = texture(skybox, reflect(-V, N)).rgb;
    vec3 finalColor = ambient + Lo + envColor;

    // Apply gamma correction
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}
