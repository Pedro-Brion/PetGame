#pragma once
#include <glad/glad.h> // Para carregar as funções do OpenGL
#include <GLFW/glfw3.h> // Para gerenciamento de janela e entrada
#include "DigiPet.h"
#include "Shader.h"
#include "SpriteRenderer.h"

namespace PetGame {
	class Application
	{
	public:
		Application();
		~Application();

		bool Init(const int width, const int height, const char* windowTitle);

		void Start();

		void Stop();

		void setWindowSize(int width, int height);

	private:
		GLFWwindow* m_window;

		int m_windowWidth;
		int m_windowHeight;

		int m_tickCount;
		float m_currentTime;
		float m_deltaTime;
		float m_fixedTickDuration;
		float m_timeAccumulator;

		Shader* m_shaderProgram;
		DigiPet::Pet* m_pet;
		SpriteRenderer* m_renderer;

		void ProcessInputs();
		void UpdateRender();
		void FixedUpdate();
		void Render();

	};
}

