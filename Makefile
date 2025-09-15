# Modern TUI Framework Makefile
# Project: mytui - A clean, class-based terminal user interface framework

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = -Iinclude
SRCDIR = src
INCLUDEDIR = include
EXAMPLEDIR = examples
BUILDDIR = build
LIBDIR = lib

# Library name
LIBNAME = libtui.a

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
HEADERS = $(wildcard $(INCLUDEDIR)/*.h)

# Example files
EXAMPLE_SOURCES = $(wildcard $(EXAMPLEDIR)/*.cpp)
EXAMPLES = $(EXAMPLE_SOURCES:$(EXAMPLEDIR)/%.cpp=$(BUILDDIR)/%)

# Default target
all: directories $(LIBDIR)/$(LIBNAME) $(EXAMPLES)

# Create necessary directories
directories:
	@mkdir -p $(BUILDDIR) $(LIBDIR)

# Build static library
$(LIBDIR)/$(LIBNAME): $(OBJECTS)
	@echo "Creating static library $@"
	@ar rcs $@ $^

# Compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Build examples
$(BUILDDIR)/%: $(EXAMPLEDIR)/%.cpp $(LIBDIR)/$(LIBNAME)
	@echo "Building example $@"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $< -L$(LIBDIR) -ltui -o $@

# Run the demo example
demo: $(BUILDDIR)/demo
	@echo "Running demo..."
	@./$(BUILDDIR)/demo

# Install headers and library (optional)
install: all
	@echo "Installing to /usr/local..."
	@sudo mkdir -p /usr/local/include/tui /usr/local/lib
	@sudo cp $(HEADERS) /usr/local/include/tui/
	@sudo cp $(LIBDIR)/$(LIBNAME) /usr/local/lib/

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILDDIR) $(LIBDIR)

# Clean everything including backup files
distclean: clean
	@echo "Deep cleaning..."
	@find . -name "*~" -delete
	@find . -name "*.bak" -delete

# Development helpers
debug: CXXFLAGS += -g -DDEBUG
debug: all

release: CXXFLAGS += -DNDEBUG
release: all

# Show project info
info:
	@echo "Modern TUI Framework"
	@echo "===================="
	@echo "Source files: $(SOURCES)"
	@echo "Headers: $(HEADERS)"
	@echo "Examples: $(EXAMPLE_SOURCES)"
	@echo "Build directory: $(BUILDDIR)"
	@echo "Library: $(LIBDIR)/$(LIBNAME)"

# Format code (requires clang-format)
format:
	@echo "Formatting code..."
	@find $(SRCDIR) $(INCLUDEDIR) $(EXAMPLEDIR) -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check for common issues
check:
	@echo "Running static analysis..."
	@cppcheck --enable=all --inconclusive --std=c++11 $(SRCDIR) $(INCLUDEDIR) 2>/dev/null || echo "cppcheck not available"

.PHONY: all directories demo install clean distclean debug release info format check