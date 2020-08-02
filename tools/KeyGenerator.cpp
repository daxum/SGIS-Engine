/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2020
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

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Usage: ./KeyGenerator <path to glfw header>\n";
		return 1;
	}

	std::ifstream glfwIn(argv[1]);
	std::ofstream headerOut("./header_out.txt");

	headerOut << "namespace Key {\n\tenum KeyEnum {\n";

	std::string transFunc = "Key fromGlfw(int key) {\n\tswitch (key) {\n";
	std::string transRevFunc = "int toGlfw(Key key) {\n\tswitch (key) {\n";

	while (!glfwIn.eof()) {
		std::string line = "";
		getline(glfwIn, line);

		if (line.find("#define GLFW_KEY_") == std::string::npos) {
			continue;
		}

		const size_t end = line.find(" ", 17);
		std::string keyName = line.substr(17, end - 17);

		if (keyName == "LAST") {
			continue;
		}

		std::cout << "Found key \"" + keyName + "\"\n";

		if (keyName != "UNKNOWN") {
			bool addPrefix = std::isdigit(keyName.at(0));
			std::string headerName = (addPrefix ? "NUM_" : "") + keyName;

			headerOut << "\t\t" << headerName << ",\n";
			transFunc += "\t\tcase GLFW_KEY_" + keyName  + ": return Key::" + headerName + ";\n";
			transRevFunc += "\t\tcase Key::" + headerName + ": return GLFW_KEY_" + keyName + ";\n";
		}
	}

	headerOut << "\t\tNUM_KEYS,\n\t\tUNKNOWN\n\t};\n}\n\n";
	transFunc += "\t\tdefault: return Key::UNKNOWN;\n\t}\n}\n\n";
	transRevFunc += "\t\tdefault: return GLFW_KEY_UNKNOWN;\n\t}\n}\n";

	headerOut << transFunc;
	headerOut << transRevFunc;

	return 0;
}
