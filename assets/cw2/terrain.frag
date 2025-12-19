#version 430

// Inputs from Vertex Shader
in vec3 vNormal;
in vec2 vTexCoord;
in vec3 v2fPos;

layout(location = 2) uniform vec3 uLightDir;
layout(location = 3) uniform vec3 uLightDiffuse;
layout(location = 4) uniform vec3 uSceneAmbient;
layout(location = 5) uniform sampler2D uTextureMap;
layout(location = 6) uniform bool uDirLightEnabled;
layout(location = 7) uniform vec3 uCameraPos; 
layout(location = 8) uniform float uShininess; 

// Point light struct for array
struct PointLight {
    vec3 position;
    vec3 color;
    bool enabled;
};

// Point light array for all 3 lights, each light will take up 3 consecutive uniform locations
layout(location = 9) uniform PointLight uPointLights[3];

// Output (per pixel colour)
out vec4 oColor;

void main()
{
    // Get texture colour at texture coordinate
    vec3 texColor = texture(uTextureMap, vTexCoord).rgb;
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(uCameraPos - v2fPos);

    // Initialise final colour to ambient scene colour
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
            float specularFactor = pow(nDotH, uShininess);
            vec3 specular = uPointLights[i].color * specularFactor; // Multiply specular factor by the colour of light

            // Accumulate
            finalColor += (diffuse + specular) * attenuation;
        }
    }

    // Multiply texture colour by the final colour and output
    oColor = vec4(texColor * finalColor, 1.0);
}