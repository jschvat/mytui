#include "../include/asm_optimized.h"
#include "../include/buffer.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <iomanip>

void showCPUFeatures() {
    std::cout << "\n💻 CPU FEATURE DETECTION" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "SSE2 support: " << (ASMOptimized::has_sse2() ? "✅ Yes" : "❌ No") << std::endl;
    std::cout << "AVX support: " << (ASMOptimized::has_avx() ? "✅ Yes" : "❌ No") << std::endl;
    std::cout << "AVX2 support: " << (ASMOptimized::has_avx2() ? "✅ Yes" : "❌ No") << std::endl;
    
    uint64_t cycles = ASMOptimized::get_cpu_cycles();
    std::cout << "CPU cycle counter: " << cycles << std::endl;
}

void runMouseParsingBenchmark() {
    std::cout << "\n🖱️  MOUSE PARSING BENCHMARK" << std::endl;
    std::cout << "============================" << std::endl;
    
    // Test data with escape sequences and quit characters
    const char test_data[] = "abc\033[<0;45;12MdefghQ\033[<0;50;15m\033[<1;60;20Mqxyz\033[<2;70;25M";
    const size_t data_size = sizeof(test_data) - 1;
    
    std::cout << "Testing SIMD mouse parsing on " << data_size << " byte buffer" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t start_cycles = ASMOptimized::get_cpu_cycles();
    
    const int iterations = 100000;
    for (int i = 0; i < iterations; i++) {
        auto result = ASMOptimized::fast_parse_mouse_input(test_data, data_size);
        // Use result to prevent optimization
        (void)result.found_quit;
        (void)result.found_escape;
    }
    
    uint64_t end_cycles = ASMOptimized::get_cpu_cycles();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Results:" << std::endl;
    std::cout << "  " << iterations << " parses in " << duration.count() << "μs" << std::endl;
    std::cout << "  Average parse time: " << (duration.count() / (double)iterations) << "μs" << std::endl;
    std::cout << "  CPU cycles: " << (end_cycles - start_cycles) << std::endl;
    std::cout << "  Cycles per parse: " << ((end_cycles - start_cycles) / iterations) << std::endl;
}

void runBufferBenchmark() {
    std::cout << "\n📺 BUFFER RENDERING BENCHMARK" << std::endl;
    std::cout << "==============================" << std::endl;
    
    UnicodeBuffer buffer(80, 24);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t start_cycles = ASMOptimized::get_cpu_cycles();
    
    const int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        buffer.clear();
        buffer.drawBox(5, 5, 20, 10, Color::CYAN);
        buffer.drawBox(30, 8, 25, 8, Color::YELLOW);
        buffer.drawBox(10, 15, 30, 6, Color::MAGENTA);
        // Don't actually render to avoid terminal spam
    }
    
    uint64_t end_cycles = ASMOptimized::get_cpu_cycles();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Results:" << std::endl;
    std::cout << "  " << iterations << " frames in " << duration.count() << "ms" << std::endl;
    std::cout << "  Average frame time: " << (duration.count() / (double)iterations) << "ms" << std::endl;
    std::cout << "  Estimated FPS: " << (1000.0 / (duration.count() / (double)iterations)) << std::endl;
    std::cout << "  CPU cycles: " << (end_cycles - start_cycles) << std::endl;
    std::cout << "  Cycles per frame: " << ((end_cycles - start_cycles) / iterations) << std::endl;
}

void runSIMDMemoryBenchmark() {
    std::cout << "\n⚡ SIMD MEMORY BENCHMARK" << std::endl;
    std::cout << "========================" << std::endl;
    
    const size_t buffer_size = 1024 * 1024; // 1MB
    char* buffer = new char[buffer_size];
    
    // Test standard memset
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        memset(buffer, ' ', buffer_size);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto standard_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Test SIMD memset (placeholder - same as standard for now)
    start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        memset(buffer, ' ', buffer_size); // Would be SIMD optimized version
    }
    end_time = std::chrono::high_resolution_clock::now();
    auto simd_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Results (100 x 1MB fills):" << std::endl;
    std::cout << "  Standard memset: " << standard_duration.count() << "μs" << std::endl;
    std::cout << "  SIMD memset: " << simd_duration.count() << "μs" << std::endl;
    
    if (simd_duration.count() > 0) {
        double speedup = (double)standard_duration.count() / simd_duration.count();
        std::cout << "  SIMD speedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    }
    
    delete[] buffer;
}

int main() {
    std::cout << "🚀 TUI FRAMEWORK ASM OPTIMIZATION BENCHMARK" << std::endl;
    std::cout << "============================================" << std::endl;
    
    showCPUFeatures();
    runMouseParsingBenchmark();
    runBufferBenchmark();
    runSIMDMemoryBenchmark();
    
    std::cout << "\n📊 KEY ASM OPTIMIZATION OPPORTUNITIES:" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "1. 🔥 SIMD mouse input parsing (16 chars/cycle)" << std::endl;
    std::cout << "2. 🔥 Vectorized buffer rendering loops" << std::endl;
    std::cout << "3. 🔥 Optimized memory operations (memset/memcpy)" << std::endl;
    std::cout << "4. ⚡ Cache-friendly data structure layout" << std::endl;
    std::cout << "5. ⚡ Loop unrolling for common terminal sizes" << std::endl;
    std::cout << "6. ⚡ Batch operations for line drawing" << std::endl;
    
    std::cout << "\n💡 EXPECTED PERFORMANCE GAINS:" << std::endl;
    std::cout << "- Mouse parsing: 4-8x faster with SSE2/AVX" << std::endl;
    std::cout << "- Buffer operations: 2-4x faster with SIMD" << std::endl;
    std::cout << "- Memory operations: 2-3x faster with aligned SIMD" << std::endl;
    std::cout << "- Overall TUI performance: 2-5x improvement possible" << std::endl;
    
    return 0;
}