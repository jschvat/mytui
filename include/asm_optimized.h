#pragma once

#include <cstddef>
#include <cstdint>

// Assembly-optimized functions for performance-critical operations
namespace ASMOptimized {
    
    // Fast buffer rendering with SIMD optimizations
    struct RenderContext {
        const char** cells;      // 2D array of cell characters
        const char** colors;     // 2D array of color codes
        char* output_buffer;     // Pre-allocated output buffer
        size_t width, height;
        size_t max_output_size;
    };
    
    // SIMD-optimized rendering (SSE2/AVX)
    size_t fast_render_buffer(const RenderContext& ctx);
    
    // Mouse input processing optimizations
    struct MouseParseResult {
        bool found_quit;
        bool found_escape;
        size_t escape_pos;
        size_t mouse_data_start;
        size_t mouse_data_end;
    };
    
    // Vectorized mouse input parsing
    MouseParseResult fast_parse_mouse_input(const char* buffer, size_t length);
    
    // Box drawing optimizations
    void fast_draw_horizontal_line(char** cells, char** colors, 
                                  int x, int y, int width, 
                                  const char* character, const char* color);
    
    void fast_draw_vertical_line(char** cells, char** colors,
                                int x, int y, int height,
                                const char* character, const char* color);
    
    // Memory operations
    void fast_clear_buffer(char** cells, char** colors, 
                          size_t width, size_t height);
    
    // CPU feature detection
    bool has_sse2();
    bool has_avx();
    bool has_avx2();
    
    // Performance measurement
    uint64_t get_cpu_cycles();
}

// Fallback to C++ implementations if ASM not available
#ifndef USE_ASM_OPTIMIZATIONS
#define USE_ASM_OPTIMIZATIONS 1
#endif