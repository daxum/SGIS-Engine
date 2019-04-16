set(ENGINE_FILES
	Engine.cpp
	TextureLoader.cpp
	ExtraMath.cpp
	Models/ModelLoader.cpp
	Models/Material.cpp
	DisplayEngine.cpp
	Screen.cpp
	RenderComponent.cpp
	ComponentManager.cpp
	AIComponentManager.cpp
	PhysicsComponentManager.cpp
	Renderer/MatrixStack.cpp
	PhysicsComponent.cpp
	Object.cpp
	RenderComponentManager.cpp
	InputHandler.cpp
	GuiComponentManager.cpp
	FontManager.cpp
	TextComponent.cpp
	SplineAnimation.cpp
	AnimationComponent.cpp
	Renderer/GlfwInterface.cpp
	Models/Vertex.cpp
	Renderer/MemoryAllocator.cpp
	Renderer/RendererMemoryManager.cpp
	Models/ModelManager.cpp
	Renderer/RenderingEngine.cpp
	Renderer/Std140Aligner.cpp
	PhysicsObject.cpp
	UpdateComponentManager.cpp
	UpdateComponent.cpp
	Models/VertexFormat.hpp
	Renderer/Buffer.cpp
)

if (USE_OPENGL)
	set(ENGINE_FILES ${ENGINE_FILES}
		Renderer/Opengl/GlRenderingEngine.cpp
		Renderer/Opengl/GlTextureLoader.cpp
		Renderer/Opengl/GlShaderLoader.cpp
		Renderer/Opengl/GlMemoryManager.cpp
		Renderer/Opengl/PhysDebRenderingEngine.cpp
		Renderer/Opengl/gl_core_4_1.c
	)

	#TODO: add_compile_definitions(USE_OPENGL) - cmake 3.12
	add_definitions(-DUSE_OPENGL)
endif()

if (USE_VULKAN)
	set(ENGINE_FILES ${ENGINE_FILES}
		Renderer/Vulkan/VkRenderingEngine.cpp
		Renderer/Vulkan/VkObjectHandler.cpp
		Renderer/Vulkan/VkExtensionFuncDefs.cpp
		Renderer/Vulkan/VkShaderLoader.cpp
		Renderer/Vulkan/VkShader.cpp
		Renderer/Vulkan/VkMemoryManager.cpp
		Renderer/Vulkan/VkPipelineCreateObject.cpp
		Renderer/Vulkan/VkImageData.cpp
		Renderer/Vulkan/VkTextureLoader.cpp
		Renderer/Vulkan/VkRenderObjects.cpp
		Renderer/Vulkan/VkBufferContainer.cpp
	)

	add_definitions(-DUSE_VULKAN)
endif()

add_library(Engine STATIC ${ENGINE_FILES})

set_target_properties(Engine PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if ((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(Engine PRIVATE "-Wall" "-Wignored-qualifiers")
endif()

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BULLET_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIR})
target_link_libraries(Engine glfw ${BULLET_LIBRARIES} freetype tbb)

if (USE_VULKAN)
	target_link_libraries(Engine vulkan)
endif()

if (USE_OPENGL)
	if (WIN32)
		#Never tested, but should probably work.
		#This *should* be the only change needed to compile on windows, but once again, completely untested.
		target_link_libraries(Engine opengl32.lib)
	else()
		#No mac for now, just assume linux.
		target_link_libraries(Engine GL)
	endif()
endif()

