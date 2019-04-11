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

#include <queue>
#include <vector>
#include <unordered_map>
#include <array>
#include <functional>

#include <vulkan/vulkan.h>

#include "Renderer/RendererMemoryManager.hpp"
#include "VkObjectHandler.hpp"
#include "vk_mem_alloc.h"
#include "VkImageData.hpp"

struct VkBufferData {
	VmaAllocator allocator;

	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VmaAllocation vertexAllocation;
	VmaAllocation indexAllocation;

	VkBufferData(VmaAllocator allocator, VkBuffer vertexBuffer, VkBuffer indexBuffer, VmaAllocation vertexAllocation, VmaAllocation indexAllocation) :
		allocator(allocator),
		vertexBuffer(vertexBuffer),
		indexBuffer(indexBuffer),
		vertexAllocation(vertexAllocation),
		indexAllocation(indexAllocation) {}

	~VkBufferData() {
		vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
		vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
	}
};

struct VkMeshRenderData {
	uintptr_t indexStart;
	uint32_t indexCount;
};

struct TransferOperation {
	//The buffer to transfer to.
	VkBuffer buffer;
	//The data to transfer.
	unsigned char* data;
	//The size of the data.
	size_t size;
	//Offset into the destination buffer to place the data.
	size_t dstOffset;
	//Offset into the transfer buffer to place the data.
	size_t srcOffset;
};

struct ImageTransferOperation {
	//The image to transfer to.
	VkImage image;
	//The image data.
	unsigned char* data;
	//The size of the data.
	size_t size;
	//The offset to place the image data into the transfer buffer.
	size_t offset;
	//The width of the image.
	uint32_t width;
	//The height of the image.
	uint32_t height;
	//The number of array layers to transfer (Typically 1).
	uint32_t arrayLayers;
};

struct DescriptorLayoutInfo {
	//Bindings for the descriptor set with this layout. First element
	//is the type, second is the name, which is used for textures.
	std::vector<std::pair<VkDescriptorType, std::string>> bindings;
	//Layout object. Don't forget to delete!
	VkDescriptorSetLayout layout;
};

class VkMemoryManager : public RendererMemoryManager {
public:
	/**
	 * Initializes a VkMemoryManager.
	 * @param logConfig The configuration for the logger.
	 * @param objects Vulkan objects.
	 */
	VkMemoryManager(const LogConfig& logConfig, VkObjectHandler& objects);

	/**
	 * Creates internal allocator.
	 */
	void init();

	/**
	 * Destroys allocator.
	 */
	void deinit();

	/**
	 * Creates descriptor pools and allocates descriptor sets for dynamic models,
	 * objects, and screens.
	 */
	void initializeDescriptors() override;

	/**
	 * Sends all pending transfer operations to their destinations.
	 */
	void executeTransfers();

	/**
	 * Gets the rendering data for the mesh with the given name.
	 * @param mesh The name of the mesh to get data for.
	 * @return The rendering data for the mesh.
	 */
	const VkMeshRenderData& getMeshRenderData(const std::string& mesh) const { return meshMap.at(mesh); }

	/**
	 * Gets the layout of the provided descriptor set, used in shader loading.
	 * @param name The name of the descriptor set layout to fetch.
	 * @return The layout with the given name.
	 */
	VkDescriptorSetLayout getSetLayout(const std::string& name) const { return descriptorLayouts.at(name).layout; }

	/**
	 * Gets the descriptor set with the given name.
	 * @param set The name of the descriptor set.
	 * @return The set with the given name.
	 */
	VkDescriptorSet getDescriptorSet(const std::string& set) const { return descriptorSets.at(set); }

	/**
	 * Adds a descriptor set to the rendering engine, as well as a layout that can be used in shaders.
	 * @param name The name of the set to add.
	 * @param uniformSet The uniform set.
	 * @param setLayout The layout of the uniform set.
	 */
	/*void addDescriptorSet(const std::string& name, const UniformSet& uniformSet, const DescriptorLayoutInfo& setLayout) {
		uniformSets.insert({name, uniformSet});
		descriptorLayouts.insert({name, setLayout});

		if (uniformSet.setType == UniformSetType::PER_SCREEN || uniformSet.setType == UniformSetType::PER_OBJECT) {
			descriptorAligners.insert({name, Std140Aligner(uniformSet.uniforms)});
		}
	}*/

