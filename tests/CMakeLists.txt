add_executable(qsp-tests runner/qsp_test_runner.c)
set_target_properties(qsp-tests PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_link_libraries(qsp-tests PRIVATE qsp)

# One CTest test per scenario file
file(GLOB QSP_TEST_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.qspt")
foreach (testFile ${QSP_TEST_FILES})
	get_filename_component(testName "${testFile}" NAME_WE)
	add_test(NAME "engine.${testName}" COMMAND qsp-tests "${testFile}")
endforeach()
