# CMake setup
cmake_minimum_required (VERSION 3.9)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

# Project name
project(OpenEDI)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CMAKE_CXX_ABI "Whether turn on CCXX_ABI" 1)
if(NOT CMAKE_CXX_ABI)
    set(CMAKE_CXX_ABI 0 CACHE STRING
        "Choose the value for _GLIBCXX_USE_CXX11_ABI, options are: 0|1."
        FORCE)
endif(NOT CMAKE_CXX_ABI)
# openEDI need new lib
message(STATUS "CMAKE_CXX_ABI: _GLIBCXX_USE_CXX11_ABI=${CMAKE_CXX_ABI}")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=${CMAKE_CXX_ABI})

# dev tool path
#if (DEFINED ENV{DEV_TOOL})   #find env
#    MESSAGE(STATUS "Found DEV_TOOL env = " $ENV{DEV_TOOL})
#    set(OPT_PATH $ENV{DEV_TOOL})
#else (NOT DEFINED ENV{DEV_TOOL})  #find defautl
#    set (OPT_PATH "/home/opt/devtoolset")
#endif()

# set(CMAKE_CXX_CPPLINT "/home/opt/Anaconda3-5.2.0-Linux-x86_64/bin/cpplint"; --linelength=120 --verbose=3)

#MESSAGE(STATUS "DEV_TOOL path: " ${OPT_PATH})

# set Boost path
if (DEFINED ENV{BOOST_DIR})   #find env
    set(Boost_DIR $ENV{BOOST_DIR})
    set(Boost_INCLUDE_DIRS ${Boost_DIR}/include)
    set(Boost_LIBRARY_DIRS ${Boost_DIR}/lib)
else ()
    find_package(Boost 1.55.0 REQUIRED)
endif ()
message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")

# set Tcl path
if (DEFINED ENV{TCL_DIR})   #find env
    set(TCL_DIR $ENV{TCL_DIR})
    set(TCL_INCLUDE_PATH ${TCL_DIR}/include)
    set(TCL_LIBRARY ${TCL_DIR}/lib)
else (NOT DEFINED ENV{DEV_TOOL})   #find env
    find_package(TCL 8.6.9 REQUIRED)
endif ()
message(STATUS "TCL_INCLUDE_PATH = ${TCL_INCLUDE_PATH}")
message(STATUS "TCL_LIBRARY = ${TCL_LIBRARY}")

# set install to default when it's not setted by user
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Prefix prepended to current source directories" FORCE )
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# set gperf path
if (DEFINED ENV{PROFILER_DIR})   #find env
    set(PROFILER_DIR $ENV{PROFILER_DIR})
endif ()
message(STATUS "PROFILER_DIR = ${PROFILER_DIR}")

# set qt5 path
find_package( Qt5Core )
find_package( Qt5Gui )
find_package( Qt5Widgets )
message(STATUS "Qt5Gui_DIR = ${Qt5Gui_DIR}")
message(STATUS "Qt5Core_DIR = ${Qt5Core_DIR}")
message(STATUS "Qt5Widgets_DIR = ${Qt5Widgets_DIR}")

find_package(BISON 3.0 REQUIRED)
find_package(FLEX REQUIRED)
# use Doxygen to generate documentation 
find_package(Doxygen)

#option(EDI_BIND_PYTHON "whether bind python" ON)
#if (TCL_FOUND)
#  message(STATUS "TCL_INCLUDE_PATH = ${TCL_INCLUDE_PATH}")
#  set(EDI_BIND_TCL ON)
#  #option(EDI_BIND_TCL "whether bind tcl" OFF)
#else(TCL_FOUND)
#  set(EDI_BIND_TCL OFF)
#endif(TCL_FOUND)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wreturn-type  -Wno-attributes")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-DDEBUGVERSION ${CMAKE_CXX_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}")

endif()

message(STATUS "CMAKE_HOST_SYSTEM: ${CMAKE_HOST_SYSTEM}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS})
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message(STATUS "EDI_BIND_PYTHON: ${EDI_BIND_PYTHON}")
message(STATUS "EDI_BIND_TCL: ${EDI_BIND_TCL}")

# without this, clang will complain about linking 
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
# testing are defined in unittest directory 
enable_testing()

OPTION(ENABLE_GCOV "Enable gcov (debug, Linux builds only)" OFF)
message(STATUS "GCOV: ${ENABLE_GCOV}")
IF (ENABLE_GCOV)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()

option(CMAKE_USE_PYTHON2 "Whether use python2 instead of python3" OFF)
if(CMAKE_USE_PYTHON2)
  find_package(PythonInterp 2)
else(CMAKE_USE_PYTHON2)
  find_package(PythonInterp 3)
  # if not found, search python2 instead 
  if (NOT PYTHONINTERP_FOUND)
    find_package(PythonInterp 2)
  endif(NOT PYTHONINTERP_FOUND)
endif(CMAKE_USE_PYTHON2)
add_subdirectory(thirdparty)

# python version is determined by pybind11 in thirdparty 
message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_VERSION: ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
option(CMAKE_PLACE "Whether compile place" TRUE)

add_subdirectory(src)
add_subdirectory(unittest)
add_subdirectory(demo)
add_subdirectory(docs)

# create an empty init script for python module  
# this is to support python 2.7, while python3 no longer needs this
install(CODE "execute_process(COMMAND touch ${CMAKE_INSTALL_PREFIX}/lib/__init__.py)")
