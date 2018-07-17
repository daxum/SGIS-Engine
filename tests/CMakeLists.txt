#Not a lot here at the moment, but at least we know the allocator works well.

add_executable(memAllocTest
	memAllocTest.cpp
	../src/MemoryAllocator.cpp
	../src/ExtraMath.cpp
)

set_target_properties(memAllocTest PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(memAllocTest PRIVATE "-Wall")
endif()
