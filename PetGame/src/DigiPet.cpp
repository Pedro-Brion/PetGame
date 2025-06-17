#include "DigiPet.h"
#include "Shader.h"

namespace PetGame {
	namespace DigiPet {
		struct CONFIG {
			static const int MAX_HUNGER = 100;
			static const int MIN_HUNGER = 0;
		};
		Pet::Pet(const std::string& name) :
			m_name(name),
			m_hunger(50),
			m_experience(0),
			m_level(Level::Egg),
			m_currentState(nullptr),
			m_position(0.f, 0.f),
			m_size(0.f, 0.f),
			m_rotation(0.f),
			m_colorTint(1.f, 1.f, 1.f)
		{
			//Initial State
			ChangeState(new IdleState(), 0);
			setTextures();


			m_size = glm::vec2(64.f);
			static const glm::vec2 center = (glm::vec2(800.f, 600.f) / 2.f) - (m_size);
			m_position = center;
		}

		Pet::~Pet()
		{
			if (m_currentState) {
				m_currentState->leave(this);
				delete m_currentState;
				m_currentState = nullptr;
			}
		}

		void Pet::UpdateTick(float deltaTime, int tickCount)
		{

			if (m_currentState) {
				m_currentState->update(this, deltaTime, tickCount);
			}
			if (m_IsHurting) {
				this->hurting(tickCount);
			}
		}

		void Pet::UpdateRender(float deltaTime)
		{
			static float time = 0.f;
			static const glm::vec2 center = (glm::vec2(800.f, 600.f)) / 2.f;
			time += deltaTime;

			switch (m_level) {
			case (Level::Egg):
				m_position = glm::vec2(center.x + (glm::sin(time * 10.f) * 2.f), center.y);
				//m_rotation = -glm::sin(time * 10.f) * 10.f;
				break;
			case (Level::Puppy):
				m_position = glm::vec2(center.x, center.y + (glm::sin(time) * 2.f));
				break;
			default:
				m_position = m_position + glm::vec2(glm::sin(time), glm::cos(time));
			}

			m_position = center;
		}

		void Pet::ChangeState(IState* newState, int tick)
		{
			if (!newState) throw "Not a valid state";
			if (m_currentState) {
				m_currentState->leave(this);
				delete m_currentState;
			}
			m_currentState = newState;
			m_currentState->enter(this, tick);
		}

		void Pet::feed(int tick)
		{
			std::cout << "Feeding..." << std::endl;
			ChangeState(new FeedingState(), tick);
		}

		void Pet::train(const int hours)
		{
			setXp(m_experience + hours);
		}

		std::string Pet::getLevel() const
		{
			switch (m_level) {
			case(Level::Egg): return "Egg";
			case(Level::Puppy): return "Puppy";
			case(Level::Child): return "Child";
			case(Level::Adult): return "Adult";
			default: return "Unknown";
			}
		}

		Texture2D* Pet::getTexture() const
		{
			auto map = m_textures.find(m_level);
			if (map != m_textures.end()) {
				return map->second.get();
			}
			return m_textures.at(Level::Egg).get();
		}


		void Pet::setHunger(int value)
		{

			m_hunger = (value < CONFIG::MIN_HUNGER)
				? CONFIG::MIN_HUNGER : (value > CONFIG::MAX_HUNGER)
				? CONFIG::MAX_HUNGER : value;
		}

		void Pet::setXp(int value)
		{
			m_experience = (value < 0) ? 0 : value;
			displayStatus();
		}

		void Pet::setTextures()
		{
			m_textures[Level::Egg] = Texture2D::CreateTexture("assets/debug.png");
			m_textures[Level::Puppy] = Texture2D::CreateTexture("assets/baby1.png");
		}

		void Pet::displayStatus() const
		{
			std::cout << "___:::STATUS:::____ " << std::endl;
			std::cout << "Pet: " << m_name << std::endl;
			std::cout << "Hunger: " << m_hunger << std::endl;
			std::cout << "XP: " << m_experience << std::endl;
			std::cout << "Level: " << getLevel() << std::endl;
			std::cout << m_currentState->getCurrentActivity(const_cast<Pet*>(this)) << std::endl;
		}
		void Pet::hurt(int tick)
		{
			if (m_IsHurting)
				return;
			m_statedHurting = tick;
			m_IsHurting = true;
			hurting(tick);

		}
		void Pet::hurting(int tick)
		{
			m_colorTint = glm::vec3(0.8f, 0.5f, 0.5f);
			int ticksHurting = tick - m_statedHurting;
			std::cout << tick << "T || Hurting:" << ticksHurting << std::endl;
			if (ticksHurting < 1) {
				return;
			}
			else {
				m_colorTint = glm::vec3(1.f);
				m_IsHurting = false;
				m_statedHurting = 0;
			}
		}
	}
}
