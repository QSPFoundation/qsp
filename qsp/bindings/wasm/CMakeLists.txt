if (NOT EMSCRIPTEN)
	message(FATAL_ERROR "The WebAssembly module can only be built with the Emscripten toolchain")
endif()
if (NOT BUILD_STATIC)
	message(FATAL_ERROR "The WebAssembly module requires BUILD_STATIC")
endif()

target_sources(qsp PRIVATE
	qsp_wasm.h
	wasm_callbacks.c
	wasm_control.c
)

set_property(TARGET qsp PROPERTY C_STANDARD 11)
target_compile_definitions(qsp PRIVATE _WASM)
target_compile_options(qsp PRIVATE -g) # for the source map of the debug module, stripped from the release

set(QSP_WASM_LINK_OPTIONS
	-sEXPORT_ES6
	-sEXPORT_NAME=createQspModule
	-sASYNCIFY
	-sASYNCIFY_STACK_SIZE=1MB
	-sSTACK_SIZE=5MB
	-sALLOW_MEMORY_GROWTH
	-sEXPORTED_FUNCTIONS=_malloc,_free
	-sEXPORTED_RUNTIME_METHODS=ccall,HEAP8,HEAPU8,HEAPU16,HEAP32,HEAPU32
	-sINCOMING_MODULE_JS_API=wasmBinary # not in the defaults since Emscripten 6
	-sFILESYSTEM=0
	-sDYNAMIC_EXECUTION=0
	"SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/qsp-wasm-host.js" # host calls, see wasm_callbacks.c
)

# The modules are linked from the library objects to keep the exports of the binding
add_executable(qsp-engine $<TARGET_OBJECTS:qsp>)
target_link_libraries(qsp-engine PRIVATE qsp)
target_link_options(qsp-engine PRIVATE ${QSP_WASM_LINK_OPTIONS} -O3)

add_executable(qsp-engine-debug $<TARGET_OBJECTS:qsp>)
target_link_libraries(qsp-engine-debug PRIVATE qsp)
target_link_options(qsp-engine-debug PRIVATE ${QSP_WASM_LINK_OPTIONS} -O1 -gsource-map -sASSERTIONS -sSTACK_OVERFLOW_CHECK=2)

set_property(TARGET qsp-engine qsp-engine-debug APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qsp-wasm-host.js")

install(FILES
		"$<TARGET_FILE:qsp-engine>"
		"$<TARGET_FILE_DIR:qsp-engine>/qsp-engine.wasm"
		"$<TARGET_FILE:qsp-engine-debug>"
		"$<TARGET_FILE_DIR:qsp-engine-debug>/qsp-engine-debug.wasm"
		"$<TARGET_FILE_DIR:qsp-engine-debug>/qsp-engine-debug.wasm.map"
		qsp-engine.d.ts
		qsp-engine-debug.d.ts
		qsp-wasm-module.ts
		qsp-wasm-memory.ts
	DESTINATION .
	COMPONENT wasm
)
