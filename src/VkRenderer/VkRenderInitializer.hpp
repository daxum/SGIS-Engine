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

#include <vulkan/vulkan.h>

#include "RenderInitializer.hpp"
#include "VkObjectHandler.hpp"
#include "VkMemoryManager.hpp"

class VkRenderInitializer : public RenderInitializer {
public:
	/**
	 * Constructor.
	 */
	VkRenderInitializer(VkObjectHandler& vkObjects, VkMemoryManager* memoryManager) : RenderInitializer(memoryManager), vkObjects(vkObjects), vkMemManager(memoryManager) {}

	/**
	 * Adds a set of uniforms that can be used in shaders and models.
	 * This creates a descriptor set layout and adds it to the memory
	 * manager along with the set itself.
	 * @param set The set to add.
	 * @param name The name of the set.
	 */
	void addUniformSet(const UniformSet& set, const std::string& name) override;

private:
	//Object handler.
	VkObjectHandler& vkObjects;
	//Convenience to avoid extra casting.
	VkMemoryManager* vkMemManager;

	/**
	 * Converts the uniform set type to a VkDescriptor type.
	 * @param type The type of the uniform set.
	 * @return The corresponding descriptor type.
	 */
	static constexpr VkDescriptorType descriptorTypeFromSet(const UniformSetType type) {
		switch (type) {
			case UniformSetType::MODEL_STATIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case UniformSetType::MODEL_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			case UniformSetType::PER_SCREEN: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			case UniformSetType::PER_OBJECT: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			default: throw std::runtime_error("Invalid uniform set type!?");
		}
	}
};
