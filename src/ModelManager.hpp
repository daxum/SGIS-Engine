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

#pragma once

#include <unordered_map>

#include "Model.hpp"
#include "RendererMemoryManager.hpp"
#include "EngineConfig.hpp"
#include "Logger.hpp"

class ModelManager {
public:
	/**
	 * Creates a model manager.
	 * @param logConfig The configuration for the manager's logger.
	 */
	ModelManager(const LogConfig& logConfig) :
		logger(logConfig.type, logConfig.mask, logConfig.outputFile),
		memoryManager(nullptr),
		meshMap(),
		modelMap() {}

	/**
	 * Gets a reference to the model with the provided name. This function is not
	 * threadsafe, however the reference returned should be, with the exception
	 * of when two or more different references to the same model, obtained from different
	 * invocations of this function, are destroyed at the same time.
	 * In addition, if the mesh for the model is not yet uploaded to the rendering engine,
	 * it will be when this function is called.
	 * @param model The model to retrieve.
	 * @return A reference to the model.
	 * @throw std::out_of_range if either the model or the model's mesh doesn't exist.
	 */
	std::shared_ptr<ModelRef> getModel(const std::string& model);

	/**
	 * Adds a mesh to the model manager. Mesh persistence is determined by the
	 * buffer it uses, and it will not ever be removed until at least one model
	 * has referenced it.
	 * @param name The name to store the mesh under.
	 * @param mesh The mesh to add.
	 */
	void addMesh(const std::string& name, const Mesh& mesh) {
		meshMap.insert({name, mesh});
		ENGINE_LOG_INFO(logger, "Added mesh \"" + name + "\"");
	}

	/**
	 * Returns whether a mesh with the provided name has already been uploaded.
	 * @return true if the mesh exists, false otherwise.
	 */
	bool hasMesh(const std::string& name) const { return meshMap.count(name); }

	/**
	 * Returns whether a model with the provided name has already been uploaded.
	 * @return true if the model exists, false otherwise.
	 */
	bool hasModel(const std::string& name) const { return modelMap.count(name); }

	/**
	 * Adds a model to the manager. The mesh the model references will have its
	 * reference count increased, but will not be uploaded to the renderer until
	 * the model is referenced. The model will also never be removed until it is
	 * referenced at least once.
	 * @param name The name of the model.
	 * @param model The model to add.
	 */
	void addModel(const std::string& name, const Model& model) {
		modelMap.insert({name, model});
		ENGINE_LOG_INFO(logger, "Added model \"" + name + "\"");
	}

	/**
	 * Only called from ModelRef's destructor. Removes a reference to the given model, and
	 * frees the model if needed. In addition, a reference to the model's mesh is also
	 * removed, and if the mesh has no more references, it is removed from the rendering
	 * engine.
	 * @param model The model to remove a reference from.
	 */
	void removeReference(const std::string& model);

	/**
	 * Called from the engine to set the renderer memory manager.
	 * @param manager A pointer the renderer's memory manager.
	 */
	void setMemoryManager(RendererMemoryManager* manager) { memoryManager = manager; }

	/**
	 * Gets the memory manager. Used by the model loader to get vertex buffers and
	 * descriptor sets.
	 * @return The memory manager.
	 */
	RendererMemoryManager* getMemoryManager() { return memoryManager; }

private:
	//The logger
	Logger logger;
	//Pointer to memory manager, for uploading mesh data.
	RendererMemoryManager* memoryManager;
	//Map of meshes for models.
	std::unordered_map<std::string, Mesh> meshMap;
	//Map of models.
	std::unordered_map<std::string, Model> modelMap;
};
