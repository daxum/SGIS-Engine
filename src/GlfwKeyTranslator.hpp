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
			case GLFW_KEY_ESCAPE: return Key::ESCAPE; break;
			case GLFW_KEY_A: return Key::A; break;
			case GLFW_KEY_D: return Key::D; break;
			case GLFW_KEY_S: return Key::S; break;
			case GLFW_KEY_W: return Key::W; break;
			case GLFW_KEY_UP: return Key::UP_ARROW; break;
			case GLFW_KEY_DOWN: return Key::DOWN_ARROW; break;
			case GLFW_KEY_RIGHT: return Key::RIGHT_ARROW; break;
			case GLFW_KEY_LEFT: return Key::LEFT_ARROW; break;
			default: return Key::UNKNOWN;
		}
	}
}
