#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj( char const* aPath )
{
	auto res = rapidobj::ParseFile(aPath);
	if (res.error)
	{
		throw Error("Unable to load OBJ file '{}' : {}", aPath, res.error.code.message());
	}
	
	rapidobj::Triangulate(res);

	SimpleMeshData ret;

	for (auto const& shape : res.shapes)
	{
		for (std::size_t i = 0; i < shape.mesh.indices.size(); i++)
		{
			auto const& idx = shape.mesh.indices[i];
			auto const& mat = res.materials[shape.mesh.material_ids[i / 3]];

			ret.positions.emplace_back ( Vec3f {
				res.attributes.positions[idx.position_index*3+0],
				res.attributes.positions[idx.position_index*3+1],
				res.attributes.positions[idx.position_index*3+2]
			} );

			// Normals
			if (idx.normal_index >= 0) 
            {
                ret.normals.emplace_back( Vec3f {
                    res.attributes.normals[idx.normal_index*3+0],
                    res.attributes.normals[idx.normal_index*3+1],
                    res.attributes.normals[idx.normal_index*3+2]
                });
            }
            else 
            {
				// if there is no normal data set up (avoids crashing)
                ret.normals.emplace_back( Vec3f { 1.0f, 0.0f, 0.0f } );
            }

			// Texture coordinates
			if (idx.texcoord_index >= 0)
			{
				ret.texcoords.emplace_back(Vec2f{
					res.attributes.texcoords[idx.texcoord_index * 2 + 0],
					res.attributes.texcoords[idx.texcoord_index * 2 + 1]
				});
			}
			else
			{
				// if there is no texture data set up (avoids crashing)
				ret.texcoords.emplace_back(Vec2f{ 0.0f, 0.0f });
			}

			/*if (i < 5) {
				std::printf("Material: %s, Shininess: %f, Diffuse: %f %f %f\n",
					mat.name.c_str(),
					mat.shininess,
					mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
			}*/

			// Colours
			ret.colors.emplace_back(Vec3f{
				mat.diffuse[0], // Was mat.ambient[0]
				mat.diffuse[1], // Was mat.ambient[1]
				mat.diffuse[2]  // Was mat.ambient[2]
				});

			// Shine
			ret.shine.push_back(mat.shininess);

			/*auto const& mat = res.materials[shape.mesh.material_ids[i / 3]];

			ret.colors.emplace_back( Vec3f {
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			});*/
		}
	}

	return ret;
}

