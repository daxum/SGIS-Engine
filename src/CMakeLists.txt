find_package(glfw3 3.2 REQUIRED)
find_package(Bullet REQUIRED)

add_library(Engine
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
	Camera.cpp
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
	GlRenderer/gl_core_4_1.c
)

set_target_properties(Engine PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Engine PRIVATE "-Wall")
endif()

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BULLET_INCLUDE_DIRS})
target_link_libraries(Engine glfw GL ${BULLET_LIBRARIES})
