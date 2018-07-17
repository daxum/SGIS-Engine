cmake_minimum_required(VERSION 3.1)
project(SGIS-Engine)

option(BUILD_ENGINE_TESTS "Build the test programs for the engine" OFF)

option(USE_OPENGL "Build the engine with OpenGL support" ON)
option(USE_VULKAN "Build the engine with Vulkan support" ON)

option(USE_INSTALLED_BULLET "Use a version of bullet installed on the system, instead of a local copy" ON)
option(USE_INSTALLED_GLFW "Use a version of glfw installed on the system" ON)

set(BULLET_DIR "" CACHE STRING "Bullet repository directory, used if linking bullet statically")
set(GLFW_DIR "" CACHE STRING "GLFW repository directory, used if linking statically")

set(FREETYPE_INCLUDE_DIR "" CACHE STRING "Freetype2 include directory")

if (BUILD_ENGINE_TESTS)
	add_subdirectory(tests)
endif()

#Threading should probably be enabled in the bullet library, not sure if it works with it off. It'll definitely be
#slower if it does, though. If bullet is linked statically, some changes might be neccessary to its cmake files to
#avoid link errors (or I'm doing something horribly wrong).

if (USE_INSTALLED_BULLET)
	#2.88 for multithreaded solvers.
	find_package(Bullet 2.88 REQUIRED)
else()
	if ("${BULLET_DIR}" STREQUAL "")
		message(FATAL_ERROR "Local bullet set, but path not specified")
	else()
		add_subdirectory(${BULLET_DIR} ${CMAKE_CURRENT_BINARY_DIR}/bullet)
		set(BULLET_INCLUDE_DIRS ${BULLET_DIR}/src)
		set(BULLET_LIBRARIES "LinearMath;Bullet3Common;BulletInverseDynamics;BulletCollision;BulletDynamics;BulletSoftBody")
	endif()
endif()

if (USE_INSTALLED_GLFW)
	find_package(glfw3 3.2 REQUIRED)
else()
	if ("${GLFW_DIR}" STREQUAL "")
		message(FATAL_ERROR "Local glfw set, but path not specified")
	else()
		add_subdirectory(${GLFW_DIR}; ${CMAKE_CURRENT_BINARY_DIR}/glfw)
	endif()
endif()

add_subdirectory(src)
