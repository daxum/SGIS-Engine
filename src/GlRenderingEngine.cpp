#include <vector>
#include "GlRenderingEngine.hpp"
#include "GlTextureLoader.hpp"

GlRenderingEngine::GlRenderingEngine() :
	RenderingEngine(std::make_shared<GlTextureLoader>(textureMap)),
	textureMap() {

	//TODO: glfw error callback

	if (!glfwInit()) {
		throw std::runtime_error("Couldn't initialize glfw");
	}
}

GlRenderingEngine::~GlRenderingEngine() {
	//Delete textures

	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<GLuint> namesToDelete(textureMap.size());

	size_t i = 0;
	for (const auto& tex : textureMap) {
		namesToDelete[i] = tex.second;
		i++;
	}

	glDeleteTextures(textureMap.size(), namesToDelete.data());

	//Delete window and terminate glfw

	if (window != nullptr) {
		glfwDestroyWindow(window);
		window = nullptr;
	}

	glfwTerminate();
}

void GlRenderingEngine::init(int windowWidth, int windowHeight, std::string windowTitle) {
	//Create the window

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	//TODO: remove once resizing is figured out
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);

	if (window == nullptr) {
		throw std::runtime_error("Failed to create window and context");
	}

	glfwMakeContextCurrent(window);

	//Load OpenGL functions

	int loadStatus = ogl_LoadFunctions();

	if (loadStatus != 1) {
		throw new std::runtime_error("OpenGl function loading failed");
	}

	//Set callbacks

	glfwSetFramebufferSizeCallback(window, GlRenderingEngine::setViewport);
	/*
	//Other callbacks - might be moved elsewhere to accommodate engine
	glfwSetKeyCallback(window, keyCallback);
	glfwSetScrollCallback(window, mouseScroll);
	glfwSetCursorPosCallback(window, cursorMove);
	glfwSetMouseButtonCallback(window, mouseClick);
	*/

	//Set state defaults

	int width = 0;
	int height = 0;

	glfwGetFramebufferSize(window, &width, &height);

	setViewport(nullptr, width, height);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
}

bool GlRenderingEngine::windowClosed() {
	return glfwWindowShouldClose(window);
}

void GlRenderingEngine::pollEvents() {
	glfwPollEvents();
}

void GlRenderingEngine::setViewport(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

	//TODO: Update projection matrix
}
