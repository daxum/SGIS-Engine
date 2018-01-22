cmake_minimum_required(VERSION 3.1)
project(SGIS-Engine)

add_library(Engine
	Engine.cpp
	TextureLoader.cpp
	GlRenderingEngine.cpp
	GlTextureLoader.cpp
	gl_core_4_1.c
)

set_target_properties(Engine PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(Engine PRIVATE "-Wall")
endif()

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
