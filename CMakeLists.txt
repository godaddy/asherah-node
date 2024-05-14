cmake_minimum_required(VERSION 3.21)
project(asherah-node)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/dist)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

# main
add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCE_DIR}/src/asherah.cc ${PROJECT_SOURCE_DIR}/src/logging.cc ${PROJECT_SOURCE_DIR}/src/logging_napi.cc)
target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DNAPI_CPP_EXCEPTIONS
        -DNODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS
        -DNODE_ADDON_API_DISABLE_DEPRECATED
        -DNODE_API_NO_EXTERNAL_BUFFERS_ALLOWED
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib/libasherah.a)
target_compile_options(${PROJECT_NAME} PUBLIC
        -fexceptions
        -g
        -O3
        -fPIC
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-c99-extensions
        -Wno-unknown-pragmas
)

# set filename
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "asherah" PREFIX "" SUFFIX ".node")

# dependencies
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/third_party/node-addon-api)
    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party)
    set(NPM_CMD npm)
    if(WIN32)
        # workaround for windows, without this the npm command will returns immediately
        set(NPM_CMD cmd /Q /C npm)
    endif()
    execute_process(
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party
        COMMAND ${NPM_CMD} init -y
    )
    # fetch packages using npm, then move them to ./third_party
    execute_process(
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party
            COMMAND ${NPM_CMD} i node-addon-api node-api-headers --save false
    )
    file(RENAME ${PROJECT_SOURCE_DIR}/third_party/node_modules/node-addon-api ${PROJECT_SOURCE_DIR}/third_party/node-addon-api)
    file(RENAME ${PROJECT_SOURCE_DIR}/third_party/node_modules/node-api-headers ${PROJECT_SOURCE_DIR}/third_party/node-api-headers)
    file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/third_party/node_modules)
    file(REMOVE ${PROJECT_SOURCE_DIR}/third_party/package.json)
    execute_process(
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party/node-api-headers
            COMMAND node -e "let a=require('./symbols.js').v8;console.log('LIBRARY node.exe\\nEXPORTS\\n'+[...a.node_api_symbols,...a.js_native_api_symbols].join('\\n'))"
            OUTPUT_FILE libnode.def
    )
endif()
target_include_directories(
        ${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/lib
        ${PROJECT_SOURCE_DIR}/third_party/node-addon-api
        ${PROJECT_SOURCE_DIR}/third_party/node-api-headers/include
)
# platform special options
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DGCC_ENABLE_CPP_EXCEPTIONS)
    target_link_options(${PROJECT_NAME} PUBLIC -undefined dynamic_lookup)
elseif(MSVC)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/third_party/node-api-headers/libnode.lib)
        execute_process(
                COMMAND ${CMAKE_AR} /def:libnode.def /out:libnode.lib
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party/node-api-headers
        )
    endif()
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/third_party/node-api-headers/libnode.lib)
elseif(MINGW)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/third_party/node-api-headers/libnode.a)
        execute_process(
                COMMAND dlltool -d libnode.def -l libnode.a
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party/node-api-headers
        )
    endif()
    target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/third_party/node-api-headers/libnode.a)
endif()