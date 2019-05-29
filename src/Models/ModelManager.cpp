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

Model ModelManager::getModel(const std::string& material, const std::string& mesh) {
	ENGINE_LOG_SPAM(logger, "Creating model from material \"" + material + "\" and mesh \"" + mesh + "\"");

	std::shared_ptr<MaterialRef> matRef = getMaterial(material);
	std::shared_ptr<MeshRef> meshRef = getMesh(mesh, CacheLevel::GPU);

	return Model{
		.material = matRef->getMaterial(),
		.mesh = meshRef->getMesh(),
		.matRef = matRef,
		.meshRef = meshRef,
	};
}

std::shared_ptr<MeshRef> ModelManager::getMesh(const std::string& meshName, CacheLevel level) {
	ENGINE_LOG_SPAM(logger, "Retrieving reference for mesh \"" + meshName + "\"");

	MeshData& meshData = meshMap.at(meshName);
	Mesh& mesh = meshData.mesh;

	meshData.users.at(level)++;

	//Upload mesh if needed
	if (meshData.users.at(CacheLevel::GPU) > 0) {
		if (!mesh.isForRendering()) {
			throw std::runtime_error("Attempt to upload non-rendering mesh to rendering engine!");
		}

		memoryManager->addMesh(meshName, &mesh);
	}

	return std::make_shared<MeshRef>(this, meshName, &mesh, level);
}

void ModelManager::removeMeshReference(const std::string meshName, CacheLevel level) {
	ENGINE_LOG_SPAM(logger, "Removing reference to mesh \"" + meshName + "\"");

	MeshData& meshData = meshMap.at(meshName);
	Mesh& mesh = meshData.mesh;

	meshData.users.at(level)--;
	ENGINE_LOG_SPAM(logger, "Remaining mesh users: " + std::to_string(meshData.users.at(level)));

	if (meshData.users.at(CacheLevel::GPU) == 0) {
		//Rendering mesh, can remove from gpu
		if (mesh.isForRendering() && level == CacheLevel::GPU) {
			ENGINE_LOG_DEBUG(logger, "Removing unused mesh \"" + meshName + "\" from vertex buffers...");
			memoryManager->freeMesh(meshName, &mesh, meshData.persist);
		}

		//If mesh is not persistent, completely remove it
		if (!meshData.persist && meshData.users.at(CacheLevel::MEMORY) == 0) {
			ENGINE_LOG_DEBUG(logger, "Deleting transitory mesh \"" + meshName + "\"");
			meshMap.erase(meshName);
		}
	}
}
