#pragma once  

#include <string>  
#include <iostream>
#include "IState.h"
#include "IdleState.h"
#include "FeedingState.h"
#include "Texture2D.h"
#include "glm/glm.hpp"
#include <map>
#include <memory>

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
			void UpdateTick(float deltaTime, int tickCount);
			void UpdateRender(float deltaTime);
			void ChangeState(IState* newState, int tickCount);
			
			/* Actions*/
			void feed(int tick);
			void train(const int hours);

			/* Getters*/
			std::string getName() const { return m_name; };
			int getHunger() const { return m_hunger; };
			int getXp() const { return m_experience; };
			std::string getLevel() const;
			Texture2D* getTexture() const;
			glm::vec2 getPosition() const { return m_position; };
			glm::vec2 getSize() const  { return m_size; };
			float getRotation() const { return m_rotation; };
			glm::vec3 getColorTint() const { return m_colorTint; };


			/* Setters*/
			void setHunger(int value);
			void setXp(int value);
			void setTextures();

			/*Debug*/
			void displayStatus() const;

			/*Actions*/
			void hurt(int tick);
			void hurting(int tick);

		private:
			std::string m_name;
			int m_hunger;
			int m_experience;
			Level m_level;

			glm::vec2 m_position;
			glm::vec2 m_size;
			float m_rotation;
			glm::vec3 m_colorTint;

			std::map<Level, std::unique_ptr<Texture2D>> m_textures;

			IState* m_currentState;

			bool m_IsHurting = false;
			int m_statedHurting = 0;
		};
	}
}
