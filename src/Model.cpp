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

namespace {
	//Needed for model's constructor, maybe make member function?
	//Strips out all uniforms that don't belong in the model's uniform buffer.
	std::vector<UniformDescription> stripNonBufferedModel(const UniformSet& uniformSet) {
		std::vector<UniformDescription> out;

		for (const UniformDescription& uniform : uniformSet.uniforms) {
			if (uniform.provider == UniformProviderType::MATERIAL && !isSampler(uniform.type)) {
				out.push_back(uniform);
			}
		}

		return out;
	}
}

Mesh::Mesh(const std::string& buffer, const std::vector<VertexElement>& format, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Aabb<float>& box, float radius) :
	vertexData(new unsigned char[vertices.at(0).getSize() * vertices.size()]),
	vertexSize(vertices.at(0).getSize() * vertices.size()),
	indices(indices),
	buffer(buffer),
	format(format),
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
	format(mesh.format),
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
	format(std::move(mesh.format)),
	box(std::move(mesh.box)),
	radius(std::exchange(mesh.radius, 0.0f)),
	users(std::exchange(mesh.users, 0)) {

}

MeshRef::MeshRef(ModelManager* manager, const std::string& meshName, Mesh& mesh) :
	manager(manager),
	mesh(mesh),
	meshName(meshName) {

}

MeshRef::~MeshRef()  {
	manager->removeMeshReference(meshName);
}

Model::Model(const std::string& name, std::shared_ptr<const MeshRef> mesh, const std::string& shader, const std::string& uniformSet, const UniformSet& uniforms, bool viewCull)  :
	name(name),
	mesh(mesh),
	shader(shader),
	uniformSet(uniformSet),
	hasBufferedUniforms(false),
	textures(),
	viewCull(viewCull),
	references(0),
	uniforms(stripNonBufferedModel(uniforms)),
	uniformData(new unsigned char[this->uniforms.getUniformDataSize()], ArrayDeleter()) {

	for (const UniformDescription& descr : uniforms.uniforms) {
		if (!isSampler(descr.type)) {
			hasBufferedUniforms = true;
			break;
		}
	}
}

ModelRef::ModelRef(ModelManager* manager, std::string modelName, Model& model) :
	manager(manager),
	model(model),
	mesh(model.mesh->getMesh()),
	modelName(modelName) {

}

ModelRef::~ModelRef()  {
	manager->removeModelReference(modelName);
}
