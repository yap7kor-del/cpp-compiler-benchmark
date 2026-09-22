# Performance Benchmark, Optimization & CI/CD Sandbox

This project is an intermediate-level C++ demonstration designed to illustrate the immense impact of compiler optimizer flags, memory access patterns, cache locality, and auto-vectorization on software performance. It also acts as a testbed for integrating static analysis tools and configuring modern CI/CD pipelines across different compilers, optimization profiles, and target hardware architectures.

---

## 📂 Project Structure

The codebase is organized into modular subfolders under the `src` directory to simulate a real-world production application structure:

```text
compare_gcc_llvm/
├── CMakeLists.txt              # Primary CMake configuration (manages targets, compilation options & Conan links)
├── conanfile.py                # Conan 2.x dependency manager (brings in fmt/11.0.2)
├── README.md                   # This comprehensive guide
├── .github/workflows/ci.yml    # Production-ready CI Matrix build & Static Analysis configuration
└── src/
    ├── main.cpp                # App entry point, runtime compiler detection, and benchmark coordinator
    ├── dsp/
    │   ├── filter.hpp          # Digital Signal Processing (FIR) interface
    │   └── filter.cpp          # Circular buffers & auto-vectorizable MAC (Multiply-Accumulate) loops
    ├── math/
    │   ├── matrix.hpp          # Custom Matrix data structure
    │   └── matrix.cpp          # Cache-unfriendly (naive) vs cache-friendly (transpose) multiply algorithms
    └── utils/
        └── benchmark.hpp       # RAII-based high-resolution microsecond timer
```

---

## 📊 Local Benchmark Results (Empirical Verification)

These benchmarks were run locally on a **GCC v16.2.0** compiler (x86_64 target with C++20 standard). It compares **Debug (-O0 / Unoptimized)** and **Release (-O3 / Fully Optimized)** builds to show how extreme the variance is under real workloads.

| Benchmark Task | Debug Mode (`-O0` / Unoptimized) | Release Mode (`-O3` / Optimized) | Speedup Factor | Description / Dynamic |
| :--- | :---: | :---: | :---: | :--- |
| **Naive $i$-$j$-$k$ Matrix Mult** | $150.65\text{ ms}$ | $13.92\text{ ms}$ | **~10.8x faster** | Classic row-column dot-product. Suffers from column striding cache-misses on matrix B. |
| **Optimized $i$-$k$-$j$ Matrix Mult** | $276.82\text{ ms}$ | $2.33\text{ ms}$ | **~119.0x faster** | Loops are transposed to sequentialize memory access. SIMD registers are easily packed. |
| **Standard Loop FIR Filter** | $102.01\text{ ms}$ | $9.84\text{ ms}$ | **~10.4x faster** | 100k element array filtered with a 128-tap moving average. |
| **Hand-unrolled FIR Filter** | $97.59\text{ ms}$ | $5.03\text{ ms}$ | **~19.4x faster** | Inner loop manually unrolled by 4. Shows how compiler unrolling and SIMD combine. |

### Why did previous small-scale tests show "no change"?
If you compile simple programs (like printing strings, or adding two integers) and toggle optimization flags, the speed difference is unnoticeable. To see the physical benefits of compiler flags:
1. **Sufficient Complexity:** You need CPU-bound operations containing nested loops (like our $250 \times 250$ matrix multiplications doing $O(N^3) = 15.6$ million operations).
2. **Elimination of I/O Overheads:** Console printing (`std::cout`, `printf`) relies on slow operating system calls. Running I/O inside a measured section will always mask execution optimizations.
3. **Prevent Dead-Code Elimination (DCE):** If you execute a massive loop but do nothing with its final outputs, a Release compiler (`-O2` or `-O3`) will delete your code entirely because it has no side-effects. In our program, we calculate and print small checksums (`result.get(0,0)`) to force the compiler to keep and optimize the loops.

---

## 🧠 Understanding Compiler Optimization Flags

