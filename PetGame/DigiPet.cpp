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
			m_currentState(nullptr) {
			//Initial State
			ChangeState(new IdleState(), 0);
			setTexture("assets/doki.png");
		}

		Pet::~Pet()
		{
			if (m_currentState) {
				m_currentState->leave(this);
				delete m_currentState;
				m_currentState = nullptr;
			}
		}

		void Pet::Update(float deltaTime, int tickCount)
		{
			if (m_currentState) {
				m_currentState->update(this, deltaTime, tickCount);
			}
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

		void Pet::setTexture(const char* filePath)
		{
			if (!m_texture.Load(filePath))
				std::cout << "Pet without texture" << std::endl;
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
	}
}
