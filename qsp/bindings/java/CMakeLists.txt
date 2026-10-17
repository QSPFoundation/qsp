target_sources(qsp PRIVATE
	java_callbacks.c
	java_control.c
	java_files.c
	qsp_java.h
)

set_property(TARGET qsp PROPERTY C_STANDARD 11)
target_compile_definitions(qsp PRIVATE _JAVA)

if (NOT ANDROID)
	find_package(JNI)
	target_include_directories(qsp PRIVATE ${JNI_INCLUDE_DIRS})
	target_link_libraries(qsp PRIVATE ${JNI_LIBRARIES})
endif()
