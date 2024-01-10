
target_include_directories (sdl_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	)

target_sources (sdl_core PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/SizePolicy.cc
	${CMAKE_CURRENT_SOURCE_DIR}/Layout.cc
	${CMAKE_CURRENT_SOURCE_DIR}/LayoutItem.cc
	${CMAKE_CURRENT_SOURCE_DIR}/SdlWidget.cc
	)
