#include "FeedingState.h"
#include "DigiPet.h"
#include "IdleState.h"

namespace PetGame {
	namespace DigiPet {
		FeedingState::FeedingState() : m_startedFeedingTick(0)
		{
		}

		FeedingState::~FeedingState()
		{
		}

		void FeedingState::enter(Pet* pet, int tick)
		{
			std::cout << pet->getName() << " is Eating in tick:" << tick << std::endl;
			m_startedFeedingTick = tick;
		}

		void FeedingState::update(Pet* pet, float deltaTime, int tick)
		{
			std::cout << tick << "||" << m_startedFeedingTick << std::endl;
			int ticksFeeding = tick - m_startedFeedingTick;
			if (ticksFeeding >= TICKS_TO_FINISH_EATING) {
				pet->ChangeState(new IdleState(), tick);
			}
			else {
				std::cout << pet->getName() << " Eat some" << std::endl;
				int currentHunger = pet->getHunger();
				pet->setHunger(currentHunger - 3);
			}
		}

		void FeedingState::leave(Pet* pet, int tick = NULL)
		{
			std::cout << "Finished eating" << std::endl;
		}

		std::string FeedingState::getCurrentActivity(const Pet* pet) const
		{
			return pet->getName() + " is busy eating.";
		}
	}

}
