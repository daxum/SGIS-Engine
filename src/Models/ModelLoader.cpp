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

#include <stdexcept>
#include <unordered_map>

#include "ModelLoader.hpp"
#include "ModelManager.hpp"
#include "Engine.hpp"
#include "MeshBuilder.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void ModelLoader::loadMaterial(const std::string& name, const MaterialCreateInfo& matInfo) {
	const UniformSet& matSet = modelManager.getMemoryManager()->getUniformSet(matInfo.uniformSet);

	Material material(name, matInfo.shader, matInfo.uniformSet, matSet, matInfo.viewCull);

	//Someone should probably change this to unordered...
	std::map<std::string, int> matMap;
	std::vector<tinyobj::material_t> loadedMats;
	std::string warn;
	std::string error;

	std::string matFileName = Engine::instance->getConfig().resourceBase + matInfo.filename;
	std::ifstream matFile(matFileName);

	if (!matFile.is_open()) {
		throw std::runtime_error("Couldn't open material file " + matFileName + "!");
	}

	tinyobj::LoadMtl(&matMap, &loadedMats, &matFile, &warn, &error);

	if (!error.empty()) {
		ENGINE_LOG_ERROR(logger, error);
	}

	if (!warn.empty()) {
		ENGINE_LOG_WARN(logger, warn);
	}

	if (loadedMats.empty()) {
		throw std::runtime_error("Failed to load material " + matFileName + "!");
	}

	tinyobj::material_t mat = loadedMats.at(0);

	for (const UniformDescription& uniform : matSet.getBufferedUniforms()) {
		//I should probably just write my own material loader, honestly
		if (uniform.name == UNIFORM_NAME_KA) material.uniforms.setVec3(UNIFORM_NAME_KA, glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]));
		if (uniform.name == UNIFORM_NAME_KD) material.uniforms.setVec3(UNIFORM_NAME_KD, glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]));
		if (uniform.name == UNIFORM_NAME_KS) material.uniforms.setVec3(UNIFORM_NAME_KS, glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]));
		if (uniform.name == UNIFORM_NAME_TRANSMITTANCE) material.uniforms.setVec3(UNIFORM_NAME_TRANSMITTANCE, glm::vec3(mat.transmittance[0], mat.transmittance[1], mat.transmittance[2]));
		if (uniform.name == UNIFORM_NAME_EMISSION) material.uniforms.setVec3(UNIFORM_NAME_EMISSION, glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]));
		if (uniform.name == UNIFORM_NAME_SHININESS) material.uniforms.setFloat(UNIFORM_NAME_SHININESS, mat.shininess);
		if (uniform.name == UNIFORM_NAME_IOR) material.uniforms.setFloat(UNIFORM_NAME_IOR, mat.ior);
		if (uniform.name == UNIFORM_NAME_DISSOLVE) material.uniforms.setFloat(UNIFORM_NAME_DISSOLVE, mat.dissolve);
		if (uniform.name == UNIFORM_NAME_ROUGHNESS) material.uniforms.setFloat(UNIFORM_NAME_ROUGHNESS, mat.roughness);
		if (uniform.name == UNIFORM_NAME_METALLIC) material.uniforms.setFloat(UNIFORM_NAME_METALLIC, mat.metallic);
		if (uniform.name == UNIFORM_NAME_SHEEN) material.uniforms.setFloat(UNIFORM_NAME_SHEEN, mat.sheen);
		if (uniform.name == UNIFORM_NAME_CLEARCOAT_THICK) material.uniforms.setFloat(UNIFORM_NAME_CLEARCOAT_THICK, mat.clearcoat_thickness);
		if (uniform.name == UNIFORM_NAME_CLEARCOAT_ROUGH) material.uniforms.setFloat(UNIFORM_NAME_CLEARCOAT_ROUGH, mat.clearcoat_roughness);
		if (uniform.name == UNIFORM_NAME_ANISOTROPY) material.uniforms.setFloat(UNIFORM_NAME_ANISOTROPY, mat.anisotropy);
		if (uniform.name == UNIFORM_NAME_ANISOTROPY_ROTATION) material.uniforms.setFloat(UNIFORM_NAME_ANISOTROPY_ROTATION, mat.anisotropy_rotation);
	}

	//Lots of textures
	for (const UniformDescription& uniform : matSet.getNonBufferedUniforms()) {
		if (uniform.name == UNIFORM_NAME_KA_TEX) material.textures.push_back(mat.ambient_texname);
		if (uniform.name == UNIFORM_NAME_KD_TEX) material.textures.push_back(mat.diffuse_texname);
		if (uniform.name == UNIFORM_NAME_KS_TEX) material.textures.push_back(mat.specular_texname);
		if (uniform.name == UNIFORM_NAME_SPEC_HILIGHT_TEX) material.textures.push_back(mat.specular_highlight_texname);
		if (uniform.name == UNIFORM_NAME_BUMP_TEX) material.textures.push_back(mat.bump_texname);
		if (uniform.name == UNIFORM_NAME_DISPLACE_TEX) material.textures.push_back(mat.displacement_texname);
		if (uniform.name == UNIFORM_NAME_ALPHA_TEX) material.textures.push_back(mat.alpha_texname);
		if (uniform.name == UNIFORM_NAME_REFLECTION_TEX) material.textures.push_back(mat.reflection_texname);
		if (uniform.name == UNIFORM_NAME_ROUGHNESS_TEX) material.textures.push_back(mat.roughness_texname);
		if (uniform.name == UNIFORM_NAME_METALLIC_TEX) material.textures.push_back(mat.metallic_texname);
		if (uniform.name == UNIFORM_NAME_SHEEN_TEX) material.textures.push_back(mat.sheen_texname);
		if (uniform.name == UNIFORM_NAME_EMISSIVE_TEX) material.textures.push_back(mat.emissive_texname);
		if (uniform.name == UNIFORM_NAME_NORMAL_TEX) material.textures.push_back(mat.normal_texname);
	}

	modelManager.addMaterial(name, std::move(material));
	ENGINE_LOG_DEBUG(logger, "Loaded material \"" + Engine::instance->getConfig().resourceBase + matInfo.filename + "\" as \"" + name + "\"");
}

