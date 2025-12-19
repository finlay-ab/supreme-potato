#version 430

// Attributes from the C++ particle struct
layout(location = 0) in vec3 iPosition;
layout(location = 1) in float iLife;

// Just the MVP matrix, no model matrix needed since we simulate in world space
layout(location = 0) uniform mat4 uProjCameraWorld;

out float vLife;

void main()
{
    // Transform to clip space
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);
    
    // Scale the point sprite based on how far away it is, gl_Position.w is the depth 
    float dist = gl_Position.w;

    // 500 is just an arbitrary size
    gl_PointSize = 500.0 / dist; 

    // Pass life to frag shader so we can fade alpha over time
    vLife = iLife;
}