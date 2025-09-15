#include "../include/asm_optimized.h"
#include <cstring>
#include <immintrin.h>

namespace ASMOptimized {

// CPU feature detection
bool has_sse2() {
    #ifdef __SSE2__
    return true;
    #else
    return false;
    #endif
}

bool has_avx() {
    #ifdef __AVX__
    return true;
    #else
    return false;
    #endif
}

bool has_avx2() {
    #ifdef __AVX2__
    return true;
    #else
    return false;
    #endif
}

// High-precision timing
uint64_t get_cpu_cycles() {
    #ifdef __x86_64__
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
    #else
    return 0;
    #endif
}

// Simplified render function (placeholder for now)
size_t fast_render_buffer(const RenderContext& ctx) {
    // For now, return 0 to indicate fallback to standard implementation
    return 0;
}

// SIMD-optimized mouse input parsing
MouseParseResult fast_parse_mouse_input(const char* buffer, size_t length) {
    MouseParseResult result = {false, false, 0, 0, 0};
    
    #ifdef __SSE2__
    // Use SSE2 to search for patterns in parallel
    const __m128i quit_pattern = _mm_set1_epi8('q');
    const __m128i quit_upper_pattern = _mm_set1_epi8('Q');
    const __m128i escape_pattern = _mm_set1_epi8('\033');
    
    size_t i = 0;
    for (; i + 16 <= length; i += 16) {
        __m128i chunk = _mm_loadu_si128((const __m128i*)(buffer + i));
        
        // Check for quit characters
        __m128i quit_cmp = _mm_cmpeq_epi8(chunk, quit_pattern);
        __m128i quit_upper_cmp = _mm_cmpeq_epi8(chunk, quit_upper_pattern);
        __m128i quit_result = _mm_or_si128(quit_cmp, quit_upper_cmp);
        
        if (_mm_movemask_epi8(quit_result) != 0) {
            result.found_quit = true;
            return result;
        }
        
        // Check for escape sequences
        __m128i escape_cmp = _mm_cmpeq_epi8(chunk, escape_pattern);
        int escape_mask = _mm_movemask_epi8(escape_cmp);
        
        if (escape_mask != 0) {
            result.found_escape = true;
            result.escape_pos = i + __builtin_ctz(escape_mask);
            break;
        }
    }
    
    // Handle remaining bytes
    for (; i < length; i++) {
        if (buffer[i] == 'q' || buffer[i] == 'Q') {
            result.found_quit = true;
            return result;
        }
        if (buffer[i] == '\033') {
            result.found_escape = true;
            result.escape_pos = i;
            break;
        }
    }
    #else
    // Fallback sequential search
    for (size_t i = 0; i < length; i++) {
        if (buffer[i] == 'q' || buffer[i] == 'Q') {
            result.found_quit = true;
            return result;
        }
        if (buffer[i] == '\033') {
            result.found_escape = true;
            result.escape_pos = i;
            break;
        }
    }
    #endif
    
    return result;
}

// Fast memory operations using SIMD
void fast_memset_pattern(void* dest, int pattern, size_t count) {
    #ifdef __SSE2__
    if (count >= 16) {
        __m128i pattern_vec = _mm_set1_epi8(pattern);
        char* ptr = (char*)dest;
        
        // Align to 16-byte boundary
        while ((uintptr_t)ptr % 16 != 0 && count > 0) {
            *ptr++ = pattern;
            count--;
        }
        
        // SIMD fill
        while (count >= 16) {
            _mm_store_si128((__m128i*)ptr, pattern_vec);
            ptr += 16;
            count -= 16;
        }
        
        // Handle remainder
        while (count > 0) {
            *ptr++ = pattern;
            count--;
        }
    } else {
        memset(dest, pattern, count);
    }
    #else
    memset(dest, pattern, count);
    #endif
}

// Placeholder implementations for compatibility
void fast_draw_horizontal_line(char** cells, char** colors, 
                              int x, int y, int width, 
                              const char* character, const char* color) {
    // Fallback implementation
    (void)cells; (void)colors; (void)x; (void)y; (void)width; (void)character; (void)color;
}

void fast_draw_vertical_line(char** cells, char** colors,
                            int x, int y, int height,
                            const char* character, const char* color) {
    // Fallback implementation  
    (void)cells; (void)colors; (void)x; (void)y; (void)height; (void)character; (void)color;
}

void fast_clear_buffer(char** cells, char** colors, 
                      size_t width, size_t height) {
    // Fallback implementation
    (void)cells; (void)colors; (void)width; (void)height;
}

} // namespace ASMOptimized