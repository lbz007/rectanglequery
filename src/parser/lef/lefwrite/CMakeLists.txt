# 

file(GLOB_RECURSE SRCS lefwriteCbs.cpp)

set(_SUBDIRNAME lefwrite)
set(_SUBLIBNAME ${PROJECT_NAME_LOWERCASE}_${_SUBDIRNAME})
set(_SUBBINNAME lefwrite)

add_library(${_SUBLIBNAME} STATIC ${SRCS})
add_executable(${_SUBBINNAME} lefwrite.cpp)
target_link_libraries(${_SUBBINNAME} ${_SUBLIBNAME} ${PROJECT_NAME_LOWERCASE}_lef)
target_include_directories(${_SUBLIBNAME} PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../lef")

# must turn fPIC on 
set_target_properties(${_SUBLIBNAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

install(TARGETS ${_SUBLIBNAME} 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static
  )

# install headers 
file(GLOB_RECURSE HEADERS *.h *.hpp)
install(FILES ${HEADERS} DESTINATION include/src/parser/lef/${_SUBDIRNAME})

unset(_SUBDIRNAME)
unset(_SUBLIBNAME)
unset(_SUBBINNAME)
