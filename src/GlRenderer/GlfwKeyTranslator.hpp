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

#include "KeyList.hpp"
#include "CombinedGl.h"

namespace GLFWKeyTranslator {
	/**
	 * Stateless function to translate a key from GLFW to one in KeyList.hpp.
	 * @param key A key code from glfw.
	 * @return A key from KeyList.hpp.
	 */
	inline Key translate(int key) {
		switch (key) {
			case GLFW_KEY_ESCAPE: return Key::ESCAPE;
			case GLFW_KEY_A: return Key::A;
			case GLFW_KEY_D: return Key::D;
			case GLFW_KEY_E: return Key::E;
			case GLFW_KEY_Q: return Key::Q;
			case GLFW_KEY_R: return Key::R;
			case GLFW_KEY_S: return Key::S;
			case GLFW_KEY_W: return Key::W;
			case GLFW_KEY_UP: return Key::UP_ARROW;
			case GLFW_KEY_DOWN: return Key::DOWN_ARROW;
			case GLFW_KEY_RIGHT: return Key::RIGHT_ARROW;
			case GLFW_KEY_LEFT: return Key::LEFT_ARROW;
			case GLFW_KEY_SPACE: return Key::SPACE;
			case GLFW_KEY_ENTER: return Key::ENTER;
			case GLFW_KEY_LEFT_SHIFT: return Key::LEFT_SHIFT;
			default: return Key::UNKNOWN;
		}
	}
}
