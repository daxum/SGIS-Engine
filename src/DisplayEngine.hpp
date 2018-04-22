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

#pragma once

#include <stack>
#include <vector>
#include <memory>
#include <unordered_map>

#include "RenderingEngine.hpp"
#include "KeyList.hpp"
#include "Screen.hpp"

class DisplayEngine {
public:
	/**
	 * Constructor
	 */
	DisplayEngine();

	/**
	 * Destructor
	 */
	~DisplayEngine() {}

	/**
	 * Pushes a new screen onto the stack, masking all screens below it (so they won't be
	 * rendered or updated, and won't receive input). To add a screen without masking others,
	 * see pushOverlay.
	 * @param screen The screen to push onto the stack.
	 */
	void pushScreen(std::shared_ptr<Screen> screen);

	/**
	 * Pops the top overlay stack from the screen stack, effectively closing all visible screens.
	 */
	void popScreen();

	/**
	 * Pushes a screen above the current top screen. The new screen will be rendered and updated
	 * along with the old one, but will always be drawn on top and will receive input events first.
	 * @param overlay The new screen to add to the current overlay stack.
	 */
	void pushOverlay(std::shared_ptr<Screen> overlay);

	/**
	 * Pops the top screen on the current overlay stack. Unlike popScreen, this will only ever
	 * effect one screen, whichever was on the top of the overlay stack.
	 */
	void popOverlay();

	/**
	 * Returns the screen on the top of the current overlay stack.
	 * @return The top of the current overlay stack.
	 */
	std::shared_ptr<Screen> getTop();

	/**
	 * Updates all active screens (all in the current overlay stack),
	 * from bottom to top.
	 * Only intended to be called from the Engine, not from screens.
	 * Possibly make this private and have a friend class later?
	 */
	void update();

	/**
	 * Renders all screens in the overlay stack, from bottom to top.
	 * @param partialTicks The time between game engine ticks.
	 */
	void render(float partialTicks);

	/**
	 * Called by the engine to set the rendering engine.
	 */
	void setRenderer(std::shared_ptr<RenderingEngine> newRenderer);

	/**
	 * Returns whether the screen stack is empty, which should only happen when the game
	 * wants to exit.
	 * @return whether to stop the engine.
	 */
	bool shouldExit();

	/**
	 * Called from the rendering engine's callback whenever a key is pressed.
	 * @param key The key that was pressed.
	 * @param action What the key did.
	 */
	void onKeyAction(Key key, KeyAction action);

	/**
	 * Called by rendering engine when the mouse is moved.
	 * @param x New mouse x.
	 * @param y New mouse y.
	 */
	void onMouseMove(float x, float y);

private:
	//Basically a stack of stacks, the first stack contains the actual screen stack,
	//and the second contains all screens that are currently being rendered.
	//The outer stack is referred to above as the "screen stack", and the inner one
	//(which is actually a vector) as the "overlay stack".
	std::stack<std::vector<std::shared_ptr<Screen>>> screenStack;

	//Set when popScreen is called during updating, breaks out of the update loop
	//to avoid updating invalid screens.
	bool popped;

	//All input events for this display, dispatched to all screen's input handlers once per tick.
	std::vector<std::shared_ptr<InputEvent>> events;

	//Rendering engine, needed for mouse hiding.
	std::shared_ptr<RenderingEngine> renderer;
};
