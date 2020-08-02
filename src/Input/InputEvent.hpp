/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018, 2020
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

#include "Events/Event.hpp"
#include "KeyList.hpp"
#include "MouseList.hpp"

//Keyboard event. Holds the key and the action (press, repeat, release).
struct KeyEvent : public Event {
	static constexpr uint64_t EVENT_TYPE = 0;

	KeyEvent(Key::KeyEnum k, KeyAction a) : Event(EVENT_TYPE), key(k), action(a) {}

	Key::KeyEnum key;
	KeyAction action;
};

//Mouse moved. Holds the new mouse position.
struct MouseMoveEvent : public Event {
	static constexpr uint64_t EVENT_TYPE = 1;

	MouseMoveEvent(float x, float y) : Event(EVENT_TYPE), x(x), y(y) {}

	float x;
	float y;
};

//Mouse click. Holds the button pressed and whether the action was a press or release.
struct MouseClickEvent : public Event {
	static constexpr uint64_t EVENT_TYPE = 2;

	MouseClickEvent(MouseButton b, MouseAction a) : Event(EVENT_TYPE), button(b), action(a) {}

	MouseButton button;
	MouseAction action;
};

//Mouse scroll. Holds the amount scrolled in the x and y directions.
struct MouseScrollEvent : public Event {
	static constexpr uint64_t EVENT_TYPE = 3;

	MouseScrollEvent(float x, float y) : Event(EVENT_TYPE), x(x), y(y) {}

	float x;
	float y;
};
