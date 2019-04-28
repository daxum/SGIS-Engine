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

#include <deque>

#define VMA_IMPLEMENTATION

//Unused variable due to VMA_ASSERT
//Stupid clang doing incompatible nonsense. If I ask for gcc
//then I want gcc!
#ifdef __GNUC__
#	ifdef __clang__
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wunused-variable"
#	else
#		pragma GCC diagnostic ignored "-Wunused-variable"
#	endif
#endif

#include "vk_mem_alloc.h"

#ifdef __GNUC__
#	ifdef __clang__
#		pragma clang diagnostic pop
#	else
#		pragma GCC diagnostic pop
#	endif
#endif

#include "VkMemoryManager.hpp"
#include "VkRenderingEngine.hpp"
#include "ExtraMath.hpp"

VkMemoryManager::VkMemoryManager(const LogConfig& logConfig, VkObjectHandler& objects) :
	RendererMemoryManager(logConfig),
	objects(objects),
	allocator(VK_NULL_HANDLE),
	transferBuffer(VK_NULL_HANDLE),
	transferAllocation(VK_NULL_HANDLE),
	transferFence(VK_NULL_HANDLE),
	transferOffset(0),
	transferSize(0),
	growTransfer(true),
	pendingTransfers(),
	descriptorLayouts(),
	poolInfo({}),
	descriptorSets(),
	samplerMap(),
	imageMap(),
	depthBuffer(VK_NULL_HANDLE),
	depthView(VK_NULL_HANDLE),
	depthAllocation(VK_NULL_HANDLE),
	transferMem(nullptr) {

}

void VkMemoryManager::init() {
	//Create allocator
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = objects.getPhysicalDevice();
	allocatorCreateInfo.device = objects.getDevice();

	vmaCreateAllocator(&allocatorCreateInfo, &allocator);

	//Create fence
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(objects.getDevice(), &fenceInfo, nullptr, &transferFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create transfer fence!");
	}

	//Create command buffer
	VkCommandBufferAllocateInfo cmdBufferInfo = {};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandPool = objects.getTransferCommandPool();
	cmdBufferInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(objects.getDevice(), &cmdBufferInfo, &transferCommands) != VK_SUCCESS) {
		throw std::runtime_error("Well that's not good... (Out of memory)");
	}
}

void VkMemoryManager::deinit() {
	deleteBuffers();

	if (transferBuffer != VK_NULL_HANDLE) {
		vmaDestroyBuffer(allocator, transferBuffer, transferAllocation);
	}

	imageMap.clear();

	vkDestroyImageView(objects.getDevice(), depthView, nullptr);
	vmaDestroyImage(allocator, depthBuffer, depthAllocation);

	vkDestroyFence(objects.getDevice(), transferFence, nullptr);
	vmaDestroyAllocator(allocator);

	while (!pendingTransfers.empty()) {
		TransferOperation& transferOp = pendingTransfers.front();
		delete[] transferOp.data;
		pendingTransfers.pop();
	}

	while (!pendingImageTransfers.empty()) {
		ImageTransferOperation& transferOp = pendingImageTransfers.front();
		delete[] transferOp.data;
		pendingImageTransfers.pop();
	}

	for (const auto& samplerPair : samplerMap) {
		vkDestroySampler(objects.getDevice(), samplerPair.second, nullptr);
	}

	for (const auto& layoutInfoPair : descriptorLayouts) {
		vkDestroyDescriptorSetLayout(objects.getDevice(), layoutInfoPair.second, nullptr);
	}

	vkDestroyDescriptorPool(objects.getDevice(), poolInfo.pool, nullptr);
}

void VkMemoryManager::initializeDescriptors() {
	//Create descriptor pool
	if (poolInfo.maxSets == 0) {
		//Empty descriptor pool, don't create
		return;
	}

	//Convert to VkDescriptorPoolSize - always create at least one of each type for now, shouldn't have any real impact
	//since everything uses the same descriptor pool
	std::array<VkDescriptorPoolSize, sizeof(DescriptorPoolInfo::bindingCounts) / sizeof(uint32_t)> poolSizes;
	poolSizes.at(0) = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, std::max(1u, poolInfo.bindingCounts.dynamicUniformBuffers)};
	poolSizes.at(1) = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, std::max(1u, poolInfo.bindingCounts.combinedImageSamplers)};

	//Create pool
	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = poolInfo.maxSets;
	poolCreateInfo.poolSizeCount = poolSizes.size();
	poolCreateInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(objects.getDevice(), &poolCreateInfo, nullptr, &poolInfo.pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}

	//Allocate non-material descriptor sets from pool (TODO: This doesn't seem to be the correct place to do this)
	for (const auto& uniformSetPair : getUniformSetMap()) {
		const UniformSet& uniformSet = uniformSetPair.second;

		if (uniformSet.getType() == UniformSetType::MATERIAL) {
			continue;
		}

		const VkDescriptorSetLayout& layout = descriptorLayouts.at(uniformSetPair.first);

		VkDescriptorSetAllocateInfo setAllocInfo = {};
		setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocInfo.descriptorPool = poolInfo.pool;
		setAllocInfo.descriptorSetCount = 1;
		setAllocInfo.pSetLayouts = &layout;

		VkDescriptorSet set = VK_NULL_HANDLE;

		if (vkAllocateDescriptorSets(objects.getDevice(), &setAllocInfo, &set) != VK_SUCCESS) {
			ENGINE_LOG_FATAL(logger, "Failed to allocate dynamic descriptor set!");
			throw std::runtime_error("Failed to allocate dynamic descriptor set!");
		}

		descriptorSets.insert({uniformSetPair.first, set});
		fillDescriptorSet(set, uniformSet, std::vector<std::string>());
	}
}

void VkMemoryManager::executeTransfers() {
	//Nothing to transfer
	if (pendingTransfers.empty() && pendingImageTransfers.empty()) {
		return;
	}

	//Wait for any previous transfers to complete
	if (vkWaitForFences(objects.getDevice(), 1, &transferFence, VK_TRUE, 0xFFFFFFFFFFFFFFFF) != VK_SUCCESS) {
		throw std::runtime_error("Transfer fence wait failed!");
	}

	vkResetFences(objects.getDevice(), 1, &transferFence);

	//Resize transfer buffer if needed
	if (growTransfer) {
		ENGINE_LOG_DEBUG(logger, "Resizing transfer buffer to " + std::to_string(transferSize) + " bytes");

		VkBufferCreateInfo transferCreateInfo = {};
		transferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		transferCreateInfo.size = transferSize;
		transferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		transferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		//Destroy old transfer buffer if needed
		if (transferBuffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(allocator, transferBuffer, transferAllocation);
		}

		VmaAllocationInfo allocInfo;

		//Create new transfer buffer
		if (vmaCreateBuffer(allocator, &transferCreateInfo, &allocCreateInfo, &transferBuffer, &transferAllocation, &allocInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create transfer buffer!");
		}

		transferMem = (unsigned char*) allocInfo.pMappedData;
		growTransfer = false;
	}

	//Copy data to be transferred into transfer buffer

	//Sorts transfers by destination buffer
	std::unordered_map<VkBuffer, std::vector<VkBufferCopy>> copyData;
	std::vector<std::pair<VkImage, VkBufferImageCopy>> imageCopyData;

	//Buffer transfers
	while (!pendingTransfers.empty()) {
		TransferOperation& transferOp = pendingTransfers.front();
		memcpy(transferMem + transferOp.srcOffset, transferOp.data, transferOp.size);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = transferOp.srcOffset;
		copyRegion.dstOffset = transferOp.dstOffset;
		copyRegion.size = transferOp.size;

		//Stupid operator[] not working with pointers...
		if (!copyData.count(transferOp.buffer)) {
			copyData.insert({transferOp.buffer, std::vector<VkBufferCopy>()});
		}

		copyData.at(transferOp.buffer).push_back(copyRegion);

		delete[] transferOp.data;
		pendingTransfers.pop();
	}

	//Image transfers
	while (!pendingImageTransfers.empty()) {
		ImageTransferOperation& imageTransferOp = pendingImageTransfers.front();
		memcpy(transferMem + imageTransferOp.offset, imageTransferOp.data, imageTransferOp.size);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = imageTransferOp.offset;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = imageTransferOp.arrayLayers;
		copyRegion.imageOffset = {0, 0, 0};
		copyRegion.imageExtent = {imageTransferOp.width, imageTransferOp.height, 1};

		imageCopyData.push_back({imageTransferOp.image, copyRegion});

		delete[] imageTransferOp.data;
		pendingImageTransfers.pop();
	}

	//Execute transfer

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(transferCommands, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("We appear to be suffering temporary amnesia... (Memory ran out)");
	}

	for (const auto& element : copyData) {
		const std::vector<VkBufferCopy>& copyVec = element.second;
		const VkBuffer& dstBuffer = element.first;

		vkCmdCopyBuffer(transferCommands, transferBuffer, dstBuffer, copyVec.size(), copyVec.data());
	}

	for (const auto& copyPair : imageCopyData) {
		VkImage image = copyPair.first;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		vkCmdPipelineBarrier(transferCommands, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		vkCmdCopyBufferToImage(transferCommands, transferBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyPair.second);

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcQueueFamilyIndex = objects.getTransferQueueIndex();
		barrier.dstQueueFamilyIndex = objects.getGraphicsQueueIndex();

		//TODO: remove once synchronization done
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		//TODO: This isn't sufficient when the transfer queue != the graphics queue. Also need a semaphore or something.
		vkCmdPipelineBarrier(transferCommands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	if (vkEndCommandBuffer(transferCommands) != VK_SUCCESS) {
		throw std::runtime_error("But why? (No more memory)");
	}

	//TODO: semaphore signal operations for buffers and textures, need function to add waits to graphics command buffer,
	//wait to free objects until after a frame has completed (solves the reallocate while in use problem, per-frame release queue?)
	//Also need to figure out how to change buffers to use VK_SHARING_MODE_EXCLUSIVE - release might be a bit difficult
	//Also need the aquire operations in the graphics queue, can't forget that

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommands;

	if (vkQueueSubmit(objects.getTransferQueue(), 1, &submitInfo, transferFence) != VK_SUCCESS) {
		throw std::runtime_error("Er.. driver? Hello? Are you still there...? (transfer queue submission failed)");
	}

	//Reset offset for next frame
	transferOffset = 0;
}

void VkMemoryManager::allocateImage(const std::string& imageName, const VkImageCreateInfo& imageInfo, const unsigned char* imageData, size_t dataSize) {
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create an image!");
	}

	queueImageTransfer(image, dataSize, imageData, imageInfo.extent.width, imageInfo.extent.height, 1);

	imageMap.insert({imageName, std::make_shared<VkImageData>(allocator, objects.getDevice(), image, allocation, imageInfo.format, false)});
}

void VkMemoryManager::allocateCubeImage(const std::string& imageName, const VkImageCreateInfo& imageInfo, const unsigned char* imageData, size_t dataSize) {
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create an image!");
	}

	queueImageTransfer(image, dataSize, imageData, imageInfo.extent.width, imageInfo.extent.height, imageInfo.arrayLayers);

	imageMap.insert({imageName, std::make_shared<VkImageData>(allocator, objects.getDevice(), image, allocation, imageInfo.format, true)});
}

VkImageView VkMemoryManager::createDepthBuffer(VkExtent2D swapExtent) {
	vkDestroyImageView(objects.getDevice(), depthView, nullptr);
	vmaDestroyImage(allocator, depthBuffer, depthAllocation);

	VkImageCreateInfo depthCreateInfo = {};
	depthCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	depthCreateInfo.format = VK_FORMAT_D32_SFLOAT;
	depthCreateInfo.extent = {swapExtent.width, swapExtent.height, 1};
	depthCreateInfo.mipLevels = 1;
	depthCreateInfo.arrayLayers = 1;
	depthCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	depthCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	depthCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	//Is this needed?
	allocInfo.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

	if (vmaCreateImage(allocator, &depthCreateInfo, &allocInfo, &depthBuffer, &depthAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create depth buffer!");
	}

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = depthBuffer;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_D32_SFLOAT;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(objects.getDevice(), &viewInfo, nullptr, &depthView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create depth image view!");
	}

	return depthView;
}

void VkMemoryManager::createUniformSetType(const std::string& name, const UniformSet& set) {
	if (poolInfo.pool != VK_NULL_HANDLE) {
		throw std::runtime_error("Attempted to add uniform set after pools were initialized!");
	}

	//Create layout
	VkDescriptorSetLayout layout = createSetLayout(set);
	descriptorLayouts.emplace(name, layout);

	//Add descriptor types needed to the desriptor pool
	poolInfo.maxSets += set.getMaxUsers();

	if (!set.getBufferedUniforms().empty()) {
		poolInfo.bindingCounts.dynamicUniformBuffers += set.getMaxUsers();
	}

	for (const UniformDescription& uniform : set.getNonBufferedUniforms()) {
		//This'll make a bit more sense when there are descriptors that aren't textures
		switch (uniform.type) {
			case UniformType::SAMPLER_2D:
			case UniformType::SAMPLER_CUBE:
				poolInfo.bindingCounts.combinedImageSamplers += set.getMaxUsers();
				break;
			default: throw std::runtime_error("Missing uniform type when creating descriptor sets!");
		}
	}
}

void VkMemoryManager::addMaterialDescriptors(const Material* material) {
	if (descriptorSets.count(material->name)) {
		return;
	}

	const VkDescriptorSetLayout layout = descriptorLayouts.at(material->uniformSet);
	const UniformSet& uniformSet = getUniformSet(material->uniformSet);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = poolInfo.pool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = &layout;

	VkDescriptorSet set = VK_NULL_HANDLE;

	if (vkAllocateDescriptorSets(objects.getDevice(), &setAllocInfo, &set) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate static model descriptor set!");
	}

	descriptorSets.emplace(material->name, set);
	fillDescriptorSet(set, uniformSet, material->textures);
}

void VkMemoryManager::queueTransfer(VkBuffer buffer, size_t offset, size_t size, const unsigned char* data) {
	ENGINE_LOG_DEBUG(logger, "Queueing buffer transfer - Transfer size: " + std::to_string(transferSize) + ", Current offset: " + std::to_string(transferOffset) + ", Data size: " + std::to_string(size));

	//Grow transfer buffer if not big enough (actual reallocation happens in executeTransfers)
	if (transferOffset + size > transferSize) {
		transferSize += size;
		growTransfer = true;
	}

	//Add data to queued transfers
	TransferOperation transferOp = {
		buffer,
		new unsigned char[size],
		size,
		offset,
		transferOffset
	};

	memcpy(transferOp.data, data, size);
	pendingTransfers.push(transferOp);

	transferOffset += size;
}

void VkMemoryManager::queueImageTransfer(VkImage image, size_t size, const unsigned char* data, uint32_t imageWidth, uint32_t imageHeight, uint32_t arrayLayers) {
	ENGINE_LOG_DEBUG(logger, "Queueing image transfer - Transfer size: " + std::to_string(transferSize) + ", Current offset: " + std::to_string(transferOffset) + ", Image size: " + std::to_string(size));

	if (transferOffset + size > transferSize) {
		transferSize += size;
		growTransfer = true;
	}

	ImageTransferOperation transferOp = {
		image,
		new unsigned char[size],
		size,
		transferOffset,
		imageWidth,
		imageHeight,
		arrayLayers
	};

	memcpy(transferOp.data, data, size);
	pendingImageTransfers.push(transferOp);

	transferOffset += size;
}

VkDescriptorSetLayout VkMemoryManager::createSetLayout(const UniformSet& set) const {
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	if (!set.getBufferedUniforms().empty()) {
		std::bitset<32> uboUseStages;

		//Set stages the uniform buffer is used in
		for (const UniformDescription& descr : set.getBufferedUniforms()) {
			uboUseStages |= descr.shaderStages;
		}

		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = bindings.size();
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		binding.descriptorCount = 1;
		binding.stageFlags = uboUseStages.to_ulong();

		bindings.push_back(binding);
	}

	//Add non-uniform buffer bindings.
	for (const UniformDescription& descr : set.getNonBufferedUniforms()) {
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = bindings.size();
		//For now, everything that isn't a uniform buffer is a texture. When that
		//changes, there should be a function somewhere to convert descr.type to
		//a VkDescriptotType
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//If arrays are supported later, this'll need to change
		binding.descriptorCount = 1;
		binding.stageFlags = descr.shaderStages.to_ulong();

		bindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = bindings.size();
	layoutCreateInfo.pBindings = bindings.data();

	VkDescriptorSetLayout layout;

	if (vkCreateDescriptorSetLayout(objects.getDevice(), &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("Could not create descriptor set layout");
	}

	return layout;
}

void VkMemoryManager::fillDescriptorSet(VkDescriptorSet set, const UniformSet& uniformSet, const std::vector<std::string>& textures) {
	std::vector<VkWriteDescriptorSet> writeOps;
	std::deque<VkDescriptorBufferInfo> bufferInfos;
	std::deque<VkDescriptorImageInfo> imageInfos;

	if (!uniformSet.getBufferedUniforms().empty()) {
		//Add dynamic uniform buffer
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = ((const VkBufferContainer*) getUniformBuffer(uniformBufferFromSetType(uniformSet.getType())))->getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = Std140Aligner::getAlignedSize(uniformSet);

		bufferInfos.push_back(bufferInfo);

		VkWriteDescriptorSet writeSet = {};
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = set;
		writeSet.dstBinding = 0;
		writeSet.dstArrayElement = 0;
		writeSet.descriptorCount = 1;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writeSet.pBufferInfo = &bufferInfos.back();

		writeOps.push_back(writeSet);
	}

	size_t currentImageIndex = 0;

	for (const UniformDescription& descr : uniformSet.getNonBufferedUniforms()) {
		switch (descr.type) {
			case UniformType::SAMPLER_2D:
			case UniformType::SAMPLER_CUBE: {
				const std::string& texName = textures.at(currentImageIndex);

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.sampler = samplerMap.at(texName);
				imageInfo.imageView = imageMap.at(texName)->getImageView();
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				imageInfos.push_back(imageInfo);

				VkWriteDescriptorSet writeSet = {};
				writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSet.dstSet = set;
				writeSet.dstBinding = writeOps.size();
				writeSet.dstArrayElement = 0;
				writeSet.descriptorCount = 1;
				writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeSet.pImageInfo = &imageInfos.back();

				writeOps.push_back(writeSet);

				currentImageIndex++;
			}; break;
			default: throw std::runtime_error("Unsupported descriptor type when filling descriptor set!");
		}
	}

	vkUpdateDescriptorSets(objects.getDevice(), writeOps.size(), writeOps.data(), 0, nullptr);
}