### A. Core Optimizer Flags (GCC/Clang)
*   **`-O0` (No Optimization):** Designed for fast compile times and maximum debugging fidelity. Translates C++ statements line-for-line into machine assembly. Variables live on the stack rather than inside CPU registers. Crucially, helper methods (like our matrix `.get()` / `.set()`) are not inlined, which is why optimized algorithms can paradoxically execute slower in `-O0` due to call-stack overhead.
*   **`-O1` (Basic Optimization):** Minimizes execution time and size without adding substantial build times. Enables basic register allocation and basic dead-code pruning.
*   **`-O2` (Standard Release Default):** GCC/Clang recommended release target. Toggles almost all optimizations that do not involve space-speed tradeoffs. Enables **strict function inlining**, instruction pipelining, and loop-invariant code motion.
*   **`-O3` (Aggressive Speed Optimization):** Focuses entirely on performance. Toggles aggressive **Auto-Vectorization** (converting sequential loops into parallel SIMD operations), inline cloning, and loop unrolling.
*   **`-Os` (Size Optimization):** Toggles all `-O2` settings that do not increase the binary file size. Ideal for embedded bare-metal firmware with strict ROM/Flash limitations.
*   **`-Ofast` (Aggressive & IEEE-relaxing):** Toggles `-O3` and disables strict math safety checks (`-ffast-math`). Disregards IEEE 754 float specifications (e.g., assumes no NaN or Inf values and allows algebraic rearrangements). Yields extreme floating-point execution speeds but risks numerical drift.

### B. Architectural/SIMD Flags
A compiler targets generic instruction sets by default. To unlock native vector units (e.g. AVX2, AVX-512, NEON):
*   **`-march=native`:** Custom-compiles the binary specifically utilizing *every* CPU instruction set present on your host machine.
*   **Cross-Compilation Target Flags (e.g. ARM Cortex-M4):**
    For CI/CD embedded cross-compilers (`arm-none-eabi-gcc`), flags like `-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16` direct the compiler to map floating-point operations straight into ARM's hardware Single-Precision Float Processing Unit.

---

## 🔍 Static Analysis Tools & Quality Assurance

Static analysis tools check the code abstract syntax tree (AST) for logical errors, undefined behavior, or memory management concerns without executing the binary.

1.  **Cppcheck:** A dedicated, ultra-fast C/C++ static analysis engine designed to detect real bugs (e.g. buffer overflows, out-of-bounds pointers, uninitialized variables, memory leaks) rather than style violations.
    *   *Usage:* `cppcheck --enable=all --inconclusive --std=c++20 --suppress=missingIncludeSystem src/`
2.  **Clang-Tidy:** Part of the Clang LLVM ecosystem. Verifies adherence to modern coding standards (such as C++ Core Guidelines) and modernizes code constructs automatically.
    *   *Usage:* `clang-tidy src/math/*.cpp src/dsp/*.cpp src/main.cpp -- -std=c++20 -Isrc`
3.  **Compiler Warning Escalation:** Excellent practice mandates treating compiler warnings as build failures.
    *   *GCC/Clang Flags:* `-Wall -Wextra -Wpedantic -Wconversion -Werror`

---

## 🚀 The CI/CD Pipeline (GitHub Actions Configuration)

The `.github/workflows/ci.yml` pipeline defines an industry-standard validation workflow:

1.  **Quality Gate (Static Analysis):**
    Installs and triggers both `cppcheck` and `clang-tidy` to check for security vulnerabilities and standard adherence. If static analysis fails, the pipeline halts immediately.
2.  **Build/Optimization Matrix:**
    Builds the binary across 5 different environment configurations in parallel:
    *   **Ubuntu + GCC + `-O0` (Debug):** Validates syntax correctness and logical debugging paths.
    *   **Ubuntu + GCC + `-O3` (Release):** Validates compilation under aggressive speed optimizations and runs the performance suite in the CI terminal.
    *   **Ubuntu + Clang + `-O2`:** Ensures compiler compatibility and cross-compiler alignment.
    *   **Ubuntu + Clang + `-Ofast`:** Builds with IEEE math relaxation checks.
    *   **Embedded Cross-Compilation (ARM Cortex-M4):** Verifies compile compliance for target microcontrollers using `-Os` size constraints.
