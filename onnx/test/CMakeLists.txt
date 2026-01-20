cmake_minimum_required(VERSION 3.22)
project(cpp_test)
set(CMAKE_CXX_STANDARD 17)

include(cmake/cpm.cmake)

set(BEATNET_ONNX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(BEATNET_ONNX_TEMP_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/BeatNetOnnxBuild)
add_subdirectory(${BEATNET_ONNX_DIR} ${BEATNET_ONNX_TEMP_OUT_DIR})

# download adamstark/AudioFile
set(AUDIOFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/AudioFile)
if(NOT EXISTS ${AUDIOFILE_DIR}/CMakeLists.txt)
	CPMAddPackage(
		NAME AudioFile
		GITHUB_REPOSITORY adamstark/AudioFile
		GIT_TAG 1.1.4
		VERSION 1.1.4
		SOURCE_DIR ${AUDIOFILE_DIR}
	)
else()
	add_subdirectory(${AUDIOFILE_DIR})
	message(STATUS "AudioFile already exists, skipping download.")
endif()

set(ONNX_TEST_SOURCE_FILES
	testCPP.cpp
)

add_executable(${PROJECT_NAME} ${ONNX_TEST_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} beatnetlib)

# add_custom_target(CopyFiles ALL
#     COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     "${BEATNET_ONNX_TEMP_OUT_DIR}/$<CONFIG>"
#     "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
# )