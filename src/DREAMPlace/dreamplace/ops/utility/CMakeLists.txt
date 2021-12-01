project(utility)

file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )
list(FILTER SOURCES EXCLUDE REGEX "dp_compute_wl.cpp")
file(GLOB DP_COMPUTE_WL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/dp_compute_wl.cpp") 

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

if (CMAKE_PLACE)
if (NOT CMAKE_CUDA_FLAGS)
    set(CMAKE_CUDA_FLAGS "-gencode=arch=compute_60,code=sm_60")
endif()

if (CUDA_FOUND)
    add_definitions(-D_CUDA_FOUND)
    file(GLOB CUDA_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cu"
        )
    set(CUDA_LINKED ${PROJECT_NAME}_cuda_linked)
    # try to turn on -fPIC for cuda, but not really working
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CUDA_SEPARABLE_COMPILATION TRUE)
    list(APPEND CUDA_NVCC_FLAGS
        -O3 -std=c++14 ${CMAKE_CUDA_FLAGS} 
        )
    list(APPEND CUDA_NVCC_FLAGS --compiler-options -fPIC)

    cuda_add_library(
        ${CUDA_LINKED} STATIC
        ${CUDA_SOURCES}
        )

    target_include_directories(
        ${CUDA_LINKED} PUBLIC 
        ${OPS_DIR}
	${LIMBO_SOURCE_DIR}
#        ${CUB_DIR}
#        ${Boost_INCLUDE_DIRS}
        )
    if (NOT ${CUDA_VERSION_MAJOR} VERSION_GREATER "10")
       target_include_directories(
           ${CUDA_LINKED} PUBLIC
           ${CUB_DIR}
           )
    endif()

    set_target_properties( ${CUDA_LINKED}
        PROPERTIES CUDA_SEPARABLE_COMPILATION ON
        )
endif(CUDA_FOUND)

add_library(dp_common STATIC ${DP_COMPUTE_WL_SRCS})
if(CUDA_FOUND)
    target_link_libraries(dp_common ${CUDA_LINKED})
endif(CUDA_FOUND)

endif(CMAKE_PLACE)
