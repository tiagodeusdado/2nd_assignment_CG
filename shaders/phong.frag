#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Material properties
uniform vec3 objectColor;
uniform vec3 specularColor;
uniform float shininess;

// Camera
uniform vec3 viewPos;

// Point Light properties (as required by assignment)
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightConstant;
uniform float pointLightLinear;
uniform float pointLightQuadratic;
uniform float pointLightIntensity;

// Directional ambient/sunlight to provide realistic overall scene lighting
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;
uniform float ambientStrength;

vec3 calculatePointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(pointLightPos - fragPos);
    float distance = length(pointLightPos - fragPos);
    
    // Attenuation
    float attenuation = 1.0 / (pointLightConstant + pointLightLinear * distance + pointLightQuadratic * (distance * distance));
    
    // Diffuse shading (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * pointLightColor * objectColor;
    
    // Specular shading (Classic Phong: reflect)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * pointLightColor * specularColor;
    
    return (diffuse + specular) * attenuation * pointLightIntensity;
}

vec3 calculateDirectionalLight(vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-dirLightDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * dirLightColor * objectColor;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * dirLightColor * specularColor * 0.3;
    
    return diffuse + specular;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 1. Ambient component
    vec3 ambient = ambientStrength * dirLightColor * objectColor;
    
    // 2. Point Light component (Required)
    vec3 pointLight = calculatePointLight(norm, FragPos, viewDir);
    
    // 3. Directional sun component
    vec3 dirLight = calculateDirectionalLight(norm, viewDir);
    
    vec3 result = ambient + pointLight + dirLight;
    
    // Simple tone mapping / clamp
    result = clamp(result, 0.0, 1.0);
    
    FragColor = vec4(result, 1.0);
}
