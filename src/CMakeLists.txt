find_package(glfw3 3.2 REQUIRED)

option(USE_INSTALLED_BULLET "Use a version of bullet installed on the system, instead of a local copy" OFF)

#Threading should probably be enabled in the bullet library, not sure if it works with it off. It'll definitely be
#slower if it does, though. If bullet is linked statically, some changes might be neccessary to its cmake files to
#avoid link errors (or I'm doing something horribly wrong).

if (USE_INSTALLED_BULLET)
	#2.88 for multithreaded solvers.
	find_package(Bullet 2.88 REQUIRED)
else()
	#Uses a copy of bullet from a directory called bullet, placed two levels above the engine source directory.
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../bullet ${CMAKE_CURRENT_BINARY_DIR}/bullet)
	set(BULLET_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../bullet/src)
	set(BULLET_LIBRARIES "LinearMath;Bullet3Common;BulletInverseDynamics;BulletCollision;BulletDynamics;BulletSoftBody")
endif()

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
target_link_libraries(Engine glfw ${BULLET_LIBRARIES})

if (WIN32)
	#Never tested, but should probably work.
	#This *should* be the only change needed to compile on windows, but once again, completely untested.
	target_link_libraries(Engine opengl32.lib)
else()
	#No mac for now, just assume linux.
	target_link_libraries(Engine GL)
endif()

