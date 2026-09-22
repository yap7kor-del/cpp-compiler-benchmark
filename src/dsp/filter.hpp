#pragma once
#include <vector>
#include <cstddef>

namespace dsp {

class FirFilter {
public:
    // Construct a FIR filter with given coefficients (taps)
    explicit FirFilter(const std::vector<float>& coefficients);

    // Process a single input sample
    float tick(float sample);

    // Process an entire signal vector (highly parallelizable/vectorizable!)
    // Demonstrates the impact of -O3 auto-vectorization, loop-unrolling, and SIMD
    std::vector<float> process(const std::vector<float>& signal);

    // Process an entire signal vector using hand-unrolled loops
    // Demonstrates whether hand-unrolling helps or hurts with modern optimizing compilers
    std::vector<float> process_unrolled(const std::vector<float>& signal);

private:
    std::vector<float> coeffs_;
    std::vector<float> history_;
    size_t write_idx_ = 0;
};

} // namespace dsp
