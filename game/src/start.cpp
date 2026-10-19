#include "../include/MainStateClass.hpp"
#include <BaseStateClass.hpp>

int main() {
	TAGBaseState::initGame(TAGBaseState::GameInitializer(1280, 720, "Boids Galaxy"));

	TAGBaseState::addState<MainState>("MAIN");

	return TAGBaseState::runGame();
}