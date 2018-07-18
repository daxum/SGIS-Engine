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

#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer(const std::vector<VertexElement>& vertexFormat, size_t size, BufferUsage usage, std::shared_ptr<RenderBufferData> renderData) :
	vertexElements(),
	vertexSize(0),
	bufferSize(size),
	usage(usage),
	renderData(renderData) {

	size_t totalSize = 0;

	for (const VertexElement& element : vertexFormat) {
		VertexElementData data = {
			element.type,
			totalSize,
			sizeFromVertexType(element.type)
		};

		totalSize += data.size;

		vertexElements.insert({element.name, data});
	}

	vertexSize = totalSize;
}
