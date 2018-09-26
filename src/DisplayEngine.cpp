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
#include "RenderingEngine.hpp"
#include "Screen.hpp"
#include "Camera.hpp"
#include "Engine.hpp"

DisplayEngine::DisplayEngine() :
	popped(false),
	stackChanged(false) {

}

void DisplayEngine::pushScreen(std::shared_ptr<Screen> screen) {
	screenStack.emplace_back();
	pushOverlay(screen);
}

void DisplayEngine::popScreen() {
	screenStack.pop_back();

	popped = true;
	stackChanged = true;

	if (!screenStack.empty() && !screenStack.back().empty()) {
		renderer->getWindowInterface().captureMouse(getTop()->mouseHidden());
	}
}

void DisplayEngine::pushOverlay(std::shared_ptr<Screen> overlay) {
	if (screenStack.empty()) {
		screenStack.emplace_back();
	}

	screenStack.back().push_back(overlay);

	renderer->getWindowInterface().captureMouse(overlay->mouseHidden());
	stackChanged = true;
}

void DisplayEngine::popOverlay() {
	screenStack.back().pop_back();

	if (!screenStack.back().empty()) {
		renderer->getWindowInterface().captureMouse(getTop()->mouseHidden());
	}

	stackChanged = true;
}

std::shared_ptr<Screen> DisplayEngine::getTop() {
	return screenStack.back().back();
}

void DisplayEngine::update() {
	if (screenStack.empty()) {
		return;
	}

	//Update the overlay stack from back to front.
	//This cannot be range-based, because modifying the
	//container you're looping over tends to behave strangely,
	//due to the iterators not getting updated.
	for (size_t i = screenStack.back().size(); i > 0; i--) {
		//If the overlay stack got popped more than once last update, i can become invalid.
		if (i > screenStack.back().size()) {
			i = screenStack.back().size();
		}

		std::shared_ptr<Screen> current = screenStack.back()[i - 1];

		//Might move into screen later.
		current->getInputHandler().update(events);
		current->update();

		//If the screen stack was popped in the last update, screenStack.top() now points
		//to a different overlay stack, so we should stop updating for this tick.
		if (popped) {
			break;
		}
	}

	events.clear();

	if (stackChanged) {
		//Update new top screen's mouse position by sending extra mouse move event.
		glm::vec2 mousePos = renderer->getWindowInterface().queryMousePos();
		onMouseMove(mousePos.x, mousePos.y);
		stackChanged = false;
	}

	popped = false;
}

void DisplayEngine::render(float partialTicks) {
	if (screenStack.empty()) {
		return;
	}

	renderer->beginFrame();

	//Render all screens in the overlay stack from botom to top.
	for (std::shared_ptr<Screen> screen : screenStack.back()) {
		renderer->render(screen->getRenderData(), screen->getCamera(), screen->getState());
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

void DisplayEngine::onMouseClick(MouseButton button, MouseAction action) {
	events.push_back(std::make_shared<MouseClickEvent>(button, action));
}

void DisplayEngine::onMouseScroll(float x, float y) {
	events.push_back(std::make_shared<MouseScrollEvent>(x, y));
}

void DisplayEngine::updateProjections() {
	for (std::vector<std::shared_ptr<Screen>>& overlay : screenStack) {
		for (std::shared_ptr<Screen>& screen : overlay) {
			screen->getCamera()->setProjection();
		}
	}
}
