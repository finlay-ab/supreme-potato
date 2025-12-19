#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	return aM;
}


GLuint create_vao( SimpleMeshData const& aMeshData )
{
	// Create position vbo
	GLuint positionVBO = 0;
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO );
	glBufferData(GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f), aMeshData.positions.data(), GL_STATIC_DRAW);

	// Create color vbo
	GLuint colorVBO = 0;
	glGenBuffers(1, &colorVBO );
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO );
	glBufferData(GL_ARRAY_BUFFER, aMeshData.colors.size() * sizeof(Vec3f), aMeshData.colors.data(), GL_STATIC_DRAW);

	// Create normal vbo
	GLuint normalVBO = 0;
	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, aMeshData.normals.size() * sizeof(Vec3f), aMeshData.normals.data(), GL_STATIC_DRAW);

	// Create tex coords vbo
	GLuint texCoordsVBO = 0;
	glGenBuffers(1, &texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, aMeshData.texcoords.size() * sizeof(Vec2f), aMeshData.texcoords.data(), GL_STATIC_DRAW);

	// Create shine value vbo
	GLuint shineVBO = 0;
	glGenBuffers(1, &shineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, shineVBO);
	glBufferData(GL_ARRAY_BUFFER, aMeshData.shine.size() * sizeof(float), aMeshData.shine.data(), GL_STATIC_DRAW);

	// Create and bind vao 
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Configure position 
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

	// Configure color 
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

	// Configure normals
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

	// Configure tex coords
	if (!aMeshData.texcoords.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, texCoordsVBO);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}

	// Configure shine
	if (!aMeshData.shine.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, shineVBO);
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);
	}

	// clean 
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &colorVBO);
	glDeleteBuffers(1, &positionVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &texCoordsVBO);
	glDeleteBuffers(1, &shineVBO);

	// return 	
	return vao;

}