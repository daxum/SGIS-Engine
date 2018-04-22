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

#include "KeyList.hpp"
#include "MouseList.hpp"

enum class EventType {
	KEY,
	MOUSE_MOVE,
	MOUSE_CLICK
};

//The base event type. Use the type variable to cast to the appropriate event structure.
struct InputEvent {
protected:
	InputEvent(EventType t) : type(t) {}

public:
	virtual ~InputEvent() {}

	const EventType type;
};

//Keyboard event. Holds the key and the action (press, repeat, release).
struct KeyEvent : public InputEvent {
	KeyEvent(Key k, KeyAction a) : InputEvent(EventType::KEY), key(k), action(a) {}

	const Key key;
	const KeyAction action;
};

//Mouse moved. Holds the distance moved from the last mouse position.
struct MouseMoveEvent : public InputEvent {
	MouseMoveEvent(float x, float y) : InputEvent(EventType::MOUSE_MOVE), x(x), y(y) {}

	const float x;
	const float y;
};

//Mouse click. Holds the button pressed and whether the action was a press or release.
struct MouseClickEvent : public InputEvent {
	MouseClickEvent(MouseButton b, MouseAction a) : InputEvent(EventType::MOUSE_CLICK), button(b), action(a) {}

	const MouseButton button;
	const MouseAction action;
};
