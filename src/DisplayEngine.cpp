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
	pushOverlay(screen);
}

void DisplayEngine::popScreen() {
	screenStack.pop();
	popped = true;

	if (!screenStack.empty() && !screenStack.top().empty()) {
		renderer->captureMouse(getTop()->mouseHidden());
	}
}

void DisplayEngine::pushOverlay(std::shared_ptr<Screen> overlay) {
	screenStack.top().push_back(overlay);

	renderer->captureMouse(overlay->mouseHidden());
}

void DisplayEngine::popOverlay() {
	screenStack.top().pop_back();

	if (!screenStack.top().empty()) {
		renderer->captureMouse(getTop()->mouseHidden());
	}
}

std::shared_ptr<Screen> DisplayEngine::getTop() {
	return screenStack.top().back();
}

void DisplayEngine::update() {
	if (screenStack.empty()) {
		return;
	}

	//Update the overlay stack from back to front.
	//This cannot be range-based, because modifying the
	//container you're looping over tends to behave strangely,
	//due to the iterators not getting updated.
	for (size_t i = screenStack.top().size(); i > 0; i--) {
		//If the overlay stack got popped more than once last update, i can become invalid.
		if (i > screenStack.top().size()) {
			i = screenStack.top().size();
		}

		//Might move into screen later.
		screenStack.top()[i - 1]->getInputHandler().update(events);
		screenStack.top()[i - 1]->update();

		//If the screen stack was popped in the last update, screenStack.top() now points
		//to a different overlay stack, so we should stop updating for this tick.
		if (popped) {
			break;
		}
	}

	popped = false;
	events.clear();
}

void DisplayEngine::render(float partialTicks) {
	if (screenStack.empty()) {
		return;
	}

	//Render all screens in the overlay stack from botom to top.
	//Clear the depth and stencil buffers after each one so they
	//don't effect each other's rendering.
	for (std::shared_ptr<Screen> screen : screenStack.top()) {
		renderer->render(screen->getRenderData(), screen->getCamera());
		renderer->clearBuffers();
	}

	renderer->present();
}

void DisplayEngine::setRenderer(std::shared_ptr<RenderingEngine> newRenderer) {
	renderer = newRenderer;
}

bool DisplayEngine::shouldExit() {
	return screenStack.empty();
}

void DisplayEngine::onKeyAction(Key key, KeyAction action) {
	events.push_back(std::make_shared<KeyEvent>(key, action));
}

void DisplayEngine::onMouseMove(float x, float y) {
	events.push_back(std::make_shared<MouseMoveEvent>(x, y));
}
