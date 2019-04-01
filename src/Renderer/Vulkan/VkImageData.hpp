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

#include "vk_mem_alloc.h"

//Currently for 2d textures only. Will change later using inheritance or something.
class VkImageData {
public:
	/**
	 * Creates the image data structure for the provided image.
	 * This will free the image when it is deleted.
	 * @param The allocator used to create the image.
	 * @param device The device the image was created with.
	 * @param image The image to manage.
	 * @param allocation The memory used to store the image.
	 * @param format The format of the managed image.
	 * @param cube Temporary hack until different texture types get sorted. True if image is a cube map.
	 */
	VkImageData(VmaAllocator allocator, VkDevice device, VkImage image, VmaAllocation allocation, VkFormat format, bool cube);

	/**
	 * Destructor - Destroys the image and image view.
	 */
	~VkImageData();

	/**
	 * It would be bad if these worked.
	 * Except the move ones, those would be fine.
	 */
	VkImageData(const VkImageData&) = delete;
	VkImageData(VkImageData&&) = delete;
	VkImageData& operator=(const VkImageData&) = delete;
	VkImageData& operator=(VkImageData&&) = delete;

	/**
	 * Gets the image view. If more than one image views are added later, this
	 * will be changed to be able to create / fetch a specific one.
	 * @return The image view.
	 */
	VkImageView getImageView() { return imageView; }

private:
	//Vulkan device.
	VkDevice device;

	//Memory management stuff.
	VmaAllocator allocator;
	VmaAllocation allocation;

	//The image.
	VkImage image;
	//Image view - might need more than one later.
	VkImageView imageView;
};
