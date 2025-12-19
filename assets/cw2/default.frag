#version 430

// Inputs from Vertex Shader
in vec3 v2fColor;
in vec3 v2fNormal;
in vec3 v2fPos;
in float vShininess;

// Uniform inputs (unchanging per draw call)
layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 3) uniform vec3 uLightDir; 
layout(location = 4) uniform vec3 uLightDiffuse; 
layout(location = 5) uniform vec3 uSceneAmbient;
layout(location = 6) uniform bool uDirLightEnabled;
layout(location = 7) uniform vec3 uCameraPos;
layout(location = 8) uniform float uShininess; // Overwrite shininess value (-1 to use MTL value)

// Point light struct for array
struct PointLight {
    vec3 position;
    vec3 color;
    bool enabled;
};

// Point light array for all 3 lights, each light will take up 3 consecutive uniform locations
layout(location = 9) uniform PointLight uPointLights[3];

// Output (per pixel colour)
out vec3 oColor;

void main()
{
    vec3 normal = normalize(v2fNormal);
    vec3 viewDir = normalize(uCameraPos - v2fPos);
    
    // Initialise final color to ambience 
    vec3 finalColor = uSceneAmbient;

    // If we have the directional light enabled then add the nDotL contribution to final colour
    if (uDirLightEnabled)
    {
        float nDotL = max(0.0, dot(normal, normalize(uLightDir)));
        finalColor += (nDotL * uLightDiffuse);
    }

    // Itterate over each point light and add its contribution to final colour if enabled
    for (int i = 0; i < 3; i++)
    {
        if (uPointLights[i].enabled)
        {
            vec3 lightDirRaw = uPointLights[i].position - v2fPos;
            float dist = length(lightDirRaw);
            vec3 lightDir = normalize(lightDirRaw);

            // Attenuation (1 / r^2)
            float attenuation = 1.0 / (dist * dist);

            // Diffuse
            float nDotL = max(0.0, dot(normal, lightDir));
            vec3 diffuse = uPointLights[i].color * nDotL;

            // Specular
            vec3 halfVec = normalize(lightDir + viewDir);
            float nDotH = max(0.0, dot(normal, halfVec));
            float actualShininess = (uShininess >= 0.0) ? uShininess : vShininess; // If a overwrite shine value is defined use it, otherwise use MTL
            float specularFactor = pow(nDotH, actualShininess);
            vec3 specular = uPointLights[i].color * specularFactor; // Multiply specular factor by the colour of light

            // Accumulate
            finalColor += (diffuse + specular) * attenuation;
        }
    }

    // Multiply the final colour by the colour received from vertex shader (for untextured objects)
    oColor = finalColor * v2fColor; 
}