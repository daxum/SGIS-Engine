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

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "Vertex.hpp"
#include "Logger.hpp"
#include "AxisAlignedBB.hpp"

class ModelManager;

struct MeshData {
	//The vertices in the model's mesh.
	std::vector<Vertex> vertices;
	//The indices for the draw order of the vertices.
	std::vector<uint32_t> indices;
};

struct MaterialCreateInfo {
	//The material file to load from.
	std::string filename;
	//The shader to use to render the material.
	std::string shader;
	//The uniform set the material uses. Must be compatible with the shader.
	std::string uniformSet;
	//Whether to perform view culling for this material. Is this the right
	//place for this?
	bool viewCull;
};

struct MeshCreateInfo {
	//The file to load the mesh from.
	std::string filename;
	//The vertex buffer to store the mesh in.
	std::string vertexBuffer;
	//The buffer to place the mesh's indices in.
	std::string indexBuffer;
	//The format of the mesh's vertices.
	std::string vertexFormat;
	//Whether the mesh is intended to be rendered. If false, the vertexBuffer
	//and indexBuffer parameters will be ignored.
	bool renderable;
};

//Lots of material names, to prevent mistyping.
const char* const UNIFORM_NAME_KA = "ka";
const char* const UNIFORM_NAME_KD = "kd";
const char* const UNIFORM_NAME_KS = "ks";
const char* const UNIFORM_NAME_TRANSMITTANCE = "trns";
const char* const UNIFORM_NAME_EMISSION = "emsn";
const char* const UNIFORM_NAME_SHININESS = "s";
const char* const UNIFORM_NAME_IOR = "ior";
const char* const UNIFORM_NAME_DISSOLVE = "dslv";

//Textures.
const char* const UNIFORM_NAME_KA_TEX = "katx";
const char* const UNIFORM_NAME_KD_TEX = "kdtx";
const char* const UNIFORM_NAME_KS_TEX = "kstx";
const char* const UNIFORM_NAME_SPEC_HILIGHT_TEX = "shltx";
const char* const UNIFORM_NAME_BUMP_TEX = "bmptx";
const char* const UNIFORM_NAME_DISPLACE_TEX = "dsptx";
const char* const UNIFORM_NAME_ALPHA_TEX = "altx";
const char* const UNIFORM_NAME_REFLECTION_TEX = "rfltx";

//PBR.
const char* const UNIFORM_NAME_ROUGHNESS = "rgh";
const char* const UNIFORM_NAME_METALLIC = "mtlc";
const char* const UNIFORM_NAME_SHEEN = "shn";
const char* const UNIFORM_NAME_CLEARCOAT_THICK = "clctthck";
const char* const UNIFORM_NAME_CLEARCOAT_ROUGH = "clctrgh";
const char* const UNIFORM_NAME_ANISOTROPY = "atrpy";
const char* const UNIFORM_NAME_ANISOTROPY_ROTATION = "atrpyrt";

//More textures.
const char* const UNIFORM_NAME_ROUGHNESS_TEX = "rghtx";
const char* const UNIFORM_NAME_METALLIC_TEX = "mtlctx";
const char* const UNIFORM_NAME_SHEEN_TEX = "shntx";
const char* const UNIFORM_NAME_EMISSIVE_TEX = "emsvtx";
const char* const UNIFORM_NAME_NORMAL_TEX = "nrmtx";

class ModelLoader {
public:
	/**
	 * Constructs a model loader.
	 * @param logConfig The configuration for the logger.
	 * @param modelManager The manager to load the models to.
	 */
	ModelLoader(const LogConfig& logConfig, ModelManager& modelManager) :
		logger(logConfig),
		modelManager(modelManager) {}

	/**
	 * Virtual destructor, does nothing.
	 */
	virtual ~ModelLoader() {}

	/**
	 * Loads a material from disk and makes it ready for rendering. This
	 * currently only supports .mtl files.
	 * @param name The name for the material.
	 * @param matInfo information about the material to load.
	 */
	void loadMaterial(const std::string& name, const MaterialCreateInfo& matInfo);

	/**
	 * Loads a mesh from disk and adds it to the model manager.
	 * @param name The name to store the mesh under.
	 * @param meshInfo The information required to load the mesh.
	 */
	void loadMesh(const std::string& name, const MeshCreateInfo& meshInfo);

protected:
	//The logger.
	Logger logger;
	//Model manager to load models to.
	ModelManager& modelManager;

	/**
	 * Loads a model from disk (currently only .obj is supported).
	 * @param filename The filename of the model to load.
	 * @param format The format of the mesh's vertices.
	 * @return a pointer to the loaded model data.
	 * @throw runtime_error if model loading failed.
	 */
	MeshData loadFromDisk(const std::string& filename, const VertexFormat* format);

	/**
	 * Calculates a bounding box for a model's mesh.
	 * @param data The model data to calculate a box for.
	 * @return a box for the mesh.
	 */
	Aabb<float> calculateBox(const MeshData& data) const;

	/**
	 * Calculates the maximum radius of the model.
	 * @param data The model data.
	 * @param center The center of the model.
	 * @return The distance of the farthest vertex from the center.
	 */
	float calculateMaxRadius(const MeshData& data, glm::vec3 center) const;
};
