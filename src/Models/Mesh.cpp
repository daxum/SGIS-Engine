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

#include "Mesh.hpp"
#include "ModelManager.hpp"

Mesh::Mesh(BufferInfo bufferInfo, const VertexFormat* format, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Aabb<float>& box, float radius) :
	vertexData(new unsigned char[format->getVertexSize() * vertices.size()]),
	vertexSize(format->getVertexSize() * vertices.size()),
	indices(indices),
	bufferInfo(bufferInfo),
	format(format),
	box(box),
	radius(radius),
	indexStart(0) {

	size_t vertSize = format->getVertexSize();
	size_t offset = 0;

	//Copy over all vertex data
	for (const Vertex& vertex : vertices) {
		memcpy(&vertexData[offset], vertex.getData(), vertSize);
		offset += vertSize;
	}
}

Mesh::Mesh(const Mesh& mesh) :
	vertexData(new unsigned char[mesh.vertexSize]),
	vertexSize(mesh.vertexSize),
	indices(mesh.indices),
	bufferInfo(mesh.bufferInfo),
	format(mesh.format),
	box(mesh.box),
	radius(mesh.radius),
	indexStart(mesh.indexStart) {

	memcpy(vertexData, mesh.vertexData, mesh.vertexSize);
}

Mesh::Mesh(Mesh&& mesh) :
	vertexData(std::exchange(mesh.vertexData, nullptr)),
	vertexSize(std::exchange(mesh.vertexSize, 0)),
	indices(std::move(mesh.indices)),
	bufferInfo(std::move(mesh.bufferInfo)),
	format(std::exchange(mesh.format, nullptr)),
	box(std::move(mesh.box)),
	radius(std::exchange(mesh.radius, 0.0f)),
	indexStart(std::exchange(mesh.indexStart, 0)) {

}

MeshRef::~MeshRef() {
	manager->removeMeshReference(meshName, level);
}
