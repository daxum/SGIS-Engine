set(ENGINE_FILES
	Engine.cpp
	TextureLoader.cpp
	ExtraMath.cpp
	ModelLoader.cpp
	Model.cpp
	DisplayEngine.cpp
	Screen.cpp
	RenderComponent.cpp
	ComponentManager.cpp
	AIComponentManager.cpp
	PhysicsComponentManager.cpp
	AxisAlignedBB.cpp
	MatrixStack.cpp
	UpdateComponentManager.cpp
	PhysicsComponent.cpp
	Object.cpp
	BoxPhysicsObject.cpp
	RenderComponentManager.cpp
	InputHandler.cpp
	GuiComponentManager.cpp
	FontManager.cpp
	TextComponent.cpp
	SplineAnimation.cpp
	AnimationComponent.cpp
	GlfwInterface.cpp
	Vertex.cpp
	VertexBuffer.cpp
	MemoryAllocator.cpp
	RendererMemoryManager.cpp
	ModelManager.cpp
	RenderingEngine.cpp
)

if (USE_OPENGL)
	set(ENGINE_FILES ${ENGINE_FILES}
		GlRenderer/GlRenderingEngine.cpp
		GlRenderer/GlTextureLoader.cpp
		GlRenderer/GlShader.cpp
		GlRenderer/GlShaderLoader.cpp
		GlRenderer/GlMemoryManager.cpp
		GlRenderer/gl_core_4_1.c
	)
endif()

if (USE_VULKAN)
	set(ENGINE_FILES ${ENGINE_FILES}
		VkRenderer/VkRenderingEngine.cpp
		VkRenderer/VkObjectHandler.cpp
		VkRenderer/VkExtensionFuncDefs.cpp
		VkRenderer/VkShaderLoader.cpp
		VkRenderer/VkShader.cpp
	)
endif()

add_library(Engine STATIC ${ENGINE_FILES})

set_target_properties(Engine PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Engine PRIVATE "-Wall")
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

