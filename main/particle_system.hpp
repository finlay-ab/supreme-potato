#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <vector>
#include <glad/glad.h>

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "../support/program.hpp"

// Basic particle state
struct Particle {
    Vec3f position;
    Vec3f velocity;
    float life = -1.0f; // Negative values for dead particles
    float maxLife = 1.0f;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

	// Shader and texture setup
    void init(ShaderProgram* pShader, GLuint pTexture);

    // Run physics and spawn new particles
    void update(float dt, Vec3f emitterPos, bool active);

    // Draw the current group of particles
    void render(Mat44f const& viewProj);

    std::vector<Particle> particles;

private:
    // Maximum particle count
    static constexpr int kMaxParticles = 1000;

	// Open GL resources
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture = 0;
    ShaderProgram* shader = nullptr;
};

#endif // PARTICLE_SYSTEM_HPP