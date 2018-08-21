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

#include "VkRenderInitializer.hpp"

void VkRenderInitializer::addUniformSet(const std::string& name, const UniformSet& set) {
	std::bitset<32> uboUseStages;
	uint32_t nextBinding = 0;
	bool hasUbo = false;

	//Set stages the uniform buffer is used in
	for (const UniformDescription& descr : set.uniforms) {
		if (!isSampler(descr.type)) {
			ENGINE_LOG_DEBUG(logger, std::string("Shader stages for uniform \"") + descr.name + "\" in set \"" + name + "\": " + std::to_string(descr.shaderStages.to_ulong()));

			uboUseStages |= descr.shaderStages;
		}
	}

	ENGINE_LOG_DEBUG(logger, std::string("Uniform buffer stages for \"") + name + "\": " + std::to_string(uboUseStages.to_ulong()));

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	DescriptorLayoutInfo layoutInfo = {};

	//Create descriptor set layout
	for (const UniformDescription& descr : set.uniforms) {
		if (!isSampler(descr.type)) {
			//Only add the uniform buffer once, at the location of the first buffered variable
			if (hasUbo) {
				continue;
			}

			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = nextBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			binding.descriptorCount = 1;
			binding.stageFlags = uboUseStages.to_ulong();

			bindings.push_back(binding);
			layoutInfo.bindings.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, "buffer"});
			nextBinding++;
			hasUbo = true;
		}
		else {
			if (set.setType == UniformSetType::PER_OBJECT || set.setType == UniformSetType::PER_SCREEN) {
				throw std::runtime_error("Samplers not allowed in per-object or per-screen uniform sets!");
			}

			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = nextBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = 1;
			binding.stageFlags = descr.shaderStages.to_ulong();

			bindings.push_back(binding);
			layoutInfo.bindings.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descr.name});
			nextBinding++;
		}
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = bindings.size();
	layoutCreateInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(vkObjects.getDevice(), &layoutCreateInfo, nullptr, &layoutInfo.layout) != VK_SUCCESS) {
		throw std::runtime_error("Could not create descriptor set layout");
	}

	vkMemManager->addDescriptorSet(name, set, layoutInfo);
}
