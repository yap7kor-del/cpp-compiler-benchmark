#include "filter.hpp"
#include <stdexcept>

namespace dsp {

FirFilter::FirFilter(const std::vector<float>& coefficients)
    : coeffs_(coefficients), history_(coefficients.size(), 0.0f), write_idx_(0) {
    if (coeffs_.empty()) {
        throw std::invalid_argument("FIR filter coefficients cannot be empty");
    }
}

float FirFilter::tick(float sample) {
    size_t size = coeffs_.size();
    if (size == 0) {
        return 0.0f;
    }

    history_[write_idx_] = sample;

    float output = 0.0f;

    // Circular buffer dot product
    for (size_t i = 0; i < size; ++i) {
        size_t h_idx = (write_idx_ + size - i) % size;
        output += coeffs_[i] * history_[h_idx];
    }

    write_idx_ = (write_idx_ + 1) % size;
    return output;
}

std::vector<float> FirFilter::process(const std::vector<float>& signal) {
    std::vector<float> output(signal.size(), 0.0f);
    size_t num_coeffs = coeffs_.size();

    // Nested loop structure
    // Perfect target for auto-vectorization when optimized (-O3, -Ofast).
    // In unoptimized mode (-O0), this is very slow due to repeated memory lookups.
    for (size_t n = 0; n < signal.size(); ++n) {
        float sum = 0.0f;
        for (size_t i = 0; i < num_coeffs; ++i) {
            if (n >= i) {
                sum += coeffs_[i] * signal[n - i];
            }
        }
        output[n] = sum;
    }

    return output;
}

std::vector<float> FirFilter::process_unrolled(const std::vector<float>& signal) {
    std::vector<float> output(signal.size(), 0.0f);
    size_t num_coeffs = coeffs_.size();

    // We assume num_coeffs is a multiple of 4 for simplicity in this unrolled example.
    // Demonstrates hand-optimization vs compiler-optimization.
    // Modern compilers often optimize 'process()' better than manual unrolling!
    for (size_t n = 0; n < signal.size(); ++n) {
        float sum = 0.0f;
        size_t i = 0;

        // Hand-unroll by 4
        for (; i + 3 < num_coeffs; i += 4) {
            float sum0 = (n >= i)     ? coeffs_[i]   * signal[n - i]     : 0.0f;
            float sum1 = (n >= i + 1) ? coeffs_[i+1] * signal[n - (i+1)] : 0.0f;
            float sum2 = (n >= i + 2) ? coeffs_[i+2] * signal[n - (i+2)] : 0.0f;
            float sum3 = (n >= i + 3) ? coeffs_[i+3] * signal[n - (i+3)] : 0.0f;
            sum += sum0 + sum1 + sum2 + sum3;
        }

        // Clean up remaining
        for (; i < num_coeffs; ++i) {
            if (n >= i) {
                sum += coeffs_[i] * signal[n - i];
            }
        }

        output[n] = sum;
    }

    return output;
}

} // namespace dsp
