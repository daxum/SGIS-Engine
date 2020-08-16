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

#include "GlfwInterface.hpp"
#include "RenderingEngine.hpp"
#include "Input/GlfwKeyTranslator.hpp"
#include "Display/WindowSizeEvent.hpp"

GlfwInterface::GlfwInterface(DisplayEngine& display, RenderingEngine* renderer) :
	display(display),
	renderer(renderer),
	window(nullptr),
	width(0.0f),
	height(0.0f) {

	glfwSetErrorCallback(GlfwInterface::glfwError);
}

void GlfwInterface::init(GLFWwindow* newWindow) {
	window = newWindow;
	glfwSetWindowUserPointer(window, this);

	//Set callbacks
	glfwSetFramebufferSizeCallback(window, GlfwInterface::setViewport);
	glfwSetKeyCallback(window, GlfwInterface::keyPress);
	glfwSetCursorPosCallback(window, GlfwInterface::mouseMove);
	glfwSetMouseButtonCallback(window, GlfwInterface::mouseClick);
	glfwSetScrollCallback(window, GlfwInterface::mouseScroll);

	//Set initial size
	int nWidth = 0;
	int nHeight = 0;

	glfwGetFramebufferSize(window, &nWidth, &nHeight);

	width = nWidth;
	height = nHeight;
}

void GlfwInterface::captureMouse(bool capture) const {
	glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

glm::vec2 GlfwInterface::queryMousePos() const {
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	return glm::vec2(x, y);
}

KeyAction GlfwInterface::queryKey(Key::KeyEnum key) const {
	int keyCode = GLFWKeyTranslator::toGlfw(key);
	int state = glfwGetKey(window, keyCode);

	switch (state) {
		case GLFW_PRESS: return KeyAction::PRESS;
		case GLFW_RELEASE: return KeyAction::RELEASE;
		default: throw std::runtime_error("GLFW returned something that wasn't press or release!");
	}
}

void GlfwInterface::glfwError(int error, const char* description) {
	throw std::runtime_error("GLFW error! " + std::to_string(error) + ": " + description);
}

void GlfwInterface::setViewport(GLFWwindow* window, int nWidth, int nHeight) {
	GlfwInterface* interface = (GlfwInterface*) glfwGetWindowUserPointer(window);

	interface->width = (float) nWidth;
	interface->height = (float) nHeight;

	interface->renderer->setViewport(nWidth, nHeight);
	interface->display.getEventQueue().onEvent(std::make_shared<WindowSizeEvent>(nWidth, nHeight));
}

void GlfwInterface::keyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
	GlfwInterface* interface = (GlfwInterface*) glfwGetWindowUserPointer(window);
	KeyAction nativeAction;

	switch (action) {
		case GLFW_PRESS: nativeAction = KeyAction::PRESS; break;
		case GLFW_REPEAT: nativeAction = KeyAction::REPEAT; break;
		case GLFW_RELEASE: nativeAction = KeyAction::RELEASE; break;
		default: return;
	}

	interface->display.getEventQueue().onEvent(std::make_shared<KeyEvent>(GLFWKeyTranslator::fromGlfw(key), nativeAction));
}

void GlfwInterface::mouseMove(GLFWwindow* window, double x, double y) {
	GlfwInterface* interface = (GlfwInterface*) glfwGetWindowUserPointer(window);
	interface->display.getEventQueue().onEvent(std::make_shared<MouseMoveEvent>((float)x, (float)y));
}

void GlfwInterface::mouseClick(GLFWwindow* window, int button, int action, int mods) {
	GlfwInterface* interface = (GlfwInterface*) glfwGetWindowUserPointer(window);
	MouseButton pressed;
	MouseAction mouseAction;

	switch(button) {
		case GLFW_MOUSE_BUTTON_LEFT: pressed = MouseButton::LEFT; break;
		case GLFW_MOUSE_BUTTON_MIDDLE: pressed = MouseButton::MIDDLE; break;
		case GLFW_MOUSE_BUTTON_RIGHT: pressed = MouseButton::RIGHT; break;
		default: return;
	}

	switch(action) {
		case GLFW_PRESS: mouseAction = MouseAction::PRESS; break;
		case GLFW_RELEASE: mouseAction = MouseAction::RELEASE; break;
		default: return;
	}

	interface->display.getEventQueue().onEvent(std::make_shared<MouseClickEvent>(pressed, mouseAction));
}

void GlfwInterface::mouseScroll(GLFWwindow* window, double x, double y) {
	GlfwInterface* interface = (GlfwInterface*) glfwGetWindowUserPointer(window);
	interface->display.getEventQueue().onEvent(std::make_shared<MouseScrollEvent>((float)x, (float)y));
}
