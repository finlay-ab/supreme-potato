#ifndef CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B
#define CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B

#include <vector>

#include <cstdlib>

#include "simple_mesh.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

// This defines the vertex data for a colored unit cube.

constexpr float const kCubePositions[] = {
	+1.f, +1.f, -1.f,
	-1.f, +1.f, -1.f,
	-1.f, +1.f, +1.f,
	+1.f, +1.f, -1.f,
	-1.f, +1.f, +1.f,
	+1.f, +1.f, +1.f,

	+1.f, -1.f, +1.f,
	+1.f, +1.f, +1.f,
	-1.f, +1.f, +1.f,
	+1.f, -1.f, +1.f,
	-1.f, +1.f, +1.f,
	-1.f, -1.f, +1.f,

	-1.f, -1.f, +1.f,
	-1.f, +1.f, +1.f,
	-1.f, +1.f, -1.f,
	-1.f, -1.f, +1.f,
	-1.f, +1.f, -1.f,
	-1.f, -1.f, -1.f,

	-1.f, -1.f, -1.f,
	+1.f, -1.f, -1.f,
	+1.f, -1.f, +1.f,
	-1.f, -1.f, -1.f,
	+1.f, -1.f, +1.f,
	-1.f, -1.f, +1.f,

	+1.f, -1.f, -1.f,
	+1.f, +1.f, -1.f,
	+1.f, +1.f, +1.f,
	+1.f, -1.f, -1.f,
	+1.f, +1.f, +1.f,
	+1.f, -1.f, +1.f,

	-1.f, -1.f, -1.f,
	-1.f, +1.f, -1.f,
	+1.f, +1.f, -1.f,
	-1.f, -1.f, -1.f,
	+1.f, +1.f, -1.f,
	+1.f, -1.f, -1.f,
};

constexpr float const kCubeColors[] = {
	+1.f, +0.f, +0.f,
	+1.f, +0.f, +0.f,
	+1.f, +0.f, +0.f,
	+1.f, +0.f, +0.f,
	+1.f, +0.f, +0.f,
	+1.f, +0.f, +0.f,

	+0.f, +1.f, +0.f,
	+0.f, +1.f, +0.f,
	+0.f, +1.f, +0.f,
	+0.f, +1.f, +0.f,
	+0.f, +1.f, +0.f,
	+0.f, +1.f, +0.f,

	+0.f, +0.f, +1.f,
	+0.f, +0.f, +1.f,
	+0.f, +0.f, +1.f,
	+0.f, +0.f, +1.f,
	+0.f, +0.f, +1.f,
	+0.f, +0.f, +1.f,

	+1.f, +0.f, +1.f,
	+1.f, +0.f, +1.f,
	+1.f, +0.f, +1.f,
	+1.f, +0.f, +1.f,
	+1.f, +0.f, +1.f,
	+1.f, +0.f, +1.f,

	+1.f, +1.f, +0.f,
	+1.f, +1.f, +0.f,
	+1.f, +1.f, +0.f,
	+1.f, +1.f, +0.f,
	+1.f, +1.f, +0.f,
	+1.f, +1.f, +0.f,

	+0.f, +1.f, +1.f,
	+0.f, +1.f, +1.f,
	+0.f, +1.f, +1.f,
	+0.f, +1.f, +1.f,
	+0.f, +1.f, +1.f,
	+0.f, +1.f, +1.f
};

static_assert( sizeof(kCubeColors) == sizeof(kCubePositions),
	"Size of cube colors and cube positions do not match. Both are 3D vectors."
);

SimpleMeshData make_cube( Vec3f aColor, Mat44f aPreTransform );

#endif // CUBE_HPP_6874B39C_112D_4D34_BD85_AB81A730955B
