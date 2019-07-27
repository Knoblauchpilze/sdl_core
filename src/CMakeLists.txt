project (sdl_core)

set (CMAKE_CXX_STANDARD 14)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package (SDL2 REQUIRED)
find_package (SDL2_ttf REQUIRED)

set (SOURCES
  SizePolicy.cc
  FocusPolicy.cc
  Layout.cc
  LayoutItem.cc
  SdlWidget.cc
  UserInputFilter.cc
  )

add_library (sdl_core SHARED
  ${SOURCES}
  ${SDL2_INCLUDE_DIR}
  ${SDL2_TTF_INCLUDE_DIRS}
  )

set(SDL_CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

target_link_libraries(sdl_core
  ${SDL2_LIBRARY}
  ${SDL2_TTF_LIBRARIES}
  core_utils
  )

target_include_directories (sdl_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${SDL2_TTF_INCLUDE_DIRS}
  )

