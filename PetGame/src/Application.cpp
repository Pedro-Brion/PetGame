#include "Application.h"
#include "iostream"
#include "string"

namespace PetGame {
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	Application::Application()
		: m_window(nullptr),
		m_windowWidth(0),
		m_windowHeight(0),
		m_tickCount(0),
		m_currentTime(0),
		m_deltaTime(0),
		m_fixedTickDuration(1.f / 2.f),
		m_timeAccumulator(0),
		m_shaderProgram(nullptr),
		m_pet(nullptr),
		m_renderer(nullptr)
	{
	}

	Application::~Application()
	{
		delete m_shaderProgram;
		Stop();
	}

	bool Application::Init(const int width, const int height, const char* windowTitle)
	{
		m_windowWidth = width;
		m_windowHeight = height;

		if (!glfwInit()) {
			std::cout << "GLFW Was not initialized" << std::endl;
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, windowTitle, nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);


		if (m_window == NULL) {
			std::cout << "Failed to create window" << std::endl;
			Stop();
			return false;
		}

		// Conecting GLFW current context
		glfwMakeContextCurrent(m_window);

		// RegisterCallBack
		glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

		// Loading GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed load GLAD" << std::endl;
			Stop();
			return false;
		}

		m_shaderProgram = new Shader("shaders/sprite.vert", "shaders/sprite.frag");
		m_renderer = new SpriteRenderer(*m_shaderProgram);

		// Creating ViewPort
		glViewport(0, 0, m_windowWidth, m_windowHeight);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_currentTime = (float)glfwGetTime();

		m_pet = new DigiPet::Pet("Titanzada");

		return true;
	}

	void Application::Start()
	{
		m_shaderProgram->use();
		glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_windowWidth),
			0.0f, static_cast<float>(m_windowHeight),
			-1.0f, 1.0f);
		m_shaderProgram->setMat4("projection", projection);
		m_shaderProgram->setInt("spriteTexture", 0);

		while (!glfwWindowShouldClose(m_window)) {

			float currentFrameTime = (float)glfwGetTime();
			float deltaTime = currentFrameTime - m_currentTime;
			m_currentTime = currentFrameTime;
			m_deltaTime = deltaTime;
			if (deltaTime > 0.03)
			{
				std::cout << "LagSpike!!! FrameTime:" << deltaTime;
				std::cout << " | FPS:" << 1.f / deltaTime << std::endl;
			}

			m_timeAccumulator += deltaTime;
			while (m_timeAccumulator >= m_fixedTickDuration) {
				FixedUpdate();
				m_timeAccumulator -= m_fixedTickDuration;
			}

			ProcessInputs();
			Render();
			glfwPollEvents();
		}
	}

	void Application::Stop()
	{
		glfwTerminate();
		m_window = nullptr;
		std::cout << "Stoped" << std::endl;
	}

	void Application::setWindowSize(int width, int height)
	{
		m_windowWidth = width;
		m_windowHeight = height;

		m_shaderProgram->use();
		glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_windowWidth),
			0.0f, static_cast<float>(m_windowHeight),
			-1.0f, 1.0f);
		m_shaderProgram->setMat4("projection", projection);
	}

	void Application::ProcessInputs()
	{

		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(m_window, true);
		}

		// C Feed
		static bool cKeyPressed = false;
		if (glfwGetKey(m_window, GLFW_KEY_C) == GLFW_PRESS && !cKeyPressed) {
			m_pet->feed(m_tickCount);
			cKeyPressed = true;
		} if (glfwGetKey(m_window, GLFW_KEY_C) == GLFW_RELEASE) cKeyPressed = false;

		// Z Train
		static bool zKeyPressed = false;
		if (glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_PRESS && !zKeyPressed) {
			m_pet->train(5);
			zKeyPressed = true;
		} if (glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_RELEASE) zKeyPressed = false;

		// X Display Status
		static bool xKeyPressed = false;
		if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS && !xKeyPressed) {
			m_pet->displayStatus();
			xKeyPressed = true;
		} if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_RELEASE) xKeyPressed = false;

		static bool spacePressed = false;
		if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
			m_pet->hurt(m_tickCount);
			spacePressed = true;
		} if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_RELEASE) spacePressed = false;
	}

	void Application::UpdateRender()
	{
		m_pet->UpdateRender(m_deltaTime);
		m_renderer->DrawPet(m_pet);
	}

	void Application::FixedUpdate()
	{
		m_pet->UpdateTick(m_fixedTickDuration, m_tickCount);
		m_tickCount++;
	}

	void Application::Render()
	{
		glClearColor(.941f, .917f, .854f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		UpdateRender();

		glfwSwapBuffers(m_window);
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		if (app) {
			app->setWindowSize(width, height); // Cria um novo método para a lógica
		}
		std::string newTitle = "Tamagochi:: " + std::to_string(width) + " x " + std::to_string(height);
		glfwSetWindowTitle(window, newTitle.c_str());
	}

}