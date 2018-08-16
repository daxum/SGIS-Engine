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

void VkRenderInitializer::addUniformSet(const UniformSet& set, const std::string& name) {
	std::bitset<2> uboUseStages;
	uint32_t nextBinding = 0;
	bool hasUbo = false;

	//Set stages the uniform buffer is used in
	for (const UniformDescription& descr : set.uniforms) {
		if (!isSampler(descr.type)) {
			uboUseStages |= descr.shaderStages;
		}
	}

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	//Create descriptor set layout
	for (const UniformDescription& descr : set.uniforms) {
		if (!isSampler(descr.type)) {
			//Only add the uniform buffer once, at the location of the first buffered variable
			if (hasUbo) {
				continue;
			}

			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = nextBinding;
			binding.descriptorType = descriptorTypeFromSet(set.setType);
			binding.descriptorCount = 1;
			binding.stageFlags = uboUseStages.to_ulong();

			bindings.push_back(binding);
			nextBinding++;
			hasUbo = true;
		}
		else {
			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = nextBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = 1;
			binding.stageFlags = descr.shaderStages.to_ulong();

			bindings.push_back(binding);
			nextBinding++;
		}
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = bindings.size();
	layoutCreateInfo.pBindings = bindings.data();

	VkDescriptorSetLayout setLayout;

	if (vkCreateDescriptorSetLayout(vkObjects.getDevice(), &layoutCreateInfo, nullptr, &setLayout) != VK_SUCCESS) {
		throw std::runtime_error("Could not create descriptor set layout");
	}

	vkMemManager->addDescriptorSet(name, set, setLayout);
}
