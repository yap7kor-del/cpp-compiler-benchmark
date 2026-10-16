#include <gtest/gtest.h>
#include "math/matrix.hpp"
#include <stdexcept>
#include <cmath>

namespace {

TEST(MatrixTest, DefaultInitialization) {
    constexpr size_t rows = 3;
    constexpr size_t cols = 4;
    math::Matrix mat(rows, cols);

    EXPECT_EQ(mat.rows(), rows);
    EXPECT_EQ(mat.cols(), cols);

    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            EXPECT_FLOAT_EQ(mat.get(r, c), 0.0f);
        }
    }
}

TEST(MatrixTest, CustomInitialization) {
    constexpr size_t rows = 4;
    constexpr size_t cols = 2;
    constexpr float init_val = 3.14f;
    math::Matrix mat(rows, cols, init_val);

    EXPECT_EQ(mat.rows(), rows);
    EXPECT_EQ(mat.cols(), cols);

    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            EXPECT_FLOAT_EQ(mat.get(r, c), init_val);
        }
    }
}

TEST(MatrixTest, ZeroDimensions) {
    math::Matrix mat(0, 0);
    EXPECT_EQ(mat.rows(), 0u);
    EXPECT_EQ(mat.cols(), 0u);
}

TEST(MatrixTest, GetSetElements) {
    math::Matrix mat(3, 3, 0.0f);

    mat.set(0, 0, 1.5f);
    mat.set(1, 2, -4.2f);
    mat.set(2, 1, 9.9f);

    EXPECT_FLOAT_EQ(mat.get(0, 0), 1.5f);
    EXPECT_FLOAT_EQ(mat.get(1, 2), -4.2f);
    EXPECT_FLOAT_EQ(mat.get(2, 1), 9.9f);

    // Verify other elements remain 0.0f
    EXPECT_FLOAT_EQ(mat.get(0, 1), 0.0f);
    EXPECT_FLOAT_EQ(mat.get(0, 2), 0.0f);
    EXPECT_FLOAT_EQ(mat.get(1, 0), 0.0f);
    EXPECT_FLOAT_EQ(mat.get(1, 1), 0.0f);
    EXPECT_FLOAT_EQ(mat.get(2, 0), 0.0f);
    EXPECT_FLOAT_EQ(mat.get(2, 2), 0.0f);
}

TEST(MatrixTest, FillRandomBoundedAndDeterministic) {
    constexpr size_t rows = 5;
    constexpr size_t cols = 5;

    math::Matrix mat1(rows, cols);
    math::Matrix mat2(rows, cols);

    mat1.fill_random();
    mat2.fill_random();

    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            float v1 = mat1.get(r, c);
            float v2 = mat2.get(r, c);

            // Bounds check: uniform_real_distribution [0.0, 1.0)
            EXPECT_GE(v1, 0.0f);
            EXPECT_LE(v1, 1.0f);

            // Determinism check (fixed seed 42)
            EXPECT_FLOAT_EQ(v1, v2);
        }
    }
}

TEST(MatrixTest, MultiplyDimensionMismatchThrows) {
    math::Matrix a(2, 3);
    math::Matrix b(2, 3); // a.cols() = 3 != b.rows() = 2

    EXPECT_THROW((void)math::Matrix::multiply(a, b), std::invalid_argument);
    EXPECT_THROW((void)math::Matrix::multiply_optimized(a, b), std::invalid_argument);

    math::Matrix c(4, 5);
    math::Matrix d(6, 4); // c.cols() = 5 != d.rows() = 6

    EXPECT_THROW((void)math::Matrix::multiply(c, d), std::invalid_argument);
    EXPECT_THROW((void)math::Matrix::multiply_optimized(c, d), std::invalid_argument);
}

TEST(MatrixTest, MultiplyIdentity) {
    constexpr size_t size = 3;
    math::Matrix a(size, size);
    a.set(0, 0, 1.0f); a.set(0, 1, 2.0f); a.set(0, 2, 3.0f);
    a.set(1, 0, 4.0f); a.set(1, 1, 5.0f); a.set(1, 2, 6.0f);
    a.set(2, 0, 7.0f); a.set(2, 1, 8.0f); a.set(2, 2, 9.0f);

    math::Matrix identity(size, size, 0.0f);
    for (size_t i = 0; i < size; ++i) {
        identity.set(i, i, 1.0f);
    }

    // A * I = A (naive)
    math::Matrix res_naive_ai = math::Matrix::multiply(a, identity);
    for (size_t r = 0; r < size; ++r) {
        for (size_t c = 0; c < size; ++c) {
            EXPECT_FLOAT_EQ(res_naive_ai.get(r, c), a.get(r, c));
        }
    }

    // I * A = A (naive)
    math::Matrix res_naive_ia = math::Matrix::multiply(identity, a);
    for (size_t r = 0; r < size; ++r) {
        for (size_t c = 0; c < size; ++c) {
            EXPECT_FLOAT_EQ(res_naive_ia.get(r, c), a.get(r, c));
        }
    }

    // A * I = A (optimized)
    math::Matrix res_opt_ai = math::Matrix::multiply_optimized(a, identity);
    for (size_t r = 0; r < size; ++r) {
        for (size_t c = 0; c < size; ++c) {
            EXPECT_FLOAT_EQ(res_opt_ai.get(r, c), a.get(r, c));
        }
    }

    // I * A = A (optimized)
    math::Matrix res_opt_ia = math::Matrix::multiply_optimized(identity, a);
    for (size_t r = 0; r < size; ++r) {
        for (size_t c = 0; c < size; ++c) {
            EXPECT_FLOAT_EQ(res_opt_ia.get(r, c), a.get(r, c));
        }
    }
}

TEST(MatrixTest, MultiplyKnownValues) {
    // A: 2x3
    // [1, 2, 3]
    // [4, 5, 6]
    math::Matrix a(2, 3);
    a.set(0, 0, 1.0f); a.set(0, 1, 2.0f); a.set(0, 2, 3.0f);
    a.set(1, 0, 4.0f); a.set(1, 1, 5.0f); a.set(1, 2, 6.0f);

    // B: 3x2
    // [7, 8]
    // [9, 1]
    // [2, 3]
    math::Matrix b(3, 2);
    b.set(0, 0, 7.0f); b.set(0, 1, 8.0f);
    b.set(1, 0, 9.0f); b.set(1, 1, 1.0f);
    b.set(2, 0, 2.0f); b.set(2, 1, 3.0f);

    // C = A * B (2x2)
    // C(0,0) = 1*7 + 2*9 + 3*2 = 31
    // C(0,1) = 1*8 + 2*1 + 3*3 = 19
    // C(1,0) = 4*7 + 5*9 + 6*2 = 85
    // C(1,1) = 4*8 + 5*1 + 6*3 = 55
    math::Matrix c_naive = math::Matrix::multiply(a, b);
    EXPECT_EQ(c_naive.rows(), 2u);
    EXPECT_EQ(c_naive.cols(), 2u);
    EXPECT_FLOAT_EQ(c_naive.get(0, 0), 31.0f);
    EXPECT_FLOAT_EQ(c_naive.get(0, 1), 19.0f);
    EXPECT_FLOAT_EQ(c_naive.get(1, 0), 85.0f);
    EXPECT_FLOAT_EQ(c_naive.get(1, 1), 55.0f);

    math::Matrix c_opt = math::Matrix::multiply_optimized(a, b);
    EXPECT_EQ(c_opt.rows(), 2u);
    EXPECT_EQ(c_opt.cols(), 2u);
    EXPECT_FLOAT_EQ(c_opt.get(0, 0), 31.0f);
    EXPECT_FLOAT_EQ(c_opt.get(0, 1), 19.0f);
    EXPECT_FLOAT_EQ(c_opt.get(1, 0), 85.0f);
    EXPECT_FLOAT_EQ(c_opt.get(1, 1), 55.0f);
}

TEST(MatrixTest, MultiplyZeroMatrix) {
    math::Matrix a(3, 3);
    a.fill_random();
    math::Matrix zero(3, 3, 0.0f);

    math::Matrix res_naive = math::Matrix::multiply(a, zero);
    math::Matrix res_opt = math::Matrix::multiply_optimized(a, zero);

    for (size_t r = 0; r < 3; ++r) {
        for (size_t c = 0; c < 3; ++c) {
            EXPECT_FLOAT_EQ(res_naive.get(r, c), 0.0f);
            EXPECT_FLOAT_EQ(res_opt.get(r, c), 0.0f);
        }
    }
}

TEST(MatrixTest, Multiply1x1) {
    math::Matrix a(1, 1, 4.0f);
    math::Matrix b(1, 1, 5.0f);

    math::Matrix c_naive = math::Matrix::multiply(a, b);
    math::Matrix c_opt = math::Matrix::multiply_optimized(a, b);

    EXPECT_EQ(c_naive.rows(), 1u);
    EXPECT_EQ(c_naive.cols(), 1u);
    EXPECT_FLOAT_EQ(c_naive.get(0, 0), 20.0f);
    EXPECT_FLOAT_EQ(c_opt.get(0, 0), 20.0f);
}

TEST(MatrixTest, NaiveVsOptimizedEquivalenceSquare) {
    constexpr size_t n = 20;
    math::Matrix a(n, n);
    math::Matrix b(n, n);
    a.fill_random();
    b.fill_random();

    math::Matrix res_naive = math::Matrix::multiply(a, b);
    math::Matrix res_opt = math::Matrix::multiply_optimized(a, b);

    EXPECT_EQ(res_naive.rows(), n);
    EXPECT_EQ(res_naive.cols(), n);
    EXPECT_EQ(res_opt.rows(), n);
    EXPECT_EQ(res_opt.cols(), n);

    for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
            EXPECT_NEAR(res_naive.get(r, c), res_opt.get(r, c), 1e-4f);
        }
    }
}

TEST(MatrixTest, NaiveVsOptimizedEquivalenceRectangular) {
    constexpr size_t r_a = 12;
    constexpr size_t c_a = 25;
    constexpr size_t c_b = 18;

    math::Matrix a(r_a, c_a);
    math::Matrix b(c_a, c_b);
    a.fill_random();
    b.fill_random();

    math::Matrix res_naive = math::Matrix::multiply(a, b);
    math::Matrix res_opt = math::Matrix::multiply_optimized(a, b);

    EXPECT_EQ(res_naive.rows(), r_a);
    EXPECT_EQ(res_naive.cols(), c_b);
    EXPECT_EQ(res_opt.rows(), r_a);
    EXPECT_EQ(res_opt.cols(), c_b);

    for (size_t r = 0; r < r_a; ++r) {
        for (size_t c = 0; c < c_b; ++c) {
            EXPECT_NEAR(res_naive.get(r, c), res_opt.get(r, c), 1e-4f);
        }
    }
}

TEST(MatrixTest, VectorOuterAndInnerProduct) {
    constexpr size_t len = 6;
    math::Matrix row_vec(1, len);
    math::Matrix col_vec(len, 1);

    for (size_t i = 0; i < len; ++i) {
        row_vec.set(0, i, static_cast<float>(i + 1));
        col_vec.set(i, 0, 2.0f);
    }

    // Inner product: (1x6) * (6x1) = 1x1
    // sum = 2 * (1 + 2 + 3 + 4 + 5 + 6) = 2 * 21 = 42
    math::Matrix inner = math::Matrix::multiply(row_vec, col_vec);
    EXPECT_EQ(inner.rows(), 1u);
    EXPECT_EQ(inner.cols(), 1u);
    EXPECT_FLOAT_EQ(inner.get(0, 0), 42.0f);

    math::Matrix inner_opt = math::Matrix::multiply_optimized(row_vec, col_vec);
    EXPECT_FLOAT_EQ(inner_opt.get(0, 0), 42.0f);

    // Outer product: (6x1) * (1x6) = 6x6
    math::Matrix outer = math::Matrix::multiply(col_vec, row_vec);
    math::Matrix outer_opt = math::Matrix::multiply_optimized(col_vec, row_vec);
    EXPECT_EQ(outer.rows(), len);
    EXPECT_EQ(outer.cols(), len);

    for (size_t r = 0; r < len; ++r) {
        for (size_t c = 0; c < len; ++c) {
            float expected = 2.0f * static_cast<float>(c + 1);
            EXPECT_FLOAT_EQ(outer.get(r, c), expected);
            EXPECT_FLOAT_EQ(outer_opt.get(r, c), expected);
        }
    }
}

} // namespace
