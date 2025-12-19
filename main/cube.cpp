#include "cube.hpp"

#include "simple_mesh.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp" 

#include <vector>

SimpleMeshData make_cube( Vec3f aColor, Mat44f aPreTransform )
{
    std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    Mat33f N = mat44_to_mat33(transpose(invert(aPreTransform)));

    Vec3f const faceNormals[6] = {
        { 0.f, 1.f, 0.f },  
        { 0.f, 0.f, 1.f }, 
        { -1.f, 0.f, 0.f }, 
        { 0.f, -1.f, 0.f }, 
        { 1.f, 0.f, 0.f },  
        { 0.f, 0.f, -1.f }  
    };

    pos.reserve(36);
    normals.reserve(36);
    for (int i = 0; i < 36; ++i)
    {
        pos.emplace_back( Vec3f{
            kCubePositions[i * 3 + 0],
            kCubePositions[i * 3 + 1],
            kCubePositions[i * 3 + 2]
        });

        
        normals.emplace_back( faceNormals[i / 6] );
    }

    for( auto& p : pos ) 
    {
        Vec4f p4{ p.x, p.y, p.z, 1.f };
        Vec4f t = aPreTransform * p4;
        t /= t.w; 
        p = Vec3f{ t.x, t.y, t.z };
    }

    for( auto& n : normals )
    {
        Vec3f t = N * n;
        n = normalize(t);
    }

    std::vector col( pos.size(), aColor );

    return SimpleMeshData{ std::move(pos), std::move(col), std::move(normals) };
}