#pragma once 
#include <string>

namespace PetGame {
	namespace DigiPet {
		class Pet;
	}
}

namespace PetGame {
	namespace DigiPet {
		class IState
		{
		public:
			virtual ~IState() = default;

			virtual void enter(Pet* pet, int tick = NULL) = 0;
			virtual void update(Pet* pet, float deltaTime, int tick) = 0;
			virtual void leave(Pet* pet, int tick = NULL) = 0;

			virtual std::string getCurrentActivity(const Pet* pet) const = 0;
		};
	}
}
