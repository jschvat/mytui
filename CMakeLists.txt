cmake_minimum_required(VERSION 3.12)
project(ModernTUI VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# ASM optimization flags
option(ENABLE_ASM_OPTIMIZATIONS "Enable SIMD assembly optimizations" ON)
option(ENABLE_AVX2 "Enable AVX2 optimizations (requires compatible CPU)" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)

if(ENABLE_ASM_OPTIMIZATIONS)
    add_compile_definitions(USE_ASM_OPTIMIZATIONS=1)
    
    # Check for SIMD support
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)
    check_cxx_compiler_flag("-mavx" COMPILER_SUPPORTS_AVX)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    
    set(SIMD_FLAGS "")
    if(COMPILER_SUPPORTS_SSE2)
        set(SIMD_FLAGS "${SIMD_FLAGS} -msse2")
        message(STATUS "SSE2 support: enabled")
    endif()
    
    if(COMPILER_SUPPORTS_AVX)
        set(SIMD_FLAGS "${SIMD_FLAGS} -mavx")
        message(STATUS "AVX support: enabled")
    endif()
    
    if(COMPILER_SUPPORTS_AVX2 AND ENABLE_AVX2)
        set(SIMD_FLAGS "${SIMD_FLAGS} -mavx2")
        message(STATUS "AVX2 support: enabled")
    endif()
    
    message(STATUS "ASM optimizations: enabled")
    message(STATUS "SIMD flags: ${SIMD_FLAGS}")
else()
    message(STATUS "ASM optimizations: disabled")
endif()

# Include directories
include_directories(include)

# Library source files
set(TUI_SOURCES
    src/buffer.cpp
    src/mouse_handler.cpp
    src/tui_app.cpp
    src/window.cpp
    src/dropdown_menu.cpp
    src/button.cpp
    src/event_system.cpp
    src/progress_bar.cpp
    src/checkbox.cpp
    src/text_input.cpp
    src/password_input.cpp
    src/radio_buttons.cpp
    src/status_bar.cpp
    src/list_box.cpp
    src/asm_optimized.cpp
)

set(TUI_HEADERS
    include/buffer.h
    include/colors.h
    include/mouse_handler.h
    include/tui_app.h
    include/window.h
    include/dropdown_menu.h
    include/button.h
    include/event_system.h
    include/asm_optimized.h
)

# Create static library
add_library(tui STATIC ${TUI_SOURCES} ${TUI_HEADERS})

# Apply ASM optimizations to specific files
if(ENABLE_ASM_OPTIMIZATIONS AND SIMD_FLAGS)
    set_source_files_properties(
        src/asm_optimized.cpp
        PROPERTIES COMPILE_FLAGS "${SIMD_FLAGS}"
    )
endif()

# Set library properties
set_target_properties(tui PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${TUI_HEADERS}"
)

# Examples
if(BUILD_EXAMPLES)
    # Basic demo
    add_executable(demo examples/demo.cpp)
    target_link_libraries(demo tui)
    
    # ASM showcase
    add_executable(asm_showcase examples/asm_showcase.cpp)
    target_link_libraries(asm_showcase tui)
    
    # ASM demo (interactive)
    add_executable(asm_demo examples/asm_demo.cpp)
    target_link_libraries(asm_demo tui)
    
    # Dropdown menu demo
    add_executable(dropdown_demo examples/dropdown_demo.cpp)
    target_link_libraries(dropdown_demo tui)
    
    # Simple button component demo
    add_executable(simple_button_demo examples/simple_button_demo.cpp)
    target_link_libraries(simple_button_demo tui)
    
    # Event system demo
    add_executable(event_system_demo examples/event_system_demo.cpp)
    target_link_libraries(event_system_demo tui)
    
    # UI Components demo
    add_executable(ui_components_demo examples/ui_components_demo.cpp)
    target_link_libraries(ui_components_demo tui)
    
    message(STATUS "Examples: enabled")
else()
    message(STATUS "Examples: disabled")
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    add_executable(simple_benchmark examples/simple_benchmark.cpp)
    target_link_libraries(simple_benchmark tui)
    
    message(STATUS "Benchmarks: enabled")
else()
    message(STATUS "Benchmarks: disabled")
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS tui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tui
)

if(BUILD_EXAMPLES)
    install(TARGETS demo asm_showcase asm_demo
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(BUILD_BENCHMARKS)
    install(TARGETS simple_benchmark
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Package configuration
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tui-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tui-config.cmake
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tui-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tui
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "ModernTUI")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance terminal user interface framework with SIMD optimizations")
set(CPACK_PACKAGE_VENDOR "Modern TUI Framework")
set(CPACK_PACKAGE_CONTACT "moderntui@example.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# Custom targets
add_custom_target(format
    COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/examples -name "*.cpp" -o -name "*.h" | xargs clang-format -i
    COMMENT "Formatting code with clang-format"
)

add_custom_target(check
    COMMAND cppcheck --enable=all --inconclusive --std=c++11 ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include 2>/dev/null || echo "cppcheck not available"
    COMMENT "Running static analysis with cppcheck"
)

# Print build configuration summary
message(STATUS "")
message(STATUS "===== Build Configuration =====")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "ASM optimizations: ${ENABLE_ASM_OPTIMIZATIONS}")
message(STATUS "Examples: ${BUILD_EXAMPLES}")
message(STATUS "Benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "===============================")
message(STATUS "")