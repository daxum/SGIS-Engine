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
#include <cstdint>

//Vulkan Physical Device Properties Decoder

/**
 * Returns the name for the pci vendor id. This is a completely pointless
 * function, but why not?
 * All values were taken from <https://pcisig.com/membership/member-companies/>.
 * @param id The pci vendor id from vkPhysicalDeviceProperties.
 * @return The name of the pci vendor.
 */
constexpr const char* getPciVendorName(uint32_t id) {
	switch (id) {
		case 0x00001002: return "Advanced Micro Devices";
		case 0x000013B5: return "ARM Ltd.";
		//Membership table didn't specify if this was hex or decimal, so going with hex.
		case 0x00008086: return "Intel Corporation";
		case 0x000010DE: return "NVidia Corporation";
		case 0x00005143: return "Qualcomm Inc.";
		default: return "Unknown Vendor";
	}
}

/**
 * Returns the name of the device type. Mostly.
 * @param type The device type to get the name of.
 * @return A string representing the name of the device type.
 */
constexpr const char* getDeviceTypeName(VkPhysicalDeviceType type) {
	switch (type) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "...Virtual GPU?";
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return "...a CPU? Really?";
		default: return "getDeviceTypeName needs updating for new devices!";
	}
}
