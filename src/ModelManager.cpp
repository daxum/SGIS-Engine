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

std::shared_ptr<ModelRef> ModelManager::getModel(const std::string& modelName) {
	logger.debug("Retrieving reference for model \"" + modelName + "\"");

	Model& model = modelMap.at(modelName);
	Mesh& mesh = meshMap.at(model.mesh);

	std::shared_ptr<ModelRef> ref = std::make_shared<ModelRef>(this, modelName, model, mesh);

	mesh.addUser();
	model.references++;

	//Upload mesh if needed
	if (!memoryManager->markUsed(model.mesh, mesh.getBuffer())) {
		logger.debug("Mesh data for \"" + model.mesh + "\" not present on renderer, uploading now...");

		auto meshData = mesh.getMeshData();

		memoryManager->addMesh(model.mesh, mesh.getBuffer(), std::get<0>(meshData), std::get<1>(meshData), std::get<2>(meshData));
	}

	return ref;
}

void ModelManager::removeReference(const std::string& modelName) {
	logger.debug("Removing reference to model \"" + modelName + "\"");

	Model& model = modelMap.at(modelName);
	const std::string meshName = model.mesh;

	Mesh& mesh = meshMap.at(meshName);

	//Get vertex buffer to determine model / mesh persistence
	BufferUsage usage = memoryManager->getBuffer(mesh.getBuffer()).getUsage();
	bool persistent = usage == BufferUsage::DEDICATED_LAZY;

	model.references--;
	logger.debug("Remaining references: " + std::to_string(model.references));

	if (model.references == 0 && !persistent) {
		logger.debug("Removing transitory model \"" + modelName + "\"");
		modelMap.erase(modelName);
	}

	//Model might not exist past this point! Don't use it!

	//Remove mesh user even if model is still present; this is so unreferenced meshes don't
	//take up unnecessary space in the vertex buffers when they're not in use (they will remain
	//present in the buffers, however, unless memory runs out)
	mesh.removeUser();
	logger.debug("Remaining mesh users: " + std::to_string(mesh.getUsers()));

	if (mesh.getUsers() == 0) {
		logger.debug("Removing unused mesh \"" + meshName + "\" from vertex buffers...");
		memoryManager->freeMesh(meshName, mesh.getBuffer());

		//If mesh is not persistent, completely remove it
		if (!persistent) {
			logger.debug("Deleting transitory mesh \"" + meshName + "\"");
			meshMap.erase(meshName);
		}
	}
}
