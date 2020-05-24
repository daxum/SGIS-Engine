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

#include "GlRenderingEngine.hpp"
#include "LinearMath/btIDebugDraw.h"

class PhysDebRenderingEngine : public GlRenderingEngine, public btIDebugDraw {
public:
	//The maximum number of lines that can be stored before the buffer needs to be flushed.
	static const size_t maxLines = 512;

	/**
	 * Created a physics debugging rendering engine, forwards almost all
	 * construction to GlRenderingEngine.
	 * @param display A reference to the engine's display engine, used for callbacks.
	 * @param rendererLog The logger config for the rendering engine.
	 * @throw runtime_error if glfw initialization failed.
	 */
	PhysDebRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog);

	/**
	 * Deletes all the junk.
	 */
	~PhysDebRenderingEngine();

	/**
	 * Initializes OpenGL as in GlRenderingEngine, but also creates buffers to store
	 * the lines for physics debugging.
	 */
	void init() override;

	/**
	 * Called from bullet to draw a line.
	 * @param from The starting point of the line.
	 * @param to The ending point of the line.
	 * @param color The color of the line.
	 */
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

	/**
	 * Draws a point, by drawing a line with from == to. Might need to mess with line width later,
	 * or switch points to their own buffers, if this is hard to see or something.
	 * @param point The point being draw.
	 * @param normal Presumably the normal for the collision?
	 * @param distance ...?
	 * @param lifetime Might be how long the objects have been colliding...
	 * @param color The color of the object.
	 */
	void drawContactPoint(const btVector3& point, const btVector3& normal, btScalar distance, int lifeTime, const btVector3& color) override {
		//Maybe draw a line from the point along the normal?
		drawLine(point, point, color);
	}

	/**
	 * These two operate on the mysterious "debugMode" variable.
	 */
	void setDebugMode(int mode) override { debugMode = mode; }
	int getDebugMode() const override { return debugMode; }

	/**
	 * Renders all pending lines.
	 */
	void flushLines() override;

	/**
	 * Reports any errors bullet might have.
	 * @param warning The message from bullet.
	 */
	void reportErrorWarning(const char* warning) override {
		ENGINE_LOG_WARN(logger, std::string("Physics debug rendering warning: ") + warning);
	}

	/**
	 * Required by btIDebugDraw, but would require some serious hackery with the font
	 * manager to implement, so left blank for now.
	 */
	void draw3dText(const btVector3& location, const char* textString) override {}

protected:
	/**
	 * Renders the objects as normal, but also renders a physics debug layer if present.
	 * @param sortedObjects A sorted map of maps of maps of sets of all the possible objects to render.
	 * @param screen The screen being rendered.
	 */
	void renderObjects(RenderManager::RenderPassList sortedObjects, const Screen* screen) override;

private:
	//Line vertex format.
	struct LineVert {
		glm::vec4 pos;
		glm::vec4 color;
	};

	//Render objects, holds line data, shaders, etc.
	GLuint lineVao;
	GLuint lineVertexBuffer;
	GLuint lineProg;

	//Something from bullet. No idea if actually used.
	int debugMode;

	//Number of lines waiting to be drawn.
	size_t pendingLines;
	//List of line vertices to be uploaded.
	std::array<LineVert, maxLines * 2> lineBuffer;

	//Camera of the screen currently being rendered. Hopefully can find
	//a way around this later.
	const Camera* currentCamera;
};
