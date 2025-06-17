#pragma once
#include "Shader.h"
#include "Texture2D.h"
#include "glm/glm.hpp"
#include "DigiPet.h"
#include <memory>
namespace PetGame {
	class SpriteRenderer
	{
	public:
		SpriteRenderer(Shader& shader);
		~SpriteRenderer();

		void DrawSprite(
			Texture2D* texture,
			glm::vec2 position,
			glm::vec2 size = glm::vec2(10.f, 10.f),
			float rotate = 0,
			glm::vec3 color = glm::vec3(1.f)
		);

		void DrawPet(const DigiPet::Pet* pet);

	private:
		Shader m_shader;
		unsigned int m_quadVAO;

		void Init();
	};

}

