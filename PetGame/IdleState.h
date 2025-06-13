#pragma once
#include "IState.h"
namespace PetGame {
	namespace DigiPet {
		const int TICKS_TO_HUNGER = 10;
		class IdleState :
			public IState
		{
		public:
			IdleState();
			~IdleState() override;

			void enter( Pet* pet, int tick= NULL) override;
			void update( Pet* pet, float deltaTime, int tick) override;
			void leave( Pet* pet, int tick = NULL) override;

			std::string getCurrentActivity(const Pet* pet) const override;

		private:
			int m_lastHungerTick;

		};

	}
}

