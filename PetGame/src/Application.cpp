#include "Application.h"
#include "iostream"
#include "string"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


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

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls


		if (m_window == NULL) {
			std::cout << "Failed to create window" << std::endl;
			Stop();
			return false;
		}


		// Conecting GLFW current context
		glfwMakeContextCurrent(m_window);

		ImGui_ImplGlfw_InitForOpenGL(m_window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
		ImGui_ImplOpenGL3_Init();

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

			glfwPollEvents();

			PetGame::Application::RenderUi();

			PetGame::Application::ProcessInputs();
			PetGame::Application::Render();
		}
	}

	void Application::Stop()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

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
			m_pet->displayStatus();
			zKeyPressed = true;
		} if (glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_RELEASE) zKeyPressed = false;

		// X Display Status
		static bool xKeyPressed = false;
		if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS && !xKeyPressed) {
			m_guiOpen = true;
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

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
	}

	void Application::RenderUi()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize(ImVec2(m_windowWidth, 100), ImGuiCond_Always); // Set size (width, height)
		ImGui::SetNextWindowPos(ImVec2(0, m_windowHeight - 100), ImGuiCond_Always);  // Set position (x, y)

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoCollapse;
		if (m_guiOpen)
		{
			ImVec4 green(0.082f, 0.494f, 0.082f,0.f);
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_WindowBg] = ImVec4(green.x, green.y, green.z, 0.5f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(green.x, green.y, green.z, 1.f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(green.x, green.y, green.z, 1.f));
			if (ImGui::Begin("DigiPet Status", &m_guiOpen, flags)) {
				// Status
				ImGui::BeginGroup();
				{
					ImGui::Text("Name: %s", m_pet->getName().c_str());
					ImGui::Text("Level: %s", m_pet->getLevel().c_str());
					ImGui::Text("Hunger: %d/100", m_pet->getHunger());
				}
				ImGui::EndGroup();

				ImGui::SameLine(0,80);
				ImGui::BeginGroup();
				{
					ImVec2 size = ImGui::GetItemRectSize();
					if (ImGui::Button("Feed", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x) * 0.5f, size.y / 2))) {
						m_pet->feed(m_tickCount);
					}
					
					ImGui::Button("Train", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x) * 0.5f, size.y /2));
				}
				ImGui::EndGroup();

				ImGui::End();
			}
			else {
				ImGui::End();
			}
			ImGui::PopStyleColor();
		}

		ImGui::ShowDemoWindow();
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