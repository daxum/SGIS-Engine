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

#include "PhysDebRenderingEngine.hpp"
#include "PhysicsComponentManager.hpp"

namespace {
	const char* const lineVertShader = "\
		#version 410 core\n\
		layout (location = 0) in vec4 posIn;\n\
		layout (location = 1) in vec4 colorIn;\n\
		out vec4 color;\n\
		uniform mat4 modelView;\n\
		uniform mat4 projection;\n\
		void main() {\n\
			color = colorIn;\n\
			gl_Position = projection * modelView * posIn;\n\
		}";

	const char* const lineFragShader = "\
		#version 410 core\n\
		in vec4 color;\n\
		out vec4 outColor;\n\
		void main() {\n\
			outColor = color;\n\
		}";
}

PhysDebRenderingEngine::PhysDebRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog) :
	GlRenderingEngine(display, rendererLog),
	lineVao(0),
	lineVertexBuffer(0),
	lineProg(0),
	debugMode(btIDebugDraw::DBG_DrawWireframe),
	pendingLines(0),
	currentCamera(nullptr) {

}

PhysDebRenderingEngine::~PhysDebRenderingEngine() {
	glDeleteVertexArrays(1, &lineVao);
	glDeleteBuffers(1, &lineVertexBuffer);
	glDeleteProgram(lineProg);
}

void PhysDebRenderingEngine::init() {
	GlRenderingEngine::init();

	//Initialize line buffers
	glGenVertexArrays(1, &lineVao);
	glGenBuffers(1, &lineVertexBuffer);

	glBindVertexArray(lineVao);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, maxLines * 2 * sizeof(LineVert), nullptr, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(LineVert), (void*) offsetof(LineVert, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVert), (void*) (offsetof(LineVert, color)));

	glBindVertexArray(0);

	//Create line shader
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	if (vertShader == 0 || fragShader == 0) {
		throw std::runtime_error("Could not allocate physics debug shaders!");
	}

	glShaderSource(vertShader, 1, &lineVertShader, nullptr);
	glShaderSource(fragShader, 1, &lineFragShader, nullptr);

	//Compilation shouldn't fail here, because the shaders never change
	glCompileShader(vertShader);
	glCompileShader(fragShader);

	lineProg = glCreateProgram();

	if (lineProg == 0) {
		throw std::runtime_error("Failed to create physics debug shader program");
	}

	glAttachShader(lineProg, vertShader);
	glAttachShader(lineProg, fragShader);

	//Linking also shouldn't fail
	glLinkProgram(lineProg);

	glDetachShader(lineProg, vertShader);
	glDetachShader(lineProg, fragShader);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void PhysDebRenderingEngine::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
	LineVert vert = {
		.pos = glm::vec4(from.x(), from.y(), from.z(), 1.0f),
		.color = glm::vec4(color.x(), color.y(), color.z(), 1.0f)
	};

	if (pendingLines == lineBuffer.size()) {
		flushLines();
	}

	lineBuffer.at(pendingLines) = vert;
	vert.pos = glm::vec4(to.x(), to.y(), to.z(), 1.0f);
	lineBuffer.at(pendingLines + 1) = vert;
	pendingLines += 2;
}

void PhysDebRenderingEngine::flushLines() {
	if (pendingLines == 0) {
		return;
	}

	glBindVertexArray(lineVao);
	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);

	//Copy over line data

	void* buffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, pendingLines * sizeof(LineVert), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(buffer, lineBuffer.data(), pendingLines * sizeof(LineVert));
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//Draw lines

	glUseProgram(lineProg);

	glm::mat4 modelView = currentCamera->getView();
	glm::mat4 projection = currentCamera->getProjection();

	GLuint mvLoc = glGetUniformLocation(lineProg, "modelView");
	GLuint projLoc = glGetUniformLocation(lineProg, "projection");
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(modelView));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glDrawArrays(GL_LINES, 0, pendingLines);

	//Reset line buffer
	pendingLines = 0;
}

void PhysDebRenderingEngine::renderObjects(const ConcurrentRenderComponentSet& objects, RenderComponentManager::RenderPassList sortedObjects, const Screen* screen) {
	GlRenderingEngine::renderObjects(objects, sortedObjects, screen);

	//Get physics component manager and do debug drawing stuff
	const PhysicsComponentManager* physicsManager = std::static_pointer_cast<const PhysicsComponentManager>(screen->getManager(PHYSICS_COMPONENT_NAME)).get();

	if (physicsManager) {
		btCollisionWorld* world = physicsManager->getWorld();
		currentCamera = screen->getCamera().get();

		world->setDebugDrawer(this);
		world->debugDrawWorld();

		flushLines();
	}
}
