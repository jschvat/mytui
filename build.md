# Build Instructions

## CMake Build System

The Modern TUI framework supports both traditional Makefile and modern CMake build systems.

### Quick Start (CMake)

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Build Options

Configure the build with CMake options:

```bash
# Debug build with all features
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_ASM_OPTIMIZATIONS=ON \
      -DENABLE_AVX2=ON \
      -DBUILD_EXAMPLES=ON \
      -DBUILD_BENCHMARKS=ON \
      ..

# Release build optimized for performance
cmake -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_ASM_OPTIMIZATIONS=ON \
      -DENABLE_AVX2=ON \
      ..

# Minimal build without optimizations
cmake -DENABLE_ASM_OPTIMIZATIONS=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_BENCHMARKS=OFF \
      ..
```

### Available Targets

```bash
make tui              # Build static library
make demo             # Build interactive demo
make asm_showcase     # Build ASM demonstration
make simple_benchmark # Build performance benchmarks
make install          # Install system-wide
make package          # Create distribution package
make format           # Format code (requires clang-format)
make check            # Static analysis (requires cppcheck)
```

### Traditional Makefile

For compatibility, the traditional Makefile is still available:

```bash
make                  # Build everything
make optimized        # Build with ASM optimizations
make debug           # Build debug version
make clean           # Clean build files
```

### Cross-Platform Support

The CMake build system supports:
- Linux (GCC, Clang)
- macOS (Clang, Apple Clang)
- Windows (MSVC, MinGW)
- Various architectures (x86_64, ARM64)

### Requirements

- CMake 3.12 or later
- C++11 compatible compiler
- Optional: clang-format (for code formatting)
- Optional: cppcheck (for static analysis)

### IDE Integration

The CMake build system provides excellent IDE support:

```bash
# Visual Studio Code
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# CLion / Qt Creator
# Open CMakeLists.txt directly

# Visual Studio
cmake -G "Visual Studio 16 2019" ..
```