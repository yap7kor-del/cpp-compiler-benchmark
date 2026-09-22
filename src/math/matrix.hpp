#pragma once
#include <vector>
#include <cstddef>

namespace math {

class Matrix {
public:
    Matrix(size_t rows, size_t cols, float initial_val = 0.0f);

    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    float get(size_t r, size_t c) const { return data_[r * cols_ + c]; }
    void set(size_t r, size_t c, float val) { data_[r * cols_ + c] = val; }

    // Naive matrix multiplication (O(N^3))
    // Highly sensitive to compiler optimizer flags (loop order, vectorization)
    static Matrix multiply(const Matrix& a, const Matrix& b);

    // Optimized matrix multiplication (transposed loop for better cache utilization)
    // Helps demonstrate how hardware-aware code design + compiler flags interact
    static Matrix multiply_optimized(const Matrix& a, const Matrix& b);

    // Fill matrix with dummy pseudo-random values for testing
    void fill_random();

private:
    size_t rows_;
    size_t cols_;
    std::vector<float> data_;
};

} // namespace math
