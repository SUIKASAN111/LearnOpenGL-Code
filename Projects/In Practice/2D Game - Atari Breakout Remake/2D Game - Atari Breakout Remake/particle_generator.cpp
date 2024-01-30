#include "particle_generator.h"

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, uint32_t amount)
	: m_shader(shader), m_texture(texture), m_amount(amount)
{
	this->init();
}

void ParticleGenerator::Update(float dt, GameObject& object, uint32_t newParticles, glm::vec2 offset)
{
	// add new particles
	for (uint32_t i = 0; i < newParticles; ++i)
	{
		int unusedParticle = this->firstUnusedParticle();
		this->respawnParticle(this->m_particles[unusedParticle], object, offset);
	}
	// update all particles
	for (uint32_t i = 0; i < this->m_amount; ++i)
	{
		Particle& p = this->m_particles[i];
		p.Life -= dt; // reduce life
		if (p.Life > 0.0f) // particle is alive, thus update
		{
			p.Position -= p.Velocity * dt;
			p.Color.a -= dt * 2.5f;
		}
	}
}

// render all particles
void ParticleGenerator::Draw()
{
	// use additive blending to give it a "glow" effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	this->m_shader.Use();
	for (Particle particle : this->m_particles)
	{
		if (particle.Life > 0.0f)
		{
			this->m_shader.SetVector2f("offset", particle.Position);
			this->m_shader.SetVector4f("color", particle.Color);
			this->m_texture.Bind();
			glBindVertexArray(this->m_VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	// don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
	// set up mesh and attribute properties
	uint32_t VBO;
	float particle_quad[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};
	glGenVertexArrays(1, &this->m_VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->m_VAO);
	// fill mesh buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
	// set mesh attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	
	// create this->m_ammout default particles instances
	for (uint32_t i = 0; i < this->m_amount; ++i)
	{
		this->m_particles.push_back(Particle());
	}
}

// stores the index of the last particle used (for quick access to next dead particle)
uint32_t lastUsedParticle = 0;
uint32_t ParticleGenerator::firstUnusedParticle()
{
	// first search from last used particle, this will usually return almost instantly
	for (uint32_t i = lastUsedParticle; i < this->m_amount; ++i)
	{
		if (this->m_particles[i].Life <= 0.0f)
		{
			lastUsedParticle = i;
			return i;
		}
	}
	// otherwise, do a linear search
	for (uint32_t i = 0; i < lastUsedParticle; ++i)
	{
		if (this->m_particles[i].Life <= 0.0f)
		{
			lastUsedParticle = i;
			return i;
		}
	}
	// all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	lastUsedParticle = 0;
	return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset)
{
	float random = ((rand() % 100) - 50) / 10.0f;
	float rColor = 0.5f + ((rand() % 100) / 100.0f);
	particle.Position = object.Position + random + offset;
	particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
	particle.Life = 1.0f;
	particle.Velocity = object.Velocity * 0.1f;
}