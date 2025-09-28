cmake_minimum_required(VERSION 3.22)
set(LIBRARY_NAME beatnetlib)
set(APP_NAME beatnet_infer)
project(${LIBRARY_NAME})

set(CMAKE_CXX_STANDARD 17)

set (BEATNET_ONNX_ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to BeatNet/onnx directory")
set(BEATNET_CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

option(ENABLE_KISSFFT "Use the Kiss FFT library instead of FFTW3" OFF)
option(ENABLE_FFTW3 "Use the FFTW3 library instead of Kiss FFT" ON)
option(BUILD_APP "Build the test application using main.cpp" OFF)

if(ENABLE_KISSFFT AND ENABLE_FFTW3)
    message(FATAL_ERROR "ENABLE_KISSFFT and ENABLE_FFTW3 cannot both be ON. Choose one.")
elseif(NOT ENABLE_KISSFFT AND NOT ENABLE_FFTW3)
    message(FATAL_ERROR "No FFT backend selected.Please enable either ENABLE_KISSFFT or ENABLE_FFTW3.")
endif()

# include dependencies ( they are downloaded within libs dir if they do not exist)
include(${BEATNET_CMAKE_MODULE_PATH}/onnxruntime.cmake)
include(${BEATNET_CMAKE_MODULE_PATH}/libsamplerate.cmake)
if(ENABLE_FFTW3)
    message(STATUS "FFT backend: FFTW3")
    include(${BEATNET_CMAKE_MODULE_PATH}/fftw3.cmake)
elseif(ENABLE_KISSFFT)
    message(STATUS "FFT backend: KissFFT")
    include(${BEATNET_CMAKE_MODULE_PATH}/kissfft.cmake)
else()
    message(WARNING "No FFT backend selected!")
endif()

set(BEATNET_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ORT_DIR}/include
    ${LIBSAMPLERATE_DIR}/include
)

# if(WIN32)
#     set(BEATNET_SUBLIBS
#         ${ORT_DIR}/lib/onnxruntime.lib
#         ${LIBSAMPLERATE_DIR}/lib/samplerate.lib
#     )
# else()
#     set(BEATNET_SUBLIBS
#         ${ORT_DIR}/lib/libonnxruntime.dylib
#         ${LIBSAMPLERATE_DIR}/lib/libsamplerate.a
#     )
# endif()

if(ENABLE_FFTW3)

    if(WIN32)
        list(APPEND BEATNET_INCLUDE_DIRS ${FFTW3_DIR})
        # list(APPEND BEATNET_SUBLIBS "${FFTW3_DIR}/libfftw3f-3.lib")
    else()
        list(APPEND BEATNET_INCLUDE_DIRS ${FFTW3_DIR}/include)
        # list(APPEND BEATNET_SUBLIBS ${FFTW3_DIR}/lib/libfftw3f.a)
    endif()
elseif(ENABLE_KISSFFT)
    add_library(libkissfft STATIC
        ${KISSFFT_DIR}/kiss_fft.c
        ${KISSFFT_DIR}/kiss_fftr.c
    )
    list(APPEND BEATNET_INCLUDE_DIRS ${KISSFFT_DIR})
    # list(APPEND BEATNET_SUBLIBS libkissfft)
endif()

message("BEATNET_INCLUDE_DIRS ${BEATNET_INCLUDE_DIRS}")

set(LIB_SOURCE_FILES  
    BeatNet.cpp 
    resampler.cpp
    frameprocessor.cpp
    fftprocessor.cpp
    filterbankprocessor.cpp
    logspecutils.cpp
    dynamic_link.cpp
)

set(SOURCE_FILE
    main.cpp
)

add_library(${LIBRARY_NAME} STATIC ${LIB_SOURCE_FILES})

# make sure they are configured properly after downloading 
if(ENABLE_FFTW3)
    add_dependencies(${LIBRARY_NAME} fftw3_ready)
endif()
if (WIN32)
    add_dependencies(${LIBRARY_NAME} libsamplerate_ready)
endif()

target_include_directories(${LIBRARY_NAME} PUBLIC ${BEATNET_INCLUDE_DIRS})
# target_link_libraries(${LIBRARY_NAME} PUBLIC ${BEATNET_SUBLIBS})

if(ENABLE_FFTW3)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC ENABLE_FFTW3)
elseif(ENABLE_KISSFFT)
    target_link_libraries(${LIBRARY_NAME} PUBLIC libkissfft)
    target_compile_definitions(${LIBRARY_NAME} PUBLIC ENABLE_KISSFFT)
endif()

if(BUILD_APP)
    add_executable(${APP_NAME} ${SOURCE_FILE})
    target_link_libraries(${APP_NAME} PRIVATE ${LIBRARY_NAME})
endif()

function(copy_beatnet_deps target_name)

    set(LIBS_AND_WEIGHTS "")

    if(WIN32)
        list(APPEND LIBS_AND_WEIGHTS 
            "${ORT_DIR}/lib/onnnxruntime.dll"
            "${LIBSAMPLERATE_DIR}/bin/samplerate.dll"
        )
        if(ENABLE_FFTW3)
            list (APPEND LIBS_AND_WEIGHTS
                "${FFTW3_DIR}/libfftw3f-3.dll")
        endif()

    elseif(APPLE)
        file(GLOB SAMPLERATE_LIBS "${LIBSAMPLERATE_DIR}/lib/libsamplerate*.dylib*")
        list(APPEND LIBS_AND_WEIGHTS ${SAMPLERATE_LIBS})
        
        file(GLOB ORT_LIBS "${ORT_DIR}/lib/libonnxruntime*.dylib*")
        list(APPEND LIBS_AND_WEIGHTS ${ORT_LIBS})
    
        if(ENABLE_FFTW3)
            file(GLOB FFTW3_LIBS "${FFTW3_DIR}/lib/libfftw3f*.dylib*")
            list(APPEND LIBS_AND_WEIGHTS ${FFTW3_LIBS})
        endif()
    elseif(UNIX)
        file(GLOB SAMPLERATE_LIBS 
            "${LIBSAMPLERATE_DIR}/lib/libsamplerate.so*"
            "${LIBSAMPLERATE_DIR}/lib64/libsamplerate.so*")
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

    foreach(DEP_FILE IN LISTS LIBS_AND_WEIGHTS)
        add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DEP_FILE}"
                $<TARGET_FILE_DIR:${LIBRARY_NAME}>)
    endforeach()

endfunction(copy_beatnet_deps target_name)

copy_beatnet_deps(${PROJECT_NAME})