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
    
    // Advanced SIMD operations
    void fast_memset_pattern(void* dest, int pattern, size_t count);
    size_t fast_string_compare_colors(const char* color1, const char* color2, size_t max_len);
    
    // Optimized box drawing
    void fast_draw_box_borders(char** cells, char** colors, int x, int y, int w, int h,
                              char corner_char, char horizontal_char, char vertical_char,
                              const char* color);
    
    // Cache optimization
    void prefetch_buffer_region(void* buffer, size_t size);
    
    // Advanced SIMD optimizations
    size_t fast_render_buffer_optimized(char* output_buffer, const char** cells, const char** colors, 
                                       size_t width, size_t height, size_t max_output_size);
    void fast_pattern_fill_avx2(void* dest, uint64_t pattern, size_t count);
    void fast_unicode_box_fill(char** cells, char** colors, int x, int y, int w, int h,
                               const char* fill_char, const char* color);
    
    // Advanced CPU feature detection
    struct CPUFeatures {
        bool sse2;
        bool sse4_1;
        bool avx;
        bool avx2;
        bool avx512f;
    };
    
    CPUFeatures detect_advanced_cpu_features();
}

// Fallback to C++ implementations if ASM not available
#ifndef USE_ASM_OPTIMIZATIONS
#define USE_ASM_OPTIMIZATIONS 1
#endif