
set (CMAKE_CXX_STANDARD 14)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

set (SOURCES
  SizePolicy.cc
  Layout.cc
  LayoutItem.cc
  SdlWidget.cc
  )

add_library (sdl_core SHARED
  ${SOURCES}
  )

set(SDL_CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

target_link_libraries(sdl_core
  core_utils
  sdl_engine
  )

target_include_directories (sdl_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

