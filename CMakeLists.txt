cmake_minimum_required(VERSION 3.16)

project(simple_db_backup LANGUAGES CXX)

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#! ! ! ! ! ! !
# Set this to ON to ship the tool
#! ! ! ! ! ! !
option(PRODUCTION_BUILD "Make this a production build" ON)

# IMPORTANT:
# This forces static CRT (/MT, /MTd).
# Use vcpkg triplet: x64-windows-static
# If you use x64-windows instead, remove this line.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(PRODUCTION_BUILD)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---- Sources ----
# DON'T ADD SOURCES BY HAND; this grabs all src/*.cpp recursively
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if(NOT MY_SOURCES)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

# ---- Executable ----
add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

# ---- Compiler options / defines ----
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /UUNICODE /U_UNICODE)
endif()

if(PRODUCTION_BUILD)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Ofast>
        $<$<CXX_COMPILER_ID:Clang>:-Ofast>
        $<$<CXX_COMPILER_ID:AppleClang>:-Ofast>
        $<$<CXX_COMPILER_ID:MSVC>:/O2 /fp:fast>
    )
endif()

if(PRODUCTION_BUILD)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        RESOURCES_PATH="./src/"
        PRODUCTION_BUILD=1
        DEVELOPLEMT_BUILD=0
    )
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/"
        PRODUCTION_BUILD=0
        DEVELOPLEMT_BUILD=1
    )
endif()

# ---- Dependencies ----
if(WIN32)
    set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL/18")
endif()
find_package(PostgreSQL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    PostgreSQL::PostgreSQL
)

# ---- Copy PostgreSQL DLLs next to the exe on Windows ----
if(WIN32)
    file(GLOB PG_DLLS "${PostgreSQL_ROOT}/bin/lib*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PG_DLLS} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND_EXPAND_LISTS
    )
endif()
