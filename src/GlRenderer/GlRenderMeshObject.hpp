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

#include "Model.hpp"

//The type of mesh, affects mesh storage
enum MeshType {
	//Static meshes will be uploaded to gpu memory, for fast access by the gpu. Their buffers
	//will never be mapped. In addition, their vertices may be combined with the vertices of other
	//static meshes to save memory.
	STATIC = 0,
	//Text meshes.
	DYNAMIC_TEXT,
	//The total number of buffers, not actually a type of buffer.
	BUFFER_COUNT
};

class GlRenderMeshObject : public RenderMeshObject {
public:
	//Which buffer the mesh is in.
	MeshType type;
	//The starting position (byte offset) in the index buffer.
	size_t indexStart;
	//The number of indices in the mesh.
	size_t indexCount;
	//Pointer to renderer memory manager, used for freeing.
	GlMemoryManager* manager;

	GlRenderMeshObject(MeshType type, size_t indexStart, size_t indexCount, GlMemoryManager* manager) :
		type(type), indexStart(indexStart), indexCount(indexCount), manager(manager) {}

	~GlRenderMeshObject() {
		if (type == DYNAMIC_TEXT) {
			manager->freeTextMesh(model.mesh);
		}
	}
};
