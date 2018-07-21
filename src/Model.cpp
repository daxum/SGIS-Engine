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

#include "Model.hpp"
#include "ModelManager.hpp"

Mesh::Mesh(const std::string& buffer, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const AxisAlignedBB& box, float radius) :
	vertexData(new unsigned char[vertices.at(0).getSize() * vertices.size()]),
	vertexSize(vertices.at(0).getSize() * vertices.size()),
	indices(indices),
	buffer(buffer),
	box(box),
	radius(radius),
	users(0) {

	size_t vertSize = vertices.at(0).getSize();
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
	buffer(mesh.buffer),
	box(mesh.box),
	radius(mesh.radius),
	users(mesh.users) {

	memcpy(vertexData, mesh.vertexData, mesh.vertexSize);
}

Mesh::Mesh(Mesh&& mesh) :
	vertexData(std::exchange(mesh.vertexData, nullptr)),
	vertexSize(std::exchange(mesh.vertexSize, 0)),
	indices(std::move(mesh.indices)),
	buffer(std::move(mesh.buffer)),
	box(std::move(mesh.box)),
	radius(std::exchange(mesh.radius, 0.0f)),
	users(std::exchange(mesh.users, 0)) {

}


ModelRef::ModelRef(ModelManager* manager, const std::string& modelName, Model& model, Mesh& mesh) :
	manager(manager),
	model(model),
	mesh(mesh),
	modelName(modelName) {

}

ModelRef::~ModelRef() {
	manager->removeReference(modelName);
}
