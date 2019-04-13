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
#include "Models/Vertex.hpp"

Font& FontManager::addFont(const std::string& name, int spaceWidth, size_t size) {
	fontMap.emplace(name, Font(name, spaceWidth, size));
	return fontMap.at(name);
}

Model FontManager::createTextModel(const TextMeshInfo& meshInfo, const std::string& material) {
	std::string meshName = getMeshName(meshInfo);

	if (!modelManager.hasMesh(meshName)) {
		createTextMesh(meshInfo);
	}

	return modelManager.getModel(material, meshName);
}

void FontManager::createTextMesh(const TextMeshInfo& meshInfo) {
	float xPos = 0.0f;
	float yPos = 0.0f;

	const Font& font = fontMap.at(meshInfo.font);
	const VertexFormat* textFormat = modelManager.getFormat(meshInfo.format);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	float farthestX = 0.0f;
	float lowestY = 0.0f;

	for (char32_t c : meshInfo.text) {
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

		Vertex vertex(textFormat);

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

	Mesh::BufferInfo bufferInfo = {};
	bufferInfo.vertexName = meshInfo.vertexBuffer;
	bufferInfo.indexName = meshInfo.indexBuffer;
	bufferInfo.vertex = modelManager.getMemoryManager()->getBuffer(bufferInfo.vertexName);
	bufferInfo.index = modelManager.getMemoryManager()->getBuffer(bufferInfo.indexName);

	modelManager.addMesh(getMeshName(meshInfo), Mesh(bufferInfo, textFormat, vertices, indices, box, radius), false);
}

std::string FontManager::getMeshName(const TextMeshInfo& meshInfo) const {
	std::string out = "__fmsh" + meshInfo.font;

	//I wouldn't try to print this
	const unsigned char* textPtr = (const unsigned char*) meshInfo.text.data();

	for (size_t i = 0; i < meshInfo.text.size() * 4; i++) {
		out += textPtr[i];
	}

	out += meshInfo.vertexBuffer + meshInfo.indexBuffer + meshInfo.format;

	return out;
}
