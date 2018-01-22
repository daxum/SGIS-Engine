#pragma once

#include <memory>
#include <string>
#include "TextureLoader.hpp"

//A generic rendering engine. Provides the base interfaces, like resource loading
//and rendering, but leaves the implementation to api-specific subclasses, like
//a GlRenderingEngine, or a VulkanRenderingEngine (or maybe even a DxRenderingEngine.
//Unlikely, though). For the purpose of optimization, most functions should probably
//be as vague as possible - so they would be like "render all this stuff, I don't
//care how", and then the rendering engine would figure it out. To facilitate this,
//things like world objects and menus should NOT contain rendering code, but rather
//information on how to render them. Hopefully this strategy won't be too restrictive.
class RenderingEngine {
public:
	/**
	 * Constructs the base rendering engine and initializes some parts of
	 * the rendering api.
	 * @param tl The texture loader this engine should use.
	 * @throw runtime_error if initialization failed.
	 */
	RenderingEngine(std::shared_ptr<TextureLoader> tl) : texLoader(tl) {}

	/**
	 * Destroys any api-agnostic resources the engine might
	 * have created.
	 */
	virtual ~RenderingEngine() {}

	/**
	 * Initialize the rendering engine. Initialization usually consists of:
	 *  - creating a window
	 *  - allocating device resources
	 *  - creating swapchains, framebuffers, and other nonsense
	 *  - registering callbacks
	 * @param windowWidth The width of the created window.
	 * @param windowHeight The height of the created window.
	 * @param windowTitle The title of the created window.
	 * @throw runtime_error if initialization failed.
	 */
	virtual void init(int windowWidth, int windowHeight, std::string windowTitle) = 0;

	/**
	 * Gets the texture loader for this rendering engine, which is
	 * used to transfer textures from the disk to the GPU. Some loaders
	 * might also have the option of storing loaded textures in RAM.
	 * @return The texture loader for this rendering engine.
	 */
	std::shared_ptr<TextureLoader> getTextureLoader() { return texLoader; }

	/**
	 * Returns the api-specific model loader for this rendering engine.
	 * Not currently implemented.
	 */
	void getModelLoader() {}

	//TODO: custom shader loader - also need a way to specify uniforms and
	//such for them from world info.

	/**
	 * Indicates whether the window was closed by the user, and the
	 * game should stop.
	 * @return Whether to terminate the game due to a closed window.
	 */
	virtual bool windowClosed() = 0;

	/**
	 * Called from the engine to poll for events. This will usually
	 * call glfwPollEvents().
	 */
	virtual void pollEvents() = 0;

protected:
	std::shared_ptr<TextureLoader> texLoader;
};