void ModelLoader::loadMesh(const std::string& name, const MeshCreateInfo& meshInfo) {
	const VertexFormat* format = modelManager.getFormat(meshInfo.vertexFormat);

	Mesh::BufferInfo bufferInfo = {};

	if (meshInfo.renderable) {
		bufferInfo.vertexName = meshInfo.vertexBuffer;
		bufferInfo.indexName = meshInfo.indexBuffer;
		bufferInfo.vertex = modelManager.getMemoryManager()->getBuffer(bufferInfo.vertexName);
		bufferInfo.index = modelManager.getMemoryManager()->getBuffer(bufferInfo.indexName);
	}

	Mesh mesh = loadFromDisk(Engine::instance->getConfig().resourceBase + meshInfo.filename, format, bufferInfo);

	ENGINE_LOG_DEBUG(logger, "Calculated box " + mesh.getBox().toString() + " for mesh " + name);
	ENGINE_LOG_DEBUG(logger, "Radius of mesh is " + std::to_string(mesh.getRadius()));

	modelManager.addMesh(name, std::move(mesh), true);
	ENGINE_LOG_DEBUG(logger, "Loaded mesh \"" + Engine::instance->getConfig().resourceBase + meshInfo.filename + "\" as \"" + name + "\"");
}

Mesh ModelLoader::loadFromDisk(const std::string& filename, const VertexFormat* format, const Mesh::BufferInfo bufferInfo) {
	ENGINE_LOG_DEBUG(logger, "Loading model \"" + filename + "\".");

	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string error;

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &error, filename.c_str())) {
		ENGINE_LOG_FATAL(logger, "Failed to load model \"" + filename + "\"!");
		throw std::runtime_error(error);
	}

	if (!warn.empty()) {
		ENGINE_LOG_WARN(logger, warn);
	}

	MeshBuilder meshBuild(format, attributes.vertices.size());
	Vertex vertex(format);

	for (const tinyobj::shape_t& shape : shapes) {
		for (const tinyobj::index_t& index : shape.mesh.indices) {
			if (format->hasElement(VERTEX_ELEMENT_POSITION)) {
				vertex.setVec3(VERTEX_ELEMENT_POSITION, glm::vec3(
					attributes.vertices[3 * index.vertex_index],
					attributes.vertices[3 * index.vertex_index + 1],
					attributes.vertices[3 * index.vertex_index + 2]
				));
			}

			if (format->hasElement(VERTEX_ELEMENT_NORMAL)) {
				vertex.setVec3(VERTEX_ELEMENT_NORMAL, glm::vec3(
					attributes.normals[3 * index.normal_index],
					attributes.normals[3 * index.normal_index + 1],
					attributes.normals[3 * index.normal_index + 2]
				));
			}

			if (format->hasElement(VERTEX_ELEMENT_TEXTURE)) {
				if (!attributes.texcoords.empty()) {
					vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(
						attributes.texcoords[2 * index.texcoord_index],
						attributes.texcoords[2 * index.texcoord_index + 1]
					));
				}
				else {
					vertex.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(0.0, 0.0));
				}
			}

			meshBuild.addVertex(vertex);
		}
	}

	ENGINE_LOG_DEBUG(logger,
		"File \"" + filename + "\" loaded from disk. Stats:" +
		"\n\tVertices:          " + std::to_string(meshBuild.vertexCount()) +
		"\n\tIndices:           " + std::to_string(meshBuild.indexCount()) +
		"\n\tTotal loaded size: " + std::to_string(meshBuild.vertexCount() * format->getVertexSize() + meshBuild.indexCount() * sizeof(uint32_t)) + " bytes");

	return meshBuild.genMesh(bufferInfo);
}
