# 用于构建 pjsua2 的 python c++ 绑定

cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ version selection")  # or 11, 14, 17, 20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # optional, ensure standard is supported
set(CMAKE_CXX_EXTENSIONS OFF)  # optional, keep compiler extensions off

cmake_policy(SET CMP0148 NEW)

# only for MSVC til now
set(PJ_DIR "" CACHE STRING "Directory of pjproject source code")
option(PJ_DYNAMIC "link to pjproject dynamic(static is default)" OFF)
option(
    PJ_REBUILD
    "re-build pjproject before build the project, only available on windows/msvc"
    OFF
)

add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>")

project(pjsua2 LANGUAGES CXX)

if("${PJ_DIR}" STREQUAL "")
    set(PJ_DIR "$ENV{PJ_DIR}")
endif()
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC AND "${PJ_DIR}" STREQUAL "")
    message(
        FATAL_ERROR
        "Viarialbe or environment ‘PJ_DIR’ is REQUIRED for ${CMAKE_CXX_COMPILER_ID}"
    )
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
    message(
        "set CMAKE_CONFIGURATION_TYPES=“${CMAKE_CONFIGURATION_TYPES}” on ${CMAKE_CXX_COMPILER_ID}. CMAKE_BUILD_TYPE is ignored!"
    )
else()
    find_package(PkgConfig REQUIRED)
endif()

# Find Python
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
# Find PyBind11
find_package(pybind11 CONFIG)
if(NOT ${pybind11_FOUND})
    message(
        "pybind11 not found, try execute `python -m pybind11 --cmakedir` ..."
    )
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE pybind11_config_retcode
        OUTPUT_VARIABLE pybind11_DIR
    )
    if("${pybind11_config_retcode}" STREQUAL "0")
        message(
            "pybind11-config returns: "
            ${pybind11_DIR}
            ". Try to find pybind11 again"
        )
        find_package(pybind11 CONFIG REQUIRED)
    else()
        message(
            FATAL_ERROR
            "pybind11-config failed: "
            ${pybind11_config_retcode}
        )
    endif()
endif()

# Pybind11 模块项目
pybind11_add_module(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/py_pjsua2.cc)

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC AND NOT ${PJ_DYNAMIC})
        message("MSVC_RUNTIME_LIBRARY set /MT (for Static link)")
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
    endif()

    set(PJ_INCLUDE_DIRS
        ${PJ_DIR}/pjlib/include
        ${PJ_DIR}/pjlib-util/include
        ${PJ_DIR}/pjmedia/include
        ${PJ_DIR}/pjsip/include
        ${PJ_DIR}/pjnath/include
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE "PJ_WIN32")
    target_include_directories(${PROJECT_NAME} PRIVATE ${PJ_INCLUDE_DIRS})

    target_link_directories(${PROJECT_NAME} PRIVATE ${PJ_DIR}/lib)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL AMD64)
            set(PJ_VS_CPU x86_64)
            set(PJ_VS_PLATFORM x64)
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL X86)
            set(PJ_VS_CPU i386)
            set(PJ_VS_PLATFORM Win32)
        else()
            message(
                FATAL_ERROR
                "${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR} is not support yet"
            )
        endif()

        if(${PJ_DYNAMIC})
            set(PJ_VS_CONFIGRATION $<CONFIG>-Dynamic)
        else()
            set(PJ_VS_CONFIGRATION $<CONFIG>-Static)
        endif()

        set(PJ_LIB
            libpjproject-${PJ_VS_CPU}-${PJ_VS_PLATFORM}-vc14-${PJ_VS_CONFIGRATION}
        )
        message("PJ_LIB: ${PJ_LIB}")
        message("pjproject VS Configuration: ${PJ_VS_CONFIGRATION}")
        target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE ${PJ_LIB}
            PUBLIC wsock32
            PUBLIC ws2_32
            PUBLIC ole32
            PUBLIC dsound
        )

        if(${PJ_REBUILD})
            message("pjproject will be built before ${PROJECT_NAME}")
            add_custom_command(
                TARGET ${PROJECT_NAME}
                PRE_BUILD
                COMMAND
                    msbuild pjproject-vs14.sln -target:pjsua -m
                    -property:Configuration=${PJ_VS_CONFIGRATION}
                    -property:Platform=${PJ_VS_PLATFORM}
                WORKING_DIRECTORY ${PJ_DIR}
                COMMENT "MSBuild pjproject"
            )
        endif()
    endif()
else()
    pkg_check_modules(LIBPJPROJECT REQUIRED "libpjproject")
    target_include_directories(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBPJPROJECT_LIBRARIES})
    target_link_options(${PROJECT_NAME} PRIVATE ${LIBPJPROJECT_LINK_OPTIONS})
endif()

# Sets the correct extension (with SOABI) for a target.
pybind11_extension(${PROJECT_NAME})

if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(${PROJECT_NAME})
endif()

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_VISIBILITY_PRESET "hidden" CUDA_VISIBILITY_PRESET "hidden"
)

# 强行：复制到子项目的目录，便于配合 virtual-env 调试，无论 Release 还是 Debug
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_CURRENT_LIST_DIR}/python
    COMMENT
        "copying $<TARGET_FILE:${PROJECT_NAME}> to ${CMAKE_CURRENT_LIST_DIR}/python"
)
