#include "game.h"
#include "window.h"
// #include <AL/al.h>

namespace gobjs = gameobjects;

namespace game {
	game::GameMode mainMenu()
	{
		Window& window = Window::getInstance();
		bool showCredits = false;
		// std::vector<std::string> credits = gui::readTextFile("assets/credit cs; c

		window.setCursorInputMode(GLFW_CURSOR_NORMAL);
		// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		window.getCamera().pitch = 0.0f;
		window.getCamera().yaw = 0.0f;
		window.getCamera().position = glm::vec3(0.0f);

		gobjs::Player player(glm::vec3(14.0f, -8.0f, -40.0f));
		
		float dt = 0.0f;
		float totalTime = 0.0f;
		while(!window.shouldClose()) {
			float start = glfwGetTime();


			//Update perspective matrix
			window.updatePerspectiveMat(FOVY, ZNEAR, ZFAR, window.getWidth(), window.getHeight());

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			gfx::displayPlayerPlane(totalTime, player.transform);
			//Display skybox
			gfx::displaySkybox();

			// game::GameMode selected = gui::displayMainMenu();
			game::GameMode selected = NONE_SELECTED;
			// if(showCredits) {
			// 	bool close = gui::displayCredits(credits);
			// 	if(close)
			// 		showCredits = false;
			// }

			player.transform.rotation.y += 3.14f * dt;

			window.updateKeyStates();
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			window.swapBuffers();
			glfwPollEvents();
			gfx::outputErrors();
			totalTime += dt;

			switch(selected) {
			case SETTINGS: //Fall through
			case ARCADE:
				return selected;
			case CREDITS:
				showCredits = true;
			default:
				break;
			}

			dt = glfwGetTime() - start;
		}

		return NONE_SELECTED;
	}

	// void highScoreTableScreen(const HighScoreTable &highscores)
	// {
	// 	State* state = State::get();

	// 	glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// 	state->getCamera().pitch = 0.0f;
	// 	state->getCamera().yaw = 0.0f;
	// 	state->getCamera().position = glm::vec3(0.0f);

	// 	bool quit = false;
	// 	while(!glfwWindowShouldClose(state->getWindow()) && !quit) {
	// 		nk_glfw3_new_frame(state->getNkGlfw());

	// 		//Update perspective matrix
	// 		state->updatePerspectiveMat(FOVY, ZNEAR, ZFAR);

	// 		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 		//Display skybox
	// 		gfx::displaySkybox();
	// 		//GUI
	// 		quit = gui::displayHighScores(highscores);

	// 		state->updateKeyStates();
	// 		nk_glfw3_render(state->getNkGlfw(), NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
	// 		glEnable(GL_CULL_FACE);
	// 		glEnable(GL_DEPTH_TEST);
	// 		glEnable(GL_BLEND);
	// 		glfwSwapBuffers(state->getWindow());
	// 		glfwPollEvents();
	// 		gfx::outputErrors();
	// 	}
	// }

	// void settingsScreen()
	// {
	// 	State* state = State::get();
	// 	SettingsValues values = GlobalSettings::get()->values;
	// 	game::SettingsScreenAction action = game::SETTINGS_NONE_SELECTED;

	// 	glfwSetInputMode(state->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// 	state->getCamera().pitch = 0.0f;
	// 	state->getCamera().yaw = 0.0f;
	// 	state->getCamera().position = glm::vec3(0.0f);

	// 	bool quit = false;
	// 	while(!glfwWindowShouldClose(state->getWindow()) && !quit) {
	// 		nk_glfw3_new_frame(state->getNkGlfw());

	// 		//Update perspective matrix
	// 		state->updatePerspectiveMat(FOVY, ZNEAR, ZFAR);

	// 		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 		//Display skybox
	// 		gfx::displaySkybox();
	// 		//Display settings
	// 		action = gui::displaySettingsMenu(values);

	// 		state->updateKeyStates();
	// 		nk_glfw3_render(state->getNkGlfw(), NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
	// 		glEnable(GL_CULL_FACE);
	// 		glEnable(GL_DEPTH_TEST);
	// 		glEnable(GL_BLEND);
	// 		glfwSwapBuffers(state->getWindow());
	// 		glfwPollEvents();
	// 		gfx::outputErrors();

	// 		if(action != game::SETTINGS_NONE_SELECTED)
	// 			quit = true;
	// 	}

	// 	if(action == game::SAVE_SETTINGS) {
	// 		GlobalSettings::get()->values = values;
	// 		alListenerf(AL_GAIN, GlobalSettings::get()->values.volume);
	// 	}
	// }
}
