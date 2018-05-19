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

#include <vector>
#include <cstdint>

#include "FontManager.hpp"
#include "Vertex.hpp"

Font& FontManager::addFont(const std::string& name, int spaceWidth, size_t size) {
	fontMap.insert({name, Font(name, spaceWidth, size)});
	return fontMap.at(name);
}

Model FontManager::createTextModel(const std::string& fontName, const std::u32string& text, const std::string& shader) {
	float xPos = 0.0f;
	float yPos = 0.0f;

	const Font& font = fontMap.at(fontName);

	std::vector<TextVertex> vertices;
	std::vector<uint32_t> indices;

	float farthestX = 0.0f;

	for (char32_t c : text) {
		//Handle space and newline
		if (c == U' ') {
			xPos += font.getSpaceWidth();
			continue;
		}

		if (c == U'\n') {
			yPos -= font.getSize();
			xPos = 0.0f;
			continue;
		}

		const GlyphData& data = font.getChar(c);
		const float baseline = yPos - font.getSize();

		farthestX = std::max(farthestX, xPos + data.bearing.x + data.size.x);
		uint32_t indexStart = vertices.size();

		//Place vertices.

		//Bottom left
		vertices.push_back({glm::vec2(xPos + data.bearing.x, baseline + data.bearing.y), glm::vec2(data.fPos.x, data.fPos.y)});
		//Bottom right
		vertices.push_back({glm::vec2(xPos + data.bearing.x + data.size.x, baseline + data.bearing.y), glm::vec2(data.fPos.z, data.fPos.y)});
		//Top left
		vertices.push_back({glm::vec2(xPos + data.bearing.x, baseline - (data.size.y - data.bearing.y)), glm::vec2(data.fPos.x, data.fPos.w)});
		//Top right
		vertices.push_back({glm::vec2(xPos + data.bearing.x + data.size.x, baseline - (data.size.y - data.bearing.y)), glm::vec2(data.fPos.z, data.fPos.w)});

		//Add indices.

		indices.push_back(3 + indexStart);
		indices.push_back(2 + indexStart);
		indices.push_back(0 + indexStart);
		indices.push_back(1 + indexStart);
		indices.push_back(3 + indexStart);
		indices.push_back(0 + indexStart);

		xPos += data.advance;
	}

	yPos -= font.getSize();

	//Temporary, fix models later.
	LightInfo fontLight = {
		glm::vec3(1.0, 1.0, 1.0),
		glm::vec3(1.0, 1.0, 1.0),
		1.0f
	};

	float radius = glm::length(glm::vec2(farthestX, yPos)) / 2.0f;
	AxisAlignedBB box(glm::vec3(0.0, yPos, -0.01), glm::vec3(farthestX, 0.0, 0.01));

	MeshRenderData modelData = renderer->getMemoryManager()->addTextMesh(vertices, indices);
	Model textModel(modelData, box, radius, font.getTexture(), shader, fontLight);

	return textModel;
}

void FontManager::freeTextModel(const Model& model) {
	renderer->getMemoryManager()->freeTextMesh(model.mesh);
}
