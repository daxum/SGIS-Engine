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

#include "VkTextureLoader.hpp"

VkTextureLoader::VkTextureLoader(VkObjectHandler& vkObjects, Logger& logger, VkMemoryManager& memoryManager) :
	TextureLoader(logger),
	vkObjects(vkObjects),
	memoryManager(memoryManager) {

}

void VkTextureLoader::loadTexture(const std::string& name, const std::string& filename, Filter minFilter, Filter magFilter, bool mipmap) {
	//Create texture

	TextureData data = loadFromDisk(filename);

	if (!data.loadSuccess) {
		ENGINE_LOG_WARN(logger, "Failed to load texture \"" + filename + "\"");
	}

	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	createInfo.extent = {data.width, data.height, 1};
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//TODO: remove once synchronization finished
	uint32_t queues[] = {vkObjects.getGraphicsQueueIndex(), vkObjects.getTransferQueueIndex()};

	if (vkObjects.hasUniqueTransfer()) {
		createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queues;
	}

	memoryManager.allocateImage(name, createInfo, data.data.get(), data.width * data.height * 4);

	//Create sampler

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = filterToVk(magFilter);
	samplerInfo.minFilter = filterToVk(minFilter);
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; //?
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE; //TODO
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	VkSampler sampler = VK_NULL_HANDLE;

	if (vkCreateSampler(vkObjects.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create sampler for texture \"" + name + "\"");
	}

	memoryManager.addSamplerForTexture(name, sampler);
}

void VkTextureLoader::loadCubeMap(const std::string& name, const std::vector<std::string>& filenames, Filter minFilter, Filter magFilter, bool mipmap) {
	/** TODO **/
}

void VkTextureLoader::addFontTexture(const std::string textureName, const TextureData& data) {
	/** TODO **/
}
