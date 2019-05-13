#Not a lot here at the moment, but at least we know the allocator works well.

add_executable(memAllocTest
	memAllocTest.cpp
	../src/Renderer/MemoryAllocator.cpp
	../src/ExtraMath.cpp
)

set_target_properties(memAllocTest PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(memAllocTest PRIVATE "-Wall")
endif()

#Now vertices get lightly tested too!

add_executable(vertexTest
	vertexTest.cpp
	../src/Models/Vertex.cpp
	../src/Models/VertexFormat.cpp
)

set_target_properties(vertexTest PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(vertexTest PRIVATE "-Wall")
endif()

#Logger optimization test, in an attempt to avoid macros.
#I blame to_string and string::operator+.

add_executable(loggerOverheadTest
	loggerOverheadTest.cpp
	../src/ExtraMath.cpp
)

set_target_properties(loggerOverheadTest PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(loggerOverheadTest PRIVATE "-Wall")
endif()

#Test for vertex hash function quality

add_executable(vertexHashTest
	vertexHashTest.cpp
	../src/ExtraMath.cpp
	../src/Models/Vertex.cpp
	../src/Models/VertexFormat.cpp
)

set_target_properties(vertexHashTest PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(vertexHashTest PRIVATE "-Wall")
endif()
