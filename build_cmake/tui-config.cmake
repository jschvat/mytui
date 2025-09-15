# ModernTUI CMake Configuration File

set(TUI_VERSION "1.0.0")

# Find dependencies
find_package(Threads REQUIRED)

# Import targets
if(NOT TARGET tui::tui)
    include("${CMAKE_CURRENT_LIST_DIR}/tui-targets.cmake")
endif()

# Set variables for compatibility
set(TUI_LIBRARIES tui::tui)
set(TUI_INCLUDE_DIRS "/usr/local/include/tui")
set(TUI_FOUND TRUE)

# Check for ASM optimization support
if(ON)
    set(TUI_ASM_OPTIMIZATIONS TRUE)
else()
    set(TUI_ASM_OPTIMIZATIONS FALSE)
endif()

# Provide information about the installation
message(STATUS "Found ModernTUI: ${TUI_VERSION}")
if(TUI_ASM_OPTIMIZATIONS)
    message(STATUS "  ASM optimizations: enabled")
else()
    message(STATUS "  ASM optimizations: disabled")
endif()
