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

#include "ModelManager.hpp"

std::shared_ptr<const ModelRef> ModelManager::getModel(const std::string& modelName) {
	ENGINE_LOG_SPAM(logger, "Retrieving reference for model \"" + modelName + "\"");

	Model& model = modelMap.at(modelName);

	std::shared_ptr<ModelRef> ref = std::make_shared<ModelRef>(this, modelName, model);

	model.references++;

	//Upload model uniform data - the memory manager will skip redundant uploads
	memoryManager->addModel(modelName, model);

	return ref;
}

std::shared_ptr<const MeshRef> ModelManager::getMesh(const std::string& meshName) {
	ENGINE_LOG_SPAM(logger, "Retrieving reference for mesh \"" + meshName + "\"");

	Mesh& mesh = meshMap.at(meshName);
	std::shared_ptr<MeshRef> ref = std::make_shared<MeshRef>(this, meshName, mesh);

	mesh.addUser();

	//Upload mesh if needed
	if (mesh.render && !memoryManager->markUsed(meshName, mesh.getBuffer())) {
		ENGINE_LOG_DEBUG(logger, "Mesh data for \"" + meshName + "\" not present on renderer, uploading now...");

		auto meshData = mesh.getMeshData();

		memoryManager->addMesh(meshName, mesh.getBuffer(), std::get<0>(meshData), std::get<1>(meshData), std::get<2>(meshData));
	}

	return ref;
}

void ModelManager::removeModelReference(const std::string& modelName) {
	ENGINE_LOG_SPAM(logger, "Removing reference to model \"" + modelName + "\"");

	Model& model = modelMap.at(modelName);

	//Determine whether the model's uniform set needs freeing.
	const UniformSetType setType = memoryManager->getUniformSet(model.uniformSet).setType;
	const bool modelPersistent = setType == UniformSetType::MODEL_STATIC;

	model.references--;
	ENGINE_LOG_SPAM(logger, "Remaining references: " + std::to_string(model.references));

	//Allow reallocation of model uniform data, but don't actually remove it from the
	//buffers unless needed
	if (model.references == 0) {
		ENGINE_LOG_DEBUG(logger, "Removing unused model \"" + modelName + "\"");
		memoryManager->freeModel(modelName, model);

		if (!modelPersistent) {
			ENGINE_LOG_DEBUG(logger, "Deleting transitory model \"" + modelName + "\"");
			modelMap.erase(modelName);
		}
	}
}

void ModelManager::removeMeshReference(const std::string meshName) {
	ENGINE_LOG_SPAM(logger, "Removing reference to mesh \"" + meshName + "\"");

	Mesh& mesh = meshMap.at(meshName);

	//Get vertex buffer to determine mesh persistence
	const BufferUsage usage = memoryManager->getBuffer(mesh.getBuffer()).getUsage();
	const bool meshPersistent = usage == BufferUsage::DEDICATED_LAZY;

	mesh.removeUser();
	ENGINE_LOG_SPAM(logger, "Remaining mesh users: " + std::to_string(mesh.getUsers()));

	if (mesh.getUsers() == 0) {
		if (mesh.render) {
			ENGINE_LOG_DEBUG(logger, "Removing unused mesh \"" + meshName + "\" from vertex buffers...");
			memoryManager->freeMesh(meshName, mesh.getBuffer());
		}

		//If mesh is not persistent, completely remove it
		if (!meshPersistent) {
			ENGINE_LOG_DEBUG(logger, "Deleting transitory mesh \"" + meshName + "\"");
			meshMap.erase(meshName);
		}
	}
}
