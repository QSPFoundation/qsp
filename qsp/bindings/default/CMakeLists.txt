include(GenerateExportHeader)

target_sources(qsp PRIVATE
	default_callbacks.c
	default_control.c
	qsp_default.h
)

generate_export_header(qsp
	BASE_NAME Qsp
	EXPORT_MACRO_NAME QSP_EXTERN
)

if (BUILD_STATIC)
	target_compile_definitions(qsp PUBLIC QSP_STATIC_DEFINE)
endif()

target_include_directories(qsp
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
	INTERFACE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
		"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/qsp/bindings>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qsp>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qsp/default>"
)

install(FILES
		../qsp.h
		../bindings_config.h
	DESTINATION
		"${CMAKE_INSTALL_INCLUDEDIR}/qsp"
)
install(FILES
		qsp_default.h
		"${CMAKE_CURRENT_BINARY_DIR}/qsp_export.h"
	DESTINATION
		"${CMAKE_INSTALL_INCLUDEDIR}/qsp/default"
)
