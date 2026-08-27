file(READ "${CMAKE_CURRENT_LIST_DIR}/pack.json" PACK_JSON)
string(JSON PACK_NAME GET "${PACK_JSON}" name)

if(NOT TARGET rigComponent)
	if(DEFINED RIGKIT_ROOT AND NOT RIGKIT_ROOT STREQUAL "")
		set(_rigComponent_dir "${RIGKIT_ROOT}/packs/rigComponent")
	else()
		set(_rigComponent_dir "${CMAKE_SOURCE_DIR}/packs/rigComponent")
	endif()
	if(EXISTS "${_rigComponent_dir}/CMakeLists.txt")
		add_subdirectory("${_rigComponent_dir}" "${CMAKE_BINARY_DIR}/packs/rigComponent")
	else()
		message(FATAL_ERROR "rigObj requires rigComponent under packs/rigComponent")
	endif()
endif()

add_library(${PACK_NAME} STATIC
	src/rigObj.cpp
	src/rigObj.h
	src/RenderSettings.h
	src/ObjIO.cpp
	src/ObjIO.h
)
target_link_libraries(${PACK_NAME} PUBLIC rigkit rigComponent)
target_include_directories(${PACK_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
)
target_include_directories(${PACK_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/third_party/tinyobjloader
)
message(STATUS "Configured pack: ${PACK_NAME} (OBJ via tinyobjloader)")
