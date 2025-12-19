#version 430

in float vLife;

layout(location = 1) uniform sampler2D uTexture;
layout(location = 2) uniform vec4 uColor;

out vec4 oColor;

void main()
{
    // GL_POINTS gives the UVs (0,0 to 1,1)
    vec4 texColor = texture(uTexture, gl_PointCoord);
    
    // Combine the texture sample with color.
    // Then multiply alpha by vLife, particle will fade out over time
    oColor = texColor * uColor * vec4(1.0, 1.0, 1.0, vLife);
}