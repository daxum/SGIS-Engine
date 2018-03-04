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
#include "GlModelLoader.hpp"

GlModelLoader::GlModelLoader(Logger& logger, std::unordered_map<std::string, Model>& modelMap, GlMemoryManager& memoryManager) :
	ModelLoader(logger),
	models(modelMap),
	memoryManager(memoryManager) {

}

void GlModelLoader::loadModel(std::string name, std::string filename, std::string texture) {
	std::shared_ptr<ModelData> data = loadFromDisk(filename);
	//Always static for now
	MeshData mesh = memoryManager.addMesh(data->vertices, data->indices, MeshType::STATIC);
	models.insert(std::make_pair(name, Model(mesh, texture)));
	logger.debug("Loaded model \"" + filename + "\" as \"" + name + "\".");
}