	/**
	 * Gets the aligner for the given descriptor set.
	 * @param name The name of the descriptor set.
	 * @return The aligner for the set.
	 */
	Std140Aligner& getDescriptorAligner(const std::string& name) { return descriptorAligners.at(name); }

	/**
	 * Writes the provided uniform values into the uniform buffer for the current frame and returns the offset
	 * they were written at.
	 * @param uniformProvider The provider of the uniforms.
	 * @param currentFrame The current frame index.
	 * @return The offset the uniform values were written at.
	 */
	uint32_t writePerFrameUniforms(const Std140Aligner& uniformProvider, size_t currentFrame);

	/**
	 * Called after each frame completes.
	 */
	void resetPerFrameOffset() { currentUniformOffset = 0; }

	/**
	 * Allocates memory for an image, and creates a VkImage using that memory.
	 * Also queues the image data for uploading.
	 * @param imageName The name of the image to add.
	 * @param imageInfo The creation struct for the image.
	 * @param imageData The image data.
	 * @param dataSize The size of the image data.
	 */
	void allocateImage(const std::string& imageName, const VkImageCreateInfo& imageInfo, const unsigned char* imageData, size_t dataSize);

	/**
	 * Allocates memory for and uploads a cubemap image. Possibly expand this later for
	 * generic array textures?
	 * @param imageName The name of the cubemap.
	 * @param imageInfo The info used to create the image.
	 * @param imageData The concatenated data for all six cubemap layers.
	 * @param dataSize The size of the concatenated data.
	 */
	void allocateCubeImage(const std::string& imageName, const VkImageCreateInfo& imageInfo, const unsigned char* imageData, size_t dataSize);

	/**
	 * Adds a sampler for the given texture, allowing it to be used in shaders.
	 * @param texture The texture the sampler is for.
	 * @param sampler The sampler to add.
	 */
	void addSamplerForTexture(const std::string& texture, VkSampler sampler) { samplerMap.insert({texture, sampler}); }

	/**
	 * Creates a depth buffer. If there was already a depth buffer, it is destroyed and a new one is created.
	 * @param swapExtent The extent of the swapchain.
	 * @return The image view for the new depth buffer.
	 */
	VkImageView createDepthBuffer(VkExtent2D swapExtent);

protected:
	/**
	 * Creates a buffer and allocation with the given parameters.
	 * @param vertexFormat The format of the vertices in the buffer.
	 * @param usage The way the buffer is intended to be used - determines which memory type
	 *     it is stored in.
	 * @param size The size of the buffer to create.
	 * @return A pointer to a VkBufferData object.
	 * @throw std::runtime_error if out of memory.
	 */
	std::shared_ptr<Buffer> createBuffer(Buffer::Usage usage, BufferStorage storage, size_t size) override;

	/**
	 * Creates a type of uniform set for which descriptors can be allocated. Specifically, this creates the descriptor
	 * layout and adjusts the size of the descriptor pool to accommodate this set.
	 * @param name The name of the set.
	 * @param type The type of the set (material, screen, object).
	 * @param maxUsers The maximum allowed users of the set. Determines descriptor pool sizes.
	 * @param set The set itself. Mostly used for creating descriptor layouts.
	 */
	void createUniformSetType(const std::string& name, UniformSetType type, size_t maxUsers, const UniformSet& set) override { /** TODO **/ }

	/**
	 * Gets the minimum alignment for offsets into a uniform buffer.
	 * @return The minimum alignment.
	 */
	size_t getMinUniformBufferAlignment() override { return objects.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment; }

