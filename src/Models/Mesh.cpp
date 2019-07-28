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

#include "Mesh.hpp"
#include "ModelManager.hpp"

Mesh::Mesh(BufferInfo bufferInfo, const VertexFormat* format, std::vector<unsigned char>&& vertices, std::vector<uint32_t>&& indices, const Aabb<float>& box, float radius) :
	vertexData(std::move(vertices)),
	indices(std::move(indices)),
	bufferInfo(bufferInfo),
	format(format),
	box(box),
	radius(radius),
	indexStart(0),
	vertexOffset(0) {

}

MeshRef::~MeshRef() {
	manager->removeMeshReference(meshName, level);
}
