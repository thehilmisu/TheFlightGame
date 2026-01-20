#include "game.h"
#include "window.h"
#include "gui.h"
// #include <AL/al.h>

namespace gobjs = gameobjects;

namespace game {
	game::GameMode mainMenu()
	{
		Window& window = Window::getInstance();
		Gui& gui = Gui::getInstance();
		
		window.setCursorInputMode(GLFW_CURSOR_NORMAL);

		window.getCamera().pitch = 0.0f;
		window.getCamera().yaw = 0.0f;
		window.getCamera().position = glm::vec3(0.0f);

		gobjs::Player player(glm::vec3(14.0f, -8.0f, -40.0f));
		
		float dt = 0.0f;
		float totalTime = 0.0f;
		while(!window.shouldClose()) {
			float start = glfwGetTime();

			gui.newFrame();

			//Update perspective matrix
			window.updatePerspectiveMat(FOVY, ZNEAR, ZFAR, window.getWidth(), window.getHeight());

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			gfx::displayPlayerPlane(totalTime, player.transform);
			//Display skybox
			gfx::displaySkybox();

			game::GameMode selected = gui.drawMainMenu();

			player.transform.rotation.y += 2.0f * dt;

			gui.render();

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
				break;
			default:
				break;
			}

			dt = glfwGetTime() - start;
		}

		return NONE_SELECTED;
	}

	// void highScoreTableScreen(const HighScoreTable &highscores)
	// {tty-cmd{"ok": true, "data": "18"}\x
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
