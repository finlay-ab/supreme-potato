#include "texture.hpp"

GLuint load_texture_2d(char const* aPath)
{
	assert(aPath);
	stbi_set_flip_vertically_on_load(true);
	int w, h, channels;

	unsigned char* data = stbi_load(aPath, &w, &h, &channels, 4);

	if (!data)
	{
		throw Error("Unable to load image '{}'\n", aPath);
	}

	GLuint texId = 0;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);

	// Generate mip map
	glGenerateMipmap(GL_TEXTURE_2D);

	// Configure texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 6.f);

	return texId;
}
