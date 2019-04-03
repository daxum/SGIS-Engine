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

#include "Material.hpp"
#include "Mesh.hpp"
#include "RendererMemoryManager.hpp"
#include "EngineConfig.hpp"
#include "Logger.hpp"

struct Model {
	//These two are the same as the pointers returned from
	//the get* functions in the references below. They are
	//only guaranteed to work while said references exist.
	const Material* material;
	const Mesh* mesh;

	//Only used for reference counting.
	std::shared_ptr<MaterialRef> matRef;
	std::shared_ptr<MeshRef> meshRef;
};

class ModelManager {
public:
	/**
	 * Creates a model manager.
	 * @param logConfig The configuration for the manager's logger.
	 */
	ModelManager(const LogConfig& logConfig) :
		logger(logConfig),
		memoryManager(nullptr) {}

	/**
	 * Creates a model using the provided material and mesh. This function is not
	 * threadsafe, however the references returned should be, with the exception
	 * of when two or more different references to the same material or mesh, obtained
	 * from different invocations of this function, are destroyed at the same time.
	 * In addition, if the material or mesh for the model is not yet uploaded to the
	 * rendering engine, it will be when this function is called.
	 * @param material The material for the model to use.
	 * @param mesh The mesh for the model to use.
	 * @return A new model which uses the provided material and mesh.
	 * @throw std::out_of_range if the material or mesh doesn't exist.
	 */
	Model getModel(const std::string& material, const std::string mesh);

	/**
	 * Gets a material. If the requested material is not uploaded to the rendering engine
	 * when this function is called, then it will be before this function returns.
	 * As all materials currently last the entire life of the engine, this function
	 * doesn't really do much at the moment.
	 * @param material The material to retrieve a reference to.
	 * @return A reference to the requested material.
	 * @throw std::out_of_range If the material doesn't exist.
	 */
	std::shared_ptr<MaterialRef> getMaterial(const std::string& material) { return std::make_shared<MaterialRef>(materialMap.at(material)); }

	/**
	 * Gets a mesh. If the mesh is not yet stored at the specified cache level,
	 * then it will be uploaded there when this function is called.
	 * @param mesh The mesh to retrieve.
	 * @param requiredLevel The level the retriever needs the mesh to be at.
	 * @return A reference to the mesh.
	 * @throw std::out_of_range if the mesh doesn't exist.
	 */
	std::shared_ptr<MeshRef> getMesh(const std::string& mesh, CacheLevel requiredLevel);

	/**
	 * Adds a mesh to the model manager. Mesh persistence is determined by the
	 * buffer it uses, and it will not ever be removed until at least one model
	 * has referenced it.
	 * @param name The name to store the mesh under.
	 * @param mesh The mesh to add.
	 * @param persist Whether to keep the mesh when it becomes unused.
	 */
	void addMesh(const std::string& name, Mesh&& mesh, bool persist) {
		meshMap.emplace(name, MeshData{std::move(mesh), std::array<size_t, CacheLevel::NUM_LEVELS>{}, persist});
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
	bool hasMaterial(const std::string& name) const { return materialMap.count(name); }

	/**
	 * Adds a material to the manager. Currently, materials are never deleted, but if
	 * they were, it wouldn't happen unless they were referenced at least once (like meshes).
	 * @param name The name of the material.
	 * @param model The material to add.
	 */
	void addMaterial(const std::string& name, Material&& material) {
		materialMap.emplace(name, std::move(material));
		ENGINE_LOG_INFO(logger, "Added model \"" + name + "\"");
	}

	/**
	 * Only called from MaterialRef's destructor. Removes a reference to the given material, and
	 * frees the material if needed.
	 * @param material The material to remove a reference from.
	 */
	void removeMaterialReference(const std::string& material) { /** Currently, all materials last the entire life of the game **/ }

	/**
	 * Only called from MeshRef's destructor. Almost exactly like removeModelReference, but
	 * will additionally remove the mesh from the rendering engine if needed.
	 * @param mesh The mesh to remove a reference from.
	 * @param level The level to remove a user from.
	 */
	void removeMeshReference(const std::string meshName, CacheLevel level);

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
	//Structure for storing meshes.
	struct MeshData {
		//The stored mesh.
		Mesh mesh;
		//Number of users for the mesh on each cache level.
		std::array<size_t, CacheLevel::NUM_LEVELS> users;
		//Whether the mesh stays loaded when it has no users.
		bool persist;
	};

	//The logger
	Logger logger;
	//Pointer to memory manager, for uploading mesh and material data.
	RendererMemoryManager* memoryManager;
	//Map of meshes.
	std::unordered_map<std::string, MeshData> meshMap;
	//Map of materials.
	std::unordered_map<std::string, Material> materialMap;
};
