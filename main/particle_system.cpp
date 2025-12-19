#include "particle_system.hpp"

#include <cstdlib> 
#include <vector>

// Constructor
ParticleSystem::ParticleSystem()
{
    // Reserving memory before hand so no realtime allocations are made
    particles.resize(kMaxParticles);

    // Generate opengl buffers
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Allocate max size on gpu
    // 4 floats per particle for 3d position and life time value
    glBufferData(GL_ARRAY_BUFFER, kMaxParticles * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // Life attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(3 * sizeof(float)));

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Destructor
ParticleSystem::~ParticleSystem()
{
    // Get rid of buffers on exit
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
}

// Initialization
void ParticleSystem::init(ShaderProgram* pShader, GLuint pTexture)
{
    shader = pShader;
    texture = pTexture;
}

// Update Simulation Loop
void ParticleSystem::update(float dt, Vec3f emitterPos, bool active)
{
    // Spawn 2 per frame if active
    int spawnCount = active ? 2 : 0;

    for (auto& p : particles)
    {
        if (spawnCount <= 0) break;

        // Any dead particles can be re used (pooling)
        if (p.life < 0.0f)
        {
            // Reset life and velocity randomly between 1.0 to 1.5s
            p.life = 1.0f + ((std::rand() % 100) / 100.0f) * 0.5f;
            p.maxLife = p.life;
            p.position = emitterPos;
            float rx = ((std::rand() % 100) / 50.0f) - 1.0f;
            float rz = ((std::rand() % 100) / 50.0f) - 1.0f;

			// Set downward velocity with some random x/z movement
            p.velocity = Vec3f{ rx, -3.0f, rz };

            spawnCount--;
        }
    }

    // Update physics for all particles (pos and life values)
    for (auto& p : particles)
    {
        if (p.life > 0.0f)
        {
            p.position += p.velocity * dt;
            p.life -= dt;
        }
    }
}

// Render Loop
void ParticleSystem::render(Mat44f const& viewProj)
{
    if (!shader) return;

    // Static vector and pre reservation stops reallocation
    static std::vector<float> gpuData;
    gpuData.clear();

    if (gpuData.capacity() < kMaxParticles * 4)
    {
        gpuData.reserve(kMaxParticles * 4);
    }

    int activeCount = 0;
    for (const auto& p : particles)
    {
        if (p.life > 0.0f)
        {
            gpuData.push_back(p.position.x);
            gpuData.push_back(p.position.y);
            gpuData.push_back(p.position.z);
            gpuData.push_back(p.life / p.maxLife);

            activeCount++;
        }
    }

	// If there is no active particles we get to save resources
    if (activeCount == 0) return;

	// Additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Stop depth writing so particles don't occlude each other
    glDepthMask(GL_FALSE);

	// Deeded for resizing points in particle vertex shader
    glEnable(GL_PROGRAM_POINT_SIZE);
    glUseProgram(shader->programId());

	// Upload data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Using sub data instea of buffer data so we don't re allocate memory on gpu
    glBufferSubData(GL_ARRAY_BUFFER, 0, gpuData.size() * sizeof(float), gpuData.data());

    // View projection matrix
    glUniformMatrix4fv(0, 1, GL_TRUE, viewProj.v);

    // Add the fire colour 
    glUniform4f(2, 1.0f, 0.4f, 0.2f, 1.0f);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(1, 0);

	// Draw the particles as points
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, activeCount);

    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}