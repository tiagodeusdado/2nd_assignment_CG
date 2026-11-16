#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 sunDir;

void main() {
    vec3 dir = normalize(TexCoords);
    
    // Atmospheric gradient based on elevation angle (dir.y)
    vec3 skyTop = vec3(0.18, 0.42, 0.78);     // Deep blue zenith
    vec3 skyHorizon = vec3(0.68, 0.82, 0.95); // Light cyan horizon
    vec3 groundColor = vec3(0.28, 0.35, 0.22); // Earth / grass tint below horizon
    
    vec3 color;
    if (dir.y >= 0.0) {
        float factor = pow(dir.y, 0.45);
        color = mix(skyHorizon, skyTop, factor);
        
        // Sun disk and glow
        float sunDot = max(dot(dir, normalize(sunDir)), 0.0);
        float sunGlow = pow(sunDot, 128.0) * 0.8;
        float sunDisk = step(0.9992, sunDot) * 1.5;
        color += (vec3(1.0, 0.95, 0.8) * (sunGlow + sunDisk));
    } else {
        float factor = clamp(-dir.y * 3.0, 0.0, 1.0);
        color = mix(skyHorizon, groundColor, factor);
    }
    
    FragColor = vec4(color, 1.0);
}
