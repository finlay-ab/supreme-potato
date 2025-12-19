#include "cone.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include <numbers>

SimpleMeshData make_cone(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    std::vector<Vec3f> pos;
    std::vector<Vec3f> normals;

    Mat33f N = mat44_to_mat33(transpose(invert(aPreTransform)));

    float prevy = std::cos(0.f);
    float prevz = std::sin(0.f);

    for (std::size_t i = 0; i < aSubdivs; i++)
    {
        float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float const y = std::cos(angle);
        float const z = std::sin(angle);

        pos.emplace_back(Vec3f{0.0f, prevy, prevz});
        normals.emplace_back(normalize(Vec3f{1.0f, prevy, prevz}));

        pos.emplace_back(Vec3f{0.0f, y, z});
        normals.emplace_back(normalize(Vec3f{1.0f, y, z}));

        pos.emplace_back(Vec3f{1.0f, 0.0f, 0.0f});
        normals.emplace_back(normalize(Vec3f{1.0f, (prevy + y) / 2.f, (prevz + z) / 2.f}));

        if (aCapped)
        {
            pos.emplace_back(Vec3f{0.0f, 0.0f, 0.0f});
            normals.emplace_back(Vec3f{-1.0f, 0.0f, 0.0f});

            pos.emplace_back(Vec3f{0.0f, y, z});
            normals.emplace_back(Vec3f{-1.0f, 0.0f, 0.0f});

            pos.emplace_back(Vec3f{0.0f, prevy, prevz});
            normals.emplace_back(Vec3f{-1.0f, 0.0f, 0.0f});
        }

        prevy = y;
        prevz = z;
    }


    for (auto &p : pos)
    {
        Vec4f p4{p.x, p.y, p.z, 1.f};
        Vec4f t = aPreTransform * p4;
        t /= t.w;
        p = Vec3f{t.x, t.y, t.z};
    }

    for (auto &n : normals)
    {
        Vec3f t = N * n;
        n = normalize(t);
    }

    std::vector col(pos.size(), aColor);
    return SimpleMeshData{std::move(pos), std::move(col), std::move(normals)};
}