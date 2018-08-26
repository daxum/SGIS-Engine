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

#define VMA_IMPLEMENTATION

//Unused variable due to VMA_ASSERT
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "vk_mem_alloc.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "VkMemoryManager.hpp"
#include "VkRenderingEngine.hpp"

VkMemoryManager::VkMemoryManager(const LogConfig& logConfig, VkObjectHandler& objects) :
	RendererMemoryManager(logConfig),
	objects(objects),
	allocator(VK_NULL_HANDLE),
	transferBuffer(VK_NULL_HANDLE),
	transferAllocation(VK_NULL_HANDLE),
	uniformBuffers({VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}),
	uniformBufferAllocations({VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}),
	transferFence(VK_NULL_HANDLE),
	transferOffset(0),
	transferSize(0),
	growTransfer(true),
	pendingTransfers(),
	meshMap(),
	descriptorLayouts(),
	staticModelPool(VK_NULL_HANDLE),
	dynamicPool(VK_NULL_HANDLE),
	descriptorSets(),
	descriptorAligners(),
	currentUniformOffset(0),
	screenObjectBufferSize(0),
	samplerMap(),
	imageMap() {

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

	for (size_t i = 0; i < uniformBuffers.size(); i++) {
		if (uniformBuffers.at(i) != VK_NULL_HANDLE) {
			vmaDestroyBuffer(allocator, uniformBuffers.at(i), uniformBufferAllocations.at(i));
		}
	}

	imageMap.clear();

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
		vkDestroyDescriptorSetLayout(objects.getDevice(), layoutInfoPair.second.layout, nullptr);
	}

	vkDestroyDescriptorPool(objects.getDevice(), staticModelPool, nullptr);
	vkDestroyDescriptorPool(objects.getDevice(), dynamicPool, nullptr);
}

void VkMemoryManager::initializeDescriptors() {
	//Create descriptor pools

	createDescriptorPool(&staticModelPool, [](UniformSetType type) -> bool { return type == UniformSetType::MODEL_STATIC; });
	createDescriptorPool(&dynamicPool, [](UniformSetType type) -> bool { return type != UniformSetType::MODEL_STATIC; });

	//Allocate dynamic descriptor sets from dynamic pool
	for (const auto& uniformSetPair : uniformSets) {
		const UniformSet& uniformSet = uniformSetPair.second;

		if (uniformSetPair.second.setType != UniformSetType::MODEL_STATIC) {
			const DescriptorLayoutInfo& layoutInfo = descriptorLayouts.at(uniformSetPair.first);

			VkDescriptorSetAllocateInfo setAllocInfo = {};
			setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			setAllocInfo.descriptorPool = dynamicPool;
			setAllocInfo.descriptorSetCount = 1;
			setAllocInfo.pSetLayouts = &layoutInfo.layout;

			VkDescriptorSet set = VK_NULL_HANDLE;

			if (vkAllocateDescriptorSets(objects.getDevice(), &setAllocInfo, &set) != VK_SUCCESS) {
				ENGINE_LOG_FATAL(logger, "Failed to allocate dynamic descriptor set!");
				throw std::runtime_error("Failed to allocate dynamic descriptor set!");
			}

			descriptorSets.insert({uniformSetPair.first, set});
			fillDescriptorSet(set, layoutInfo, uniformSet);
		}
	}
}

void VkMemoryManager::executeTransfers() {
	//Nothing to transfer
	if (pendingTransfers.empty() && pendingImageTransfers.empty()) {
		return;
	}

	//Wait for any previous transfers to complete
	vkWaitForFences(objects.getDevice(), 1, &transferFence, VK_TRUE, 0);
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

		//Destroy old transfer buffer if needed
		if (transferBuffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(allocator, transferBuffer, transferAllocation);
		}

		//Create new transfer buffer
		if (vmaCreateBuffer(allocator, &transferCreateInfo, &allocCreateInfo, &transferBuffer, &transferAllocation, nullptr) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create transfer buffer!");
		}

		growTransfer = false;
	}

	//Copy data to be transferred into transfer buffer
	//TODO: Persistently map?
	unsigned char* bufferData = nullptr;
	if (vmaMapMemory(allocator, transferAllocation, (void**) &bufferData) != VK_SUCCESS) {
		throw std::runtime_error("Wait a second. How did this happen? We're smarter than this. (Error while mapping transfer buffer)");
	}

	//Sorts transfers by destination buffer
	std::unordered_map<VkBuffer, std::vector<VkBufferCopy>> copyData;
	std::vector<std::pair<VkImage, VkBufferImageCopy>> imageCopyData;

	//Buffer transfers
	while (!pendingTransfers.empty()) {
		TransferOperation& transferOp = pendingTransfers.front();
		memcpy(bufferData + transferOp.srcOffset, transferOp.data, transferOp.size);

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
		memcpy(bufferData + imageTransferOp.offset, imageTransferOp.data, imageTransferOp.size);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = imageTransferOp.offset;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageOffset = {0, 0, 0};
		copyRegion.imageExtent = {imageTransferOp.width, imageTransferOp.height, 1};

		imageCopyData.push_back({imageTransferOp.image, copyRegion});

		delete[] imageTransferOp.data;
		pendingImageTransfers.pop();
	}

	vmaUnmapMemory(allocator, transferAllocation);

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
		barrier.subresourceRange.layerCount = 1;

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

uint32_t VkMemoryManager::writePerFrameUniforms(const Std140Aligner& uniformProvider, size_t currentFrame) {
	unsigned char* bufferData = nullptr;

	if (vmaMapMemory(allocator, uniformBufferAllocations.at(2), (void**)&bufferData) != VK_SUCCESS) {
		throw std::runtime_error("Failed to map memory?!?!");
	}

	const unsigned char* writeData = uniformProvider.getData().first;
	const size_t writeSize = uniformProvider.getData().second;

	const size_t writeOffset = screenObjectBufferSize * currentFrame + currentUniformOffset;

	memcpy(&bufferData[writeOffset], writeData, writeSize);
	currentUniformOffset += writeSize;

	//TODO: flush if not coherent

	vmaUnmapMemory(allocator, uniformBufferAllocations.at(2));

	return writeOffset;
}

void VkMemoryManager::allocateImage(const std::string& imageName, const VkImageCreateInfo& imageInfo, const unsigned char* imageData, size_t dataSize) {
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create an image!");
	}

	queueImageTransfer(image, dataSize, imageData, imageInfo.extent.width, imageInfo.extent.height);

	imageMap.insert({imageName, std::make_shared<VkImageData>(allocator, objects.getDevice(), image, allocation)});
}

std::shared_ptr<RenderBufferData> VkMemoryManager::createBuffer(const std::vector<VertexElement>& vertexFormat, BufferUsage usage, size_t size) {
	VkBufferUsageFlags transferFlags = 0;
	VmaMemoryUsage memoryUsage;

	switch (usage) {
		case BufferUsage::DEDICATED_LAZY:
			transferFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case BufferUsage::DEDICATED_SINGLE:
			transferFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case BufferUsage::STREAM:
			memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			break;
		default: throw std::runtime_error("Missing buffer usage in memory manager");
	}

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | transferFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t bufferUsers[] = { objects.getGraphicsQueueIndex(), objects.getTransferQueueIndex() };

	if (objects.hasUniqueTransfer()) {
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferCreateInfo.queueFamilyIndexCount = 2;
		bufferCreateInfo.pQueueFamilyIndices = bufferUsers;
	}

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = memoryUsage;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexAllocation = VK_NULL_HANDLE;

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &vertexBuffer, &vertexAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | transferFlags;

	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexAllocation = VK_NULL_HANDLE;

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &indexBuffer, &indexAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create index buffer!");
	}

	return std::make_shared<VkBufferData>(allocator, vertexBuffer, indexBuffer, vertexAllocation, indexAllocation);
}

void VkMemoryManager::createUniformBuffers(size_t modelStaticSize, size_t modelDynamicSize, size_t screenObjectSize) {
	VkBufferCreateInfo staticModelCreateInfo = {};
	staticModelCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	staticModelCreateInfo.size = modelStaticSize;
	staticModelCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	staticModelCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t bufferUsers[] = { objects.getGraphicsQueueIndex(), objects.getTransferQueueIndex() };

	if (objects.hasUniqueTransfer()) {
		staticModelCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		staticModelCreateInfo.queueFamilyIndexCount = 2;
		staticModelCreateInfo.pQueueFamilyIndices = bufferUsers;
	}

	VkBufferCreateInfo dynamicModelCreateInfo = {};
	dynamicModelCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	dynamicModelCreateInfo.size = modelDynamicSize;
	dynamicModelCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	dynamicModelCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (objects.hasUniqueTransfer()) {
		dynamicModelCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		dynamicModelCreateInfo.queueFamilyIndexCount = 2;
		dynamicModelCreateInfo.pQueueFamilyIndices = bufferUsers;
	}

	VkBufferCreateInfo screenObjectCreateInfo = {};
	screenObjectCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	screenObjectCreateInfo.size = screenObjectSize * VkRenderingEngine::MAX_ACTIVE_FRAMES;
	screenObjectCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	screenObjectCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	screenObjectBufferSize = screenObjectSize;

	VmaAllocationCreateInfo modelAllocCreateInfo = {};
	modelAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationCreateInfo screenObjectAllocCreateInfo = {};
	screenObjectAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	if (!(modelStaticSize == 0 || vmaCreateBuffer(allocator, &staticModelCreateInfo, &modelAllocCreateInfo, &uniformBuffers.at(0), &uniformBufferAllocations.at(0), nullptr) == VK_SUCCESS) ||
		!(modelDynamicSize == 0 || vmaCreateBuffer(allocator, &dynamicModelCreateInfo, &modelAllocCreateInfo, &uniformBuffers.at(1), &uniformBufferAllocations.at(1), nullptr) == VK_SUCCESS) ||
		!(screenObjectSize == 0 || vmaCreateBuffer(allocator, &screenObjectCreateInfo, &screenObjectAllocCreateInfo, &uniformBuffers.at(2), &uniformBufferAllocations.at(2), nullptr) == VK_SUCCESS)) {

		throw std::runtime_error("Failed to create one or more uniform buffers!");
	}

	ENGINE_LOG_DEBUG(logger, "Created uniform buffers");
}

void VkMemoryManager::uploadMeshData(const VertexBuffer& buffer, const std::string& mesh, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) {
	std::shared_ptr<const VkBufferData> bufferData = std::static_pointer_cast<const VkBufferData>(buffer.getRenderData());

	queueTransfer(bufferData->vertexBuffer, offset, size, vertexData);
	queueTransfer(bufferData->indexBuffer, indexOffset, indexSize, (const unsigned char*) indexData);

	//Add to mesh map
	meshMap.insert({mesh, VkMeshRenderData{indexOffset, (uint32_t) (indexSize / sizeof(uint32_t))}});
}

void VkMemoryManager::addModelDescriptors(const Model& model) {
	if (descriptorSets.count(model.name)) {
		return;
	}

	const DescriptorLayoutInfo& layoutInfo = descriptorLayouts.at(model.uniformSet);
	const UniformSet& uniformSet = uniformSets.at(model.uniformSet);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = staticModelPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = &layoutInfo.layout;

	VkDescriptorSet set = VK_NULL_HANDLE;

	if (vkAllocateDescriptorSets(objects.getDevice(), &setAllocInfo, &set) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate static model descriptor set!");
	}

	descriptorSets.insert({model.name, set});
	fillDescriptorSet(set, layoutInfo, uniformSet);
}

void VkMemoryManager::uploadModelData(const UniformBufferType buffer, const size_t offset, const size_t size, const unsigned char* data) {
	VkBuffer uploadBuffer = VK_NULL_HANDLE;

	switch (buffer) {
		case UniformBufferType::STATIC_MODEL: uploadBuffer = uniformBuffers.at(0); break;
		case UniformBufferType::DYNAMIC_MODEL: uploadBuffer = uniformBuffers.at(1); break;
		default: throw std::runtime_error("Invalid uniform buffer for transfer upload!");
	}

	//Probably need uniform-specific transfer for better concurrency later
	queueTransfer(uploadBuffer, offset, size, data);
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

void VkMemoryManager::queueImageTransfer(VkImage image, size_t size, const unsigned char* data, uint32_t imageWidth, uint32_t imageHeight) {
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
		imageHeight
	};

	memcpy(transferOp.data, data, size);
	pendingImageTransfers.push(transferOp);

	transferOffset += size;
}

void VkMemoryManager::createDescriptorPool(VkDescriptorPool* pool, std::function<bool(UniformSetType)> setInPool) {
	uint32_t setCount = 0;
	std::unordered_map<VkDescriptorType, uint32_t> typeCounts;

	//Find all descriptor set counts and max number of sets
	for (const auto& setPair : uniformSets) {
		if (setInPool(setPair.second.setType)) {
			setCount += setPair.second.maxUsers;

			//Add all the binding counts
			for (const auto& bindingPair : descriptorLayouts.at(setPair.first).bindings) {
				if (typeCounts.count(bindingPair.first) == 0) {
					typeCounts.insert({bindingPair.first, 0});
				}

				typeCounts.at(bindingPair.first) += setPair.second.maxUsers;
			}
		}
	}

	//Empty descriptor pool, don't create
	if (setCount == 0) {
		return;
	}

	//Convert to VkDescriptorPoolSize

	std::vector<VkDescriptorPoolSize> poolSizes;

	for (const auto& elem : typeCounts) {
		poolSizes.push_back({elem.first, elem.second});
	}

	//Create pool
	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = setCount;
	poolCreateInfo.poolSizeCount = poolSizes.size();
	poolCreateInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(objects.getDevice(), &poolCreateInfo, nullptr, pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void VkMemoryManager::fillDescriptorSet(VkDescriptorSet set, const DescriptorLayoutInfo& layoutInfo, const UniformSet& uniformSet) {
	std::vector<VkWriteDescriptorSet> writeOps(layoutInfo.bindings.size(), VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET});
	std::vector<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkDescriptorImageInfo> imageInfos;

	for (size_t i = 0; i < layoutInfo.bindings.size(); i++) {
		const VkDescriptorType type = layoutInfo.bindings.at(i).first;
		const std::string& name = layoutInfo.bindings.at(i).second;

		switch (type) {
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = uniformBuffers.at(bufferIndexFromSetType(uniformSet.setType));
				bufferInfo.offset = 0;
				bufferInfo.range = Std140Aligner::getAlignedSize(uniformSet);

				bufferInfos.push_back(bufferInfo);

				VkWriteDescriptorSet& writeSet = writeOps.at(i);
				writeSet.dstSet = set;
				writeSet.dstBinding = i;
				writeSet.dstArrayElement = 0;
				writeSet.descriptorCount = 1;
				writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				writeSet.pBufferInfo = &bufferInfos.back();
			}; break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.sampler = samplerMap.at(name);
				imageInfo.imageView = imageMap.at(name)->getImageView();
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				imageInfos.push_back(imageInfo);

				VkWriteDescriptorSet& writeSet = writeOps.at(i);
				writeSet.dstSet = set;
				writeSet.dstBinding = i;
				writeSet.dstArrayElement = 0;
				writeSet.descriptorCount = 1;
				writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeSet.pImageInfo = &imageInfos.back();
			}; break;
			default: throw std::runtime_error("Unsupported descriptor type when filling descriptor set!");
		}
	}

	vkUpdateDescriptorSets(objects.getDevice(), writeOps.size(), writeOps.data(), 0, nullptr);
}
