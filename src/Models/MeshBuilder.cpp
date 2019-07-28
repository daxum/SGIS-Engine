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

#include "MeshBuilder.hpp"

MeshBuilder::MeshBuilder(const VertexFormat* format, size_t maxVerts) :
	format(format),
	numVerts(0),
	hasPos(format->hasElement(VERTEX_ELEMENT_POSITION)),
	radiusSq(0.0f) {

	vertexData.reserve(maxVerts * format->getVertexSize());
}

void MeshBuilder::addVertex(const Vertex& vert) {
	const size_t vertSize = format->getVertexSize();

	if (!uniqueVertices.count(vert)) {
		uniqueVertices[vert] = numVerts;
		vertexData.insert(vertexData.end(), vertSize, 0);
		memcpy(&vertexData[numVerts * vertSize], vert.getData(), vertSize);
		numVerts++;
	}

	indices.push_back(uniqueVertices.at(vert));

	if (hasPos) {
		glm::vec3 pos = vert.getVec3(VERTEX_ELEMENT_POSITION);

		meshBox.min = glm::min(meshBox.min, pos);
		meshBox.max = glm::max(meshBox.max, pos);

		radiusSq = std::max(radiusSq, glm::dot(pos, pos));
	}
}

Mesh MeshBuilder::genMesh(Mesh::BufferInfo bufferInfo) {
	return Mesh(bufferInfo, format, std::move(vertexData), std::move(indices), meshBox, std::sqrt(radiusSq));
}
