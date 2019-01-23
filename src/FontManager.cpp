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

std::shared_ptr<ModelRef> FontManager::createTextModel(const std::string& fontName, const std::u32string& text, const std::string& shader, const std::string& buffer, const std::string& uniformSet) {
	std::string meshName = getMeshName(fontName, text, buffer);

	if (!modelManager.hasMesh(meshName)) {
		createTextMesh(fontName, text, buffer);
	}

	std::string modelName = meshName + shader + "_mdl__";

	if (!modelManager.hasModel(modelName)) {
		const Font& font = fontMap.at(fontName);

		Model model(modelName, meshName, shader, uniformSet, modelManager.getMemoryManager()->getUniformSet(uniformSet), true);
		model.textures.push_back(font.getTexture());

		modelManager.addModel(modelName, std::move(model));
	}

	return modelManager.getModel(modelName);
}

void FontManager::createTextMesh(const std::string& fontName, const std::u32string& text, const std::string& buffer) {
	float xPos = 0.0f;
	float yPos = 0.0f;

	const Font& font = fontMap.at(fontName);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VertexBuffer& vertexBuffer = modelManager.getMemoryManager()->getBuffer(buffer);

	float farthestX = 0.0f;
	float lowestY = 0.0f;

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

		Vertex vertex = vertexBuffer.getVertex();

		//Top left
		vertex.setVec2(VERTEX_ELEMENT_POSITION, glm::vec2(xPos + data.bearing.x, baseline + data.bearing.y));
		vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(data.fPos.x, data.fPos.y));
		vertices.push_back(vertex);
		//Top right
		vertex.setVec2(VERTEX_ELEMENT_POSITION, glm::vec2(xPos + data.bearing.x + data.size.x, baseline + data.bearing.y));
		vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(data.fPos.z, data.fPos.y));
		vertices.push_back(vertex);
		//Bottom left
		vertex.setVec2(VERTEX_ELEMENT_POSITION, glm::vec2(xPos + data.bearing.x, baseline - (data.size.y - data.bearing.y)));
		vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(data.fPos.x, data.fPos.w));
		vertices.push_back(vertex);
		//Bottom right
		vertex.setVec2(VERTEX_ELEMENT_POSITION, glm::vec2(xPos + data.bearing.x + data.size.x, baseline - (data.size.y - data.bearing.y)));
		vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(data.fPos.z, data.fPos.w));
		vertices.push_back(vertex);

		//Add indices.

		indices.push_back(0 + indexStart);
		indices.push_back(2 + indexStart);
		indices.push_back(3 + indexStart);
		indices.push_back(0 + indexStart);
		indices.push_back(3 + indexStart);
		indices.push_back(1 + indexStart);

		xPos += data.advance;
		lowestY = std::min(baseline - (data.size.y - data.bearing.y), lowestY);
	}

	float radius = glm::length(glm::vec2(farthestX, lowestY)) / 2.0f;
	Aabb<float> box(glm::vec3(0.0, lowestY, -0.01), glm::vec3(farthestX, 0.0, 0.01));

	modelManager.addMesh(getMeshName(fontName, text, buffer), Mesh(buffer, vertexBuffer.getVertexFormat(), vertices, indices, box, radius));
}

std::string FontManager::getMeshName(const std::string& fontName, const std::u32string& text, const std::string& buffer) const {
	std::string out = "__fnt_mng_mesh_";
	out += fontName + "_";

	//I wouldn't try to print this
	const unsigned char* textPtr = (const unsigned char*) text.data();

	for (size_t i = 0; i < text.size() * 4; i++) {
		out += textPtr[i];
	}

	out += std::string("_") + buffer + "__";

	return out;
}
