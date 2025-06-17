#include "SpriteRenderer.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <iostream>

PetGame::SpriteRenderer::SpriteRenderer(Shader& shader)
	:m_shader(shader)
{
	Init();
}

PetGame::SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &m_quadVAO);
}

void PetGame::SpriteRenderer::DrawSprite(Texture2D* texture, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color)
{
	m_shader.use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.f, 0.f, 1.f));
	model = glm::scale(model, glm::vec3(size, 1.f));
	

	m_shader.setMat4("model", model);
	m_shader.setVec3("spriteColor", color);

	glActiveTexture(GL_TEXTURE0);
	texture->Bind();

	glBindVertexArray(m_quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void PetGame::SpriteRenderer::DrawPet(const DigiPet::Pet* pet)
{
	this->DrawSprite(pet->getTexture(),pet->getPosition(),pet->getSize(),pet->getRotation(),pet->getColorTint());
}

void PetGame::SpriteRenderer::Init()
{
	// Setting up quad
	float quadVertices[] = {
		// Position				//Text coord
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, // Bottom Left
		-0.5f,	0.5f, 0.0f,		0.0f, 1.0f, // Top Left
		 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, // Top Right
		 0.5f, -0.5f, 0.0f,		1.0f, 0.0f, // Bottom Right
	};
	unsigned int quadIndices[] = {
		0,1,2,
		0,2,3,
	};

	unsigned int VBO, EBO;
	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(m_quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(0));
	glEnableVertexAttribArray(0);
	// Texture Coord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
