#version 430

// Inputs - Position, Colour & Normal
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iColor;
layout(location = 2) in vec3 iNormal;
layout(location = 4) in float iShininess;

// Uniform Inputs (unchanging per draw call) - Projection * Camera * World matrix, Normal Matrix & Model Matrix (required for lighting calculations)
layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 2) uniform mat4 uModelMatrix;

// Outputs (to fragment) - Colour, Normal & Position (in world space)
out vec3 v2fColor;
out vec3 v2fNormal;
out vec3 v2fPos;
out float vShininess;

void main()
{
    v2fColor = iColor;

    vShininess = iShininess;
    
    v2fNormal = normalize(uNormalMatrix * iNormal);

    v2fPos = vec3(uModelMatrix * vec4(iPosition, 1.0));

    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);
}