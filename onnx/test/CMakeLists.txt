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

function(copy_beatnet_deps target_name)

    set(LIBS_AND_WEIGHTS "")

    if(WIN32)
        list(APPEND LIBS_AND_WEIGHTS 
            "${ORT_DIR}/lib/onnxruntime.dll"
            "${SAMPLERATE_DIR}/bin/samplerate.dll"
        )
        if(ENABLE_FFTW3)
            list (APPEND LIBS_AND_WEIGHTS
                "${FFTW3_DIR}/libfftw3f-3.dll")
        endif()

    elseif(APPLE)
        file(GLOB SAMPLERATE_LIBS "${SAMPLERATE_DIR}/lib/libsamplerate*.dylib*")
        list(APPEND LIBS_AND_WEIGHTS ${SAMPLERATE_LIBS})
        
        file(GLOB ORT_LIBS "${ORT_DIR}/lib/libonnxruntime*.dylib*")
        list(APPEND LIBS_AND_WEIGHTS ${ORT_LIBS})
    
        if(ENABLE_FFTW3)
            file(GLOB FFTW3_LIBS "${FFTW3_DIR}/lib/libfftw3f*.dylib*")
            list(APPEND LIBS_AND_WEIGHTS ${FFTW3_LIBS})
        endif()
    elseif(UNIX)
        file(GLOB SAMPLERATE_LIBS 
            "${SAMPLERATE_DIR}/lib/libsamplerate.so*"
            "${SAMPLERATE_DIR}/lib64/libsamplerate.so*")
        list(APPEND LIBS_AND_WEIGHTS ${SAMPLERATE_LIBS})
        
        file(GLOB ORT_LIBS "${ORT_DIR}/lib/libonnxruntime.so*")
        list(APPEND LIBS_AND_WEIGHTS ${ORT_LIBS})
    
        if(ENABLE_FFTW3)
            file(GLOB FFTW3_LIBS 
                "${FFTW3_DIR}/lib/libfftw3f.so*"
                "${FFTW3_DIR}/lib64/libfftw3f.so*")
            list(APPEND LIBS_AND_WEIGHTS ${FFTW3_LIBS})
        endif()
    endif()

    list (APPEND LIBS_AND_WEIGHTS "${BEATNET_ONNX_ROOTDIR}/beatnet_bda.onnx")

    set(LIBRARY_NAME beatnetlib)

    foreach(DEP_FILE IN LISTS LIBS_AND_WEIGHTS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DEP_FILE}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach()

endfunction(copy_beatnet_deps target_name)

copy_beatnet_deps(${PROJECT_NAME})