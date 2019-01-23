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

#include <memory>

#include "PhysicsObject.hpp"
#include "Model.hpp"
#include "Engine.hpp"

PhysicsObject::PhysicsObject(const PhysicsInfo& createInfo) :
	body(nullptr),
	shape(nullptr),
	state(nullptr),
	mesh(nullptr),
	startingMass(createInfo.mass) {

	switch (createInfo.shape) {
		case PhysicsShape::PLANE: shape = createPlaneObject(createInfo); break;
		case PhysicsShape::BOX: shape = createBoxObject(createInfo); break;
		case PhysicsShape::CAPSULE: shape = createCapsuleObject(createInfo); break;
		default: throw std::runtime_error("Missing physics shape!");
	}

	state = new btDefaultMotionState();

	btVector3 localInertia(0.0f, 0.0f, 0.0f);

	if (createInfo.mass != 0.0f) {
		shape->calculateLocalInertia(createInfo.mass, localInertia);
	}

	glm::vec3 origin = createInfo.pos;

	btTransform initialTransform;
	initialTransform.setIdentity();
	initialTransform.setOrigin(btVector3(origin.x, origin.y, origin.z));
	state->setWorldTransform(initialTransform);

	btRigidBody::btRigidBodyConstructionInfo info(createInfo.mass, state, shape, localInertia);

	body = new btRigidBody(info);
}

PhysicsObject::PhysicsObject(const std::string& modelName, const glm::vec3& pos) :
	body(nullptr),
	shape(nullptr),
	state(nullptr),
	mesh(nullptr),
	startingMass(0) {

	//TODO: This will currently upload the mesh and model to the rendering engine.
	//Prevent this in the future?
	std::shared_ptr<ModelRef> model = Engine::instance->getModel(modelName);
	const std::vector<VertexElement>& format = model->getMesh().getFormat();

	size_t vertexSize = 0;
	size_t posOffset = 0;
	bool hasPos = false;

	//Find position component of vertices
	for (const VertexElement& elem : format) {
		if (elem.name == VERTEX_ELEMENT_POSITION) {
			posOffset = vertexSize;
			hasPos = true;
		}

		vertexSize += sizeFromVertexType(elem.type);
	}

	if (!hasPos) {
		throw std::runtime_error("Attempt to generate physics shape from mesh without positions!");
	}

	const std::tuple<const unsigned char*, size_t, const std::vector<uint32_t>&>& meshData = model->getMesh().getMeshData();
	btTriangleMesh* mesh = new btTriangleMesh();

	//Create physics mesh
	for (size_t i = posOffset; i < std::get<1>(meshData); i += vertexSize) {
		glm::vec3 vertPos = *(glm::vec3*)(std::get<0>(meshData) + i);
		mesh->findOrAddVertex(btVector3(vertPos.x, vertPos.y, vertPos.z), false);
	}

	const std::vector<uint32_t>& indexData = std::get<2>(meshData);

	//Add indices
	for (size_t i = 0; i < indexData.size(); i += 3) {
		mesh->addTriangleIndices(indexData.at(i), indexData.at(i + 1), indexData.at(i + 2));
	}

	//Second parameter is "useQuantizedAabbCompression" - not sure what that does, but
	//it sounds good
	shape = new btBvhTriangleMeshShape(mesh, true);

	state = new btDefaultMotionState();

	btTransform initialTransform;
	initialTransform.setIdentity();
	initialTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	state->setWorldTransform(initialTransform);

	btRigidBody::btRigidBodyConstructionInfo info(0.0, state, shape);

	body = new btRigidBody(info);
}

btStaticPlaneShape* PhysicsObject::createPlaneObject(const PhysicsInfo& createInfo) {
	glm::vec3 normal(createInfo.box.max - createInfo.box.min);
	glm::normalize(normal);

	//Point on the plane
	glm::vec3 point = createInfo.box.min;

	//'d' from the plane equation: ax + by + cz + d = 0. Normally distance uses
	//absolute value and divides by length, but here that is fortunately not
	//necessary.
	float offset = glm::dot(normal, point);

	return new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), offset);
}
