#pragma once  

#include <string>  
#include <iostream>
#include "IState.h"
#include "IdleState.h"
#include "FeedingState.h"
#include "Texture2D.h"

namespace PetGame {
	namespace DigiPet {
		enum Level {
			Egg = 0,
			Puppy = 1,
			Child = 2,
			Adult = 3,
		};
		class Pet
		{
		public:
			Pet(const std::string& name);

			~Pet();

			/* Game Functions*/
			void Update(float deltaTime, int tickCount);
			void ChangeState(IState* newState, int tickCount);
			
			/* Actions*/
			void feed(int tick);
			void train(const int hours);

			/* Getters*/
			std::string getName() const { return m_name; };
			int getHunger() const { return m_hunger; };
			int getXp() const { return m_experience; };
			std::string getLevel() const;
			const Texture2D& getTexture() const { return m_texture; };

			/* Setters*/
			void setHunger(int value);
			void setXp(int value);
			void setTexture(const char* filePath);

			/*Debug*/
			void displayStatus() const;

		private:
			std::string m_name;
			int m_hunger;
			int m_experience;
			Level m_level;

			Texture2D m_texture;

			IState* m_currentState;
		};
	}
}