	/**
	 * Allocates a descriptor set for the material. This can be called more than once
	 * for the same material - subsequent calls should be ignored.
	 * @param model The material to allocate a descriptor set for.
	 */
	void addMaterialDescriptors(const Material& material) override { /** TODO **/ }

private:
	//Object handler for vulkan objects.
	VkObjectHandler& objects;
	//Allocator for the vertex buffers.
	VmaAllocator allocator;
	//Transfer buffer.
	VkBuffer transferBuffer;
	//Allocation for transfer buffer.
	VmaAllocation transferAllocation;
	//Transfer command buffer.
	VkCommandBuffer transferCommands;
	//Uniform buffers and allocations.
	//0 - static model, 1 - dynamic model, 2 - screen / object.
	std::array<VkBuffer, 3> uniformBuffers;
	std::array<VmaAllocation, 3> uniformBufferAllocations;
	//Fence to prevent overwriting in-transit memory.
	VkFence transferFence;
	//Current offset into transfer buffer, reset to 0 after each transfer operation.
	size_t transferOffset;
	//Size of the transfer buffer.
	size_t transferSize;
	//Indicates that the transfer buffer needs to be reallocated.
	bool growTransfer;
	//All queued transfer operations.
	std::queue<TransferOperation> pendingTransfers;
	//All queued image transfer operations.
	std::queue<ImageTransferOperation> pendingImageTransfers;
	//Map of uploaded mesh data.
	std::unordered_map<std::string, VkMeshRenderData> meshMap;
	//All possible descriptor set layouts.
	std::unordered_map<std::string, DescriptorLayoutInfo> descriptorLayouts;
	//Static model descriptor pool. Once allocated, descriptor sets are never freed or updated.
	VkDescriptorPool staticModelPool;
	//Dynamic model, object, and screen descriptor pool. All descriptor sets are allocated once at
	//pool creation time and then never updated (Unless something happens with textures, of course).
	VkDescriptorPool dynamicPool;
	//Contains all allocated descriptor sets. These are never freed until program termination.
	std::unordered_map<std::string, VkDescriptorSet> descriptorSets;
	//Stores one aligner for each per-screen or per-object descriptor set, to avoid dynamic allocation
	//inside the rendering loop.
	std::unordered_map<std::string, Std140Aligner> descriptorAligners;
	//Current offset into the object/screen uniform buffer, gets reset each frame.
	uint32_t currentUniformOffset;
	//Allowed usage size of the screen object buffer for each frame.
	size_t screenObjectBufferSize;
	//Map of all added samplers.
	std::unordered_map<std::string, VkSampler> samplerMap;
	//Map to insert loaded images into.
	std::unordered_map<std::string, std::shared_ptr<VkImageData>> imageMap;
	//Image for the depth buffer.
	VkImage depthBuffer;
	//Image view for depth buffer image.
	VkImageView depthView;
	//Allocation for depth buffer image.
	VmaAllocation depthAllocation;
	//Transfer buffer memory.
	unsigned char* transferMem;
	//Uniform stream buffer memory.
	unsigned char* uniformMem;

	/**
	 * Adds a transfer operation to the pending transfer queue.
	 * @param buffer The destination buffer for the transfer.
	 * @param offset The offset into the destination buffer to place the data.
	 * @param size The size of the data.
	 * @param data The data to transfer.
	 */
	void queueTransfer(VkBuffer buffer, size_t offset, size_t size, const unsigned char* data);

	/**
	 * Adds an image transfer operation to the pending image transfer queue.
	 * @param image The image to transfer the data to.
	 * @param size The size of the image data.
	 * @param data The image data to transfer.
	 * @param imageWidth The width of the image.
	 * @param imageHeight the height of the image.
	 * @param arrayLayers The number of layers to transfer.
	 */
	void queueImageTransfer(VkImage image, size_t size, const unsigned char* data, uint32_t imageWidth, uint32_t imageHeight, uint32_t arrayLayers);

	/**
	 * Creates a descriptor pool that has enough descriptors to hold the sets that setInPool
	 * returns true for.
	 * @param pool The descriptor pool to create.
	 * @param setInPool A function that determines in a given uniform set belongs in the pool
	 *     to be created.
	 */
	void createDescriptorPool(VkDescriptorPool* pool, std::function<bool(UniformSetType)> setInPool);

	/**
	 * Writes the descriptors to the descriptor set.
	 * @param set The set to write to.
	 * @param layoutInfo The descriptor layout info for the set.
	 * @param uniformSet The uniform set that corresponds to the layout info.
	 * @param textures A vector of image views for all the textures used in the set.
	 */
	void fillDescriptorSet(VkDescriptorSet set, const DescriptorLayoutInfo& layoutInfo, const UniformSet& uniformSet, const std::vector<VkImageView>& textures);

	/**
	 * Takes a uniform set type and turns it into an index. This function
	 * gives an odd feeling of deja vu...
	 * @param type The uniform buffer type.
	 * @return The index for the buffer.
	 */
	static constexpr size_t bufferIndexFromSetType(const UniformSetType type) {
		switch (type) {
			case UniformSetType::MATERIAL: return 0;
			case UniformSetType::PER_SCREEN: return 1;
			case UniformSetType::PER_OBJECT: return 1;
			//Should be optimized better than an exception?
			default: return 0xFFFFFFFFFFFFFFFF;
		}
	}
};
