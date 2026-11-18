#include <states/MainStateClass.hpp>
#include <BaseStateClass.hpp>

int main() {
	TAGBaseState::initGame(TAGBaseState::GameInitializer(1280, 720, "Boids Galaxy"));

	TAGBaseState::far = 1000.0f;
	TAGResourceManager::asset_path = "../../../game/assets/";

	TAGBaseState::addState<MainState>("MAIN");

	return TAGBaseState::runGame();
}