#pragma once
#include "IState.h"
#include <iostream>

namespace PetGame {
	namespace DigiPet {
		const int TICKS_TO_FINISH_EATING = 10;
		class FeedingState :
			public IState
		{
		public:
			FeedingState();
			~FeedingState() override;

			void enter(Pet* pet, int tick) override;
			void update(Pet* pet, float deltaTime, int tick) override;
			void leave(Pet* pet, int tick) override;

			std::string getCurrentActivity(const Pet* pet) const override;

		private:
			int m_startedFeedingTick;
		};
	}

}

