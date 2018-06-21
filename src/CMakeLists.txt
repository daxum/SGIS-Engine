add_library(Engine STATIC
	Engine.cpp
	TextureLoader.cpp
	GlRenderer/GlRenderingEngine.cpp
	GlRenderer/GlTextureLoader.cpp
	ExtraMath.cpp
	GlRenderer/GlShader.cpp
	GlRenderer/GlShaderLoader.cpp
	ModelLoader.cpp
	GlRenderer/GlMemoryManager.cpp
	Model.cpp
	Logger.cpp
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
	GlRenderer/gl_core_4_1.c
)

set_target_properties(Engine PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Engine PRIVATE "-Wall")
endif()

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BULLET_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIR})
target_link_libraries(Engine glfw ${BULLET_LIBRARIES} freetype tbb)

if (WIN32)
	#Never tested, but should probably work.
	#This *should* be the only change needed to compile on windows, but once again, completely untested.
	target_link_libraries(Engine opengl32.lib)
else()
	#No mac for now, just assume linux.
	target_link_libraries(Engine GL)
endif()

