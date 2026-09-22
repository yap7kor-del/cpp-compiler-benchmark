#include "matrix.hpp"
#include <stdexcept>
#include <random>

namespace math {

Matrix::Matrix(size_t rows, size_t cols, float initial_val)
    : rows_(rows), cols_(cols), data_(rows * cols, initial_val) {}

void Matrix::fill_random() {
    // Standard random number engine
    std::mt19937 gen(42); // Seed fixed for determinism
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    for (auto& val : data_) {
        val = dis(gen);
    }
}

Matrix Matrix::multiply(const Matrix& a, const Matrix& b) {
    if (a.cols() != b.rows()) {
        throw std::invalid_argument("Matrix inner dimensions must match for multiplication.");
    }

    size_t r_a = a.rows();
    size_t c_a = a.cols();
    size_t c_b = b.cols();

    Matrix result(r_a, c_b, 0.0f);

    // Naive i-j-k loop: Cache unfriendly for b (column-wise access)
    // -O0 will make this extremely slow.
    // -O3 will attempt loop unrolling and SIMD optimization, but strides will hinder it.
    for (size_t i = 0; i < r_a; ++i) {
        for (size_t j = 0; j < c_b; ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < c_a; ++k) {
                sum += a.get(i, k) * b.get(k, j);
            }
            result.set(i, j, sum);
        }
    }

    return result;
}

Matrix Matrix::multiply_optimized(const Matrix& a, const Matrix& b) {
    if (a.cols() != b.rows()) {
        throw std::invalid_argument("Matrix inner dimensions must match for multiplication.");
    }

    size_t r_a = a.rows();
    size_t c_a = a.cols();
    size_t c_b = b.cols();

    Matrix result(r_a, c_b, 0.0f);

    // Optimized i-k-j loop: Cache friendly!
    // b is accessed row-wise (sequentially in memory).
    // The optimizer can easily auto-vectorize the inner loop (j) using SIMD instructions.
    for (size_t i = 0; i < r_a; ++i) {
        for (size_t k = 0; k < c_a; ++k) {
            float val_a = a.get(i, k);
            for (size_t j = 0; j < c_b; ++j) {
                // Using raw data access style internally for maximum vectorization capability
                float existing = result.get(i, j);
                result.set(i, j, existing + val_a * b.get(k, j));
            }
        }
    }

    return result;
}

} // namespace math
