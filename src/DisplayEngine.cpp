/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "DisplayEngine.hpp"
#include "Screen.hpp"

DisplayEngine::DisplayEngine() :
	popped(false) {

}

void DisplayEngine::pushScreen(std::shared_ptr<Screen> screen) {
	screenStack.emplace();
	screenStack.top().push_back(screen);
}

void DisplayEngine::popScreen() {
	screenStack.pop();
	popped = true;
}

void DisplayEngine::pushOverlay(std::shared_ptr<Screen> overlay) {
	screenStack.top().push_back(overlay);
}

void DisplayEngine::popOverlay() {
	screenStack.top().pop_back();
}

std::shared_ptr<Screen> DisplayEngine::getTop() {
	return screenStack.top().back();
}

void DisplayEngine::update() {
	if (screenStack.empty()) {
		return;
	}

	//Update the overlay stack from bottom to top.
	//This cannot be range-based, because modifying the
	//container you're looping over tends to behave strangely,
	//due to the iterators not getting updated.
	for (size_t i = 0; i < screenStack.top().size(); i++) {
		screenStack.top()[i]->update();

		//If the screen stack was popped in the last update, screenStack.top() now points
		//to a different overlay stack, so we should stop updating for this tick.
		if (popped) {
			break;
		}
	}

	popped = false;
}

void DisplayEngine::render(float partialTicks, std::shared_ptr<RenderingEngine> renderer) {
	if (screenStack.empty()) {
		return;
	}

	//Render all screens in the overlay stack from botom to top.
	//Clear the depth and stencil buffers after each one so they
	//don't effect each other's rendering.
	for (std::shared_ptr<Screen> screen : screenStack.top()) {
		renderer->render(screen->getRenderData(), partialTicks);
		renderer->clearBuffers();
	}

	renderer->present();
}

bool DisplayEngine::shouldExit() {
	return screenStack.empty();
}

void DisplayEngine::onKeyAction(Key key, KeyAction action) {
	//If pressed, notify all relevent screens and update key map.
	if (action == KeyAction::PRESS) {
		keyMap[key] = true;

		if (screenStack.empty()) {
			return;
		}

		//Iterate from the top screen to the bottom one so they can properly claim inputs
		//The loop has weird parameters to prevent unsigned underflow.
		for (size_t i = screenStack.top().size(); i >= 1; i--) {
			bool stop = screenStack.top()[i - 1]->onKeyPressed(key);

			//The screen stack could have been popped here as well.
			if (stop || popped) {
				break;
			}
		}

		popped = false;
	}
	//If released, just update key map.
	else if (action == KeyAction::RELEASE) {
		keyMap[key] = false;
	}

	//No action for repeated key presses (KeyAction::REPEAT, eg. the key is held down and gets multiple press events) for now.
}

bool DisplayEngine::isKeyPressed(Key key) {
	//Theoretically, operator[] default-initializes missing members for maps, but
	//not sure if it can be trusted to consistently initialize booleans to false.
	//(Old MSVC might initialize to true?)
	if (keyMap.count(key) == 0) {
		keyMap[key] = false;
	}

	return keyMap[key];
}
