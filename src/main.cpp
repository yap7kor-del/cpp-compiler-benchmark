#include <fmt/core.h>
#include "utils/benchmark.hpp"
#include "math/matrix.hpp"
#include "dsp/filter.hpp"

#include <vector>
#include <string>

int main() {
    fmt::print("====================================================\n");
    fmt::print("--- PERFORMANCE BENCHMARK & OPTIMIZATION DEMO ---\n");
    fmt::print("====================================================\n\n");

    // 1. Detect and print compiler information
    #if defined(__clang__)
        std::string compiler = fmt::format("Clang v{}.{}.{}", __clang_major__, __clang_minor__, __clang_patchlevel__);
    #elif defined(__GNUC__)
        std::string compiler = fmt::format("GCC v{}.{}.{}", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #elif defined(_MSC_VER)
        std::string compiler = fmt::format("MSVC v{}", _MSC_VER);
    #else
        std::string compiler = "Unknown Compiler";
    #endif

    // 2. Detect build configuration / optimization flag active
    std::string opt_status = "Disabled (Debug / -O0)";
    #if defined(__OPTIMIZE__)
        opt_status = "Enabled (-O1, -O2, -O3, -Os, or -Ofast)";
    #endif

    #if defined(__FAST_MATH__)
        opt_status += " with Fast-Math enabled!";
    #endif

    fmt::print("Compiler Info : {}\n", compiler);
    fmt::print("Optimization  : {}\n", opt_status);

    #if defined(__AVX2__)
        fmt::print("SIMD Target   : AVX2 instructions enabled!\n");
    #elif defined(__ARM_NEON)
        fmt::print("SIMD Target   : ARM NEON instructions enabled!\n");
    #else
        fmt::print("SIMD Target   : Generic/Standard Instruction Set\n");
    #endif

    fmt::print("\n----------------------------------------------------\n");
    fmt::print("1. MATRIX MULTIPLICATION BENCHMARK (O(N^3))\n");
    fmt::print("----------------------------------------------------\n");

    constexpr size_t MATRIX_SIZE = 250; // Size selected to make execution time distinct
    fmt::print("Initializing two {}x{} matrices...\n", MATRIX_SIZE, MATRIX_SIZE);

    math::Matrix matA(MATRIX_SIZE, MATRIX_SIZE);
    math::Matrix matB(MATRIX_SIZE, MATRIX_SIZE);
    matA.fill_random();
    matB.fill_random();

    {
        utils::Benchmark b("Naive i-j-k Matrix Mult");
        math::Matrix result1 = math::Matrix::multiply(matA, matB);
        // Prevent dead-code elimination by using a small checksum of the result
        fmt::print("  -> Naive checksum: {:.4f}\n", result1.get(0, 0));
    }

    {
        utils::Benchmark b("Optimized i-k-j Matrix Mult");
        math::Matrix result2 = math::Matrix::multiply_optimized(matA, matB);
        fmt::print("  -> Optimized checksum: {:.4f}\n", result2.get(0, 0));
    }

    fmt::print("\n----------------------------------------------------\n");
    fmt::print("2. DSP FIR FILTER BENCHMARK (Auto-Vectorization)\n");
    fmt::print("----------------------------------------------------\n");

    constexpr size_t SIGNAL_SIZE = 100000;
    constexpr size_t FILTER_TAPS = 128; // Taps must be multiple of 4 for process_unrolled
    fmt::print("Generating signal of size {} and filter with {} taps...\n", SIGNAL_SIZE, FILTER_TAPS);

    std::vector<float> signal(SIGNAL_SIZE, 0.5f);
    // Add some variation to signal
    for (size_t i = 0; i < SIGNAL_SIZE; ++i) {
        if (i % 2 == 0) signal[i] = -0.5f;
    }

    std::vector<float> coeffs(FILTER_TAPS, 1.0f / FILTER_TAPS); // Simple moving average
    dsp::FirFilter filter(coeffs);

    {
        utils::Benchmark b("Standard nested loop FIR");
        std::vector<float> result = filter.process(signal);
        fmt::print("  -> Standard checksum: {:.4f}\n", result[SIGNAL_SIZE - 1]);
    }

    {
        utils::Benchmark b("Hand-unrolled nested loop FIR");
        std::vector<float> result = filter.process_unrolled(signal);
        fmt::print("  -> Unrolled checksum: {:.4f}\n", result[SIGNAL_SIZE - 1]);
    }

    fmt::print("\n====================================================\n");
    fmt::print("Benchmark complete! Observe how compilation options\naffect execution times above.\n");
    fmt::print("====================================================\n");

    return 0;
}
