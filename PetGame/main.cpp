#include <iostream>
#include "Application.h"
#include "DigiPet.h"

enum SCREEN {
	WIDTH = 800,
	HEIGHT = 600
};

int main() {
	PetGame::Application game = PetGame::Application();

	if (game.Init(SCREEN::WIDTH, SCREEN::HEIGHT, "Tamagochi")) {
		game.Start();
	}

	game.Stop();
}
