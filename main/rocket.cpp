#include "rocket.hpp"

#include "cylinder.hpp"
#include "cone.hpp" 
#include "cube.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

#include <numbers> 

SimpleMeshData create_rocket()
{
    // core (red)
    auto core = make_cylinder(true, 16, {0.85f, 0.85f, 0.85f},
        make_rotation_z(std::numbers::pi_v<float> / 2.f) *
        make_scaling(Vec3f{ 4.0f, 0.5f, 0.5f}) 
    );

    // top
    auto top = make_cone(true, 16, {0.85f, 0.85f, 0.85f},
        make_rotation_z(std::numbers::pi_v<float> / 2.f) *
        make_translation(Vec3f{4.0f, 0.0f, 0.0f}) *
        make_scaling(Vec3f{1.0f, 0.5f, 0.5f})
    );

    // winglets 
    auto winglet1 = make_cube({0.75f, 0.75f, 0.80f},
        make_translation(Vec3f{1.0f, 1.0f, 0.0f}) * 
        make_scaling(Vec3f{0.5f, 0.8f, 0.05f}) 
    );

    auto winglet2 = make_cube({0.75f, 0.75f, 0.80f},
        make_rotation_y(std::numbers::pi_v<float> / 2.f) * 
        make_translation(Vec3f{1.0f, 1.0f, 0.0f}) *
        make_scaling(Vec3f{0.5f, 0.8f, 0.05f})
    );

    auto winglet3 = make_cube({0.75f, 0.75f, 0.80f},
        make_rotation_y(std::numbers::pi_v<float>) * 
        make_translation(Vec3f{1.0f, 1.0f, 0.0f}) *
        make_scaling(Vec3f{0.5f, 0.8f, 0.05f})
    );

    auto winglet4 = make_cube({0.75f, 0.75f, 0.80f},
        make_rotation_y(std::numbers::pi_v<float> * 1.5f) * 
        make_translation(Vec3f{1.0f, 1.0f, 0.0f}) *
        make_scaling(Vec3f{0.5f, 0.8f, 0.05f})
    );

    // engine 
    auto engine = make_cone(true, 16, {0.20f, 0.20f, 0.25f},
        make_rotation_z(std::numbers::pi_v<float> / 2.f) *
        make_translation(Vec3f{-1.0f, 0.0f, 0.0f}) *
        make_scaling(Vec3f{2.0f, 0.3f, 0.3f})
    );

    auto rocket = concatenate( std::move(core) , top);
    rocket = concatenate( std::move(rocket) , winglet1);
    rocket = concatenate( std::move(rocket) , winglet2);
    rocket = concatenate( std::move(rocket) , winglet3);
    rocket = concatenate( std::move(rocket) , winglet4);
    rocket = concatenate( std::move(rocket) , engine);

    return rocket;
}