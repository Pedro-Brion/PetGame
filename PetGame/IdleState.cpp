#include "IdleState.h"
#include "DigiPet.h"
#include <iostream>

namespace PetGame {
	namespace DigiPet {
		IdleState::IdleState() :m_lastHungerTick(0) {
		}
		IdleState::~IdleState() {}
		void IdleState::enter(Pet* pet, int tick) {
			std::cout << pet->getName() << "Is idle" << std::endl;
			m_lastHungerTick = tick;
		}
		void IdleState::update(Pet* pet, float deltaTime, int currentTick) {
			int ticksSinceLastHunger = currentTick - m_lastHungerTick;
			if (ticksSinceLastHunger >= TICKS_TO_HUNGER) {
				m_lastHungerTick = currentTick;
				pet->setHunger(pet->getHunger() + 1);
				std::cout << pet->getName() << " got a bit hungrier..." << std::endl;
			}
		}
		void IdleState::leave(Pet* pet, int tick) {
			std::cout << pet->getName() << " is no longer Idle." << std::endl;

		}

		std::string IdleState::getCurrentActivity(const Pet* pet) const
		{
			return pet->getName() + " is idling around...";
		}

	}
}
