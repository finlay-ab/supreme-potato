#version 430
layout(location = 0) in vec3 iPosition;
layout(location = 2) in vec3 iNormal;
layout(location = 3) in vec2 iTexCoord;

layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 v2fPos;

void main()
{
    vNormal = normalize(uNormalMatrix * iNormal);
    vTexCoord = iTexCoord;
    v2fPos = iPosition;
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);
}