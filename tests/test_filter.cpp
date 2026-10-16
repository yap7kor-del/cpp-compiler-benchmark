#include <gtest/gtest.h>
#include "dsp/filter.hpp"
#include <stdexcept>
#include <vector>
#include <cmath>

namespace {

TEST(FirFilterTest, EmptyCoefficientsThrows) {
    const std::vector<float> empty_coeffs;
    EXPECT_THROW((void)dsp::FirFilter(empty_coeffs), std::invalid_argument);
}

TEST(FirFilterTest, SingleTapIdentity) {
    const std::vector<float> coeffs = {1.0f};
    dsp::FirFilter filter(coeffs);

    const std::vector<float> input = {1.0f, -2.5f, 3.2f, 0.0f, 100.0f};

    // Test process()
    std::vector<float> proc_out = filter.process(input);
    ASSERT_EQ(proc_out.size(), input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_FLOAT_EQ(proc_out[i], input[i]);
    }

    // Test process_unrolled()
    std::vector<float> unrolled_out = filter.process_unrolled(input);
    ASSERT_EQ(unrolled_out.size(), input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_FLOAT_EQ(unrolled_out[i], input[i]);
    }

    // Test tick() sequentially
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_FLOAT_EQ(filter.tick(input[i]), input[i]);
    }
}

TEST(FirFilterTest, SingleTapGain) {
    const std::vector<float> coeffs = {2.5f};
    dsp::FirFilter filter(coeffs);

    const std::vector<float> input = {2.0f, -1.5f, 0.5f};
    const std::vector<float> expected = {5.0f, -3.75f, 1.25f};

    std::vector<float> proc_out = filter.process(input);
    ASSERT_EQ(proc_out.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(proc_out[i], expected[i]);
    }

    std::vector<float> unrolled_out = filter.process_unrolled(input);
    ASSERT_EQ(unrolled_out.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(unrolled_out[i], expected[i]);
    }
}

TEST(FirFilterTest, PureDelayFilter) {
    // 2-sample delay filter: [0, 0, 1]
    const std::vector<float> coeffs = {0.0f, 0.0f, 1.0f};
    dsp::FirFilter filter(coeffs);

    const std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    const std::vector<float> expected = {0.0f, 0.0f, 1.0f, 2.0f, 3.0f};

    std::vector<float> proc_out = filter.process(input);
    ASSERT_EQ(proc_out.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(proc_out[i], expected[i]);
    }

    std::vector<float> unrolled_out = filter.process_unrolled(input);
    ASSERT_EQ(unrolled_out.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(unrolled_out[i], expected[i]);
    }

    // Tick test from clean filter
    dsp::FirFilter tick_filter(coeffs);
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_FLOAT_EQ(tick_filter.tick(input[i]), expected[i]);
    }
}

TEST(FirFilterTest, ImpulseResponseMatchesCoeffs) {
    // Standard property of FIR filters: impulse response equals the filter taps
    const std::vector<float> coeffs = {0.1f, -0.2f, 0.5f, -0.3f, 0.8f};
    dsp::FirFilter filter(coeffs);

    // Delta signal: [1, 0, 0, 0, 0, 0, 0]
    std::vector<float> delta(7, 0.0f);
    delta[0] = 1.0f;

    std::vector<float> proc_out = filter.process(delta);
    ASSERT_EQ(proc_out.size(), delta.size());

    // First 5 elements must match coeffs exactly
    for (size_t i = 0; i < coeffs.size(); ++i) {
        EXPECT_FLOAT_EQ(proc_out[i], coeffs[i]);
    }
    // Remaining elements must be zero
    for (size_t i = coeffs.size(); i < delta.size(); ++i) {
        EXPECT_FLOAT_EQ(proc_out[i], 0.0f);
    }

    // Same for process_unrolled
    std::vector<float> unrolled_out = filter.process_unrolled(delta);
    ASSERT_EQ(unrolled_out.size(), delta.size());
    for (size_t i = 0; i < coeffs.size(); ++i) {
        EXPECT_FLOAT_EQ(unrolled_out[i], coeffs[i]);
    }
    for (size_t i = coeffs.size(); i < delta.size(); ++i) {
        EXPECT_FLOAT_EQ(unrolled_out[i], 0.0f);
    }

    // Same for tick
    dsp::FirFilter tick_filter(coeffs);
    for (size_t i = 0; i < coeffs.size(); ++i) {
        EXPECT_FLOAT_EQ(tick_filter.tick(delta[i]), coeffs[i]);
    }
    for (size_t i = coeffs.size(); i < delta.size(); ++i) {
        EXPECT_FLOAT_EQ(tick_filter.tick(delta[i]), 0.0f);
    }
}

TEST(FirFilterTest, MovingAverageStepResponse) {
    // 4-tap moving average filter
    const std::vector<float> coeffs = {0.25f, 0.25f, 0.25f, 0.25f};
    dsp::FirFilter filter(coeffs);

    // Step function input: all 1.0
    const std::vector<float> step(6, 1.0f);
    const std::vector<float> expected = {0.25f, 0.50f, 0.75f, 1.00f, 1.00f, 1.00f};

    std::vector<float> proc_out = filter.process(step);
    ASSERT_EQ(proc_out.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(proc_out[i], expected[i]);
    }

    std::vector<float> unrolled_out = filter.process_unrolled(step);
    ASSERT_EQ(unrolled_out.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(unrolled_out[i], expected[i]);
    }

    dsp::FirFilter tick_filter(coeffs);
    for (size_t i = 0; i < step.size(); ++i) {
        EXPECT_FLOAT_EQ(tick_filter.tick(step[i]), expected[i]);
    }
}

TEST(FirFilterTest, TickVsProcessEquivalence) {
    const std::vector<float> coeffs = {0.1f, 0.15f, -0.05f, 0.2f, 0.3f, -0.1f, 0.05f, 0.25f};
    dsp::FirFilter filter_proc(coeffs);
    dsp::FirFilter filter_tick(coeffs);

    // Generate test signal
    constexpr size_t signal_len = 64;
    std::vector<float> signal(signal_len);
    for (size_t i = 0; i < signal_len; ++i) {
        signal[i] = std::sin(static_cast<float>(i) * 0.2f);
    }

    std::vector<float> proc_out = filter_proc.process(signal);
    std::vector<float> tick_out(signal_len);
    for (size_t i = 0; i < signal_len; ++i) {
        tick_out[i] = filter_tick.tick(signal[i]);
    }

    ASSERT_EQ(proc_out.size(), signal_len);
    for (size_t i = 0; i < signal_len; ++i) {
        EXPECT_NEAR(proc_out[i], tick_out[i], 1e-5f);
    }
}

TEST(FirFilterTest, ProcessVsProcessUnrolledMultipleOf4) {
    // Test with 4, 8, 12, 16 taps
    const std::vector<size_t> tap_counts = {4, 8, 12, 16};
    constexpr size_t signal_len = 100;
    std::vector<float> signal(signal_len);
    for (size_t i = 0; i < signal_len; ++i) {
        signal[i] = (i % 2 == 0) ? 1.0f : -0.5f;
    }

    for (size_t taps : tap_counts) {
        std::vector<float> coeffs(taps);
        for (size_t k = 0; k < taps; ++k) {
            coeffs[k] = 1.0f / static_cast<float>(k + 1);
        }

        dsp::FirFilter filter(coeffs);
        std::vector<float> standard_out = filter.process(signal);
        std::vector<float> unrolled_out = filter.process_unrolled(signal);

        ASSERT_EQ(standard_out.size(), unrolled_out.size());
        for (size_t i = 0; i < signal_len; ++i) {
            EXPECT_NEAR(standard_out[i], unrolled_out[i], 1e-5f);
        }
    }
}

TEST(FirFilterTest, ProcessVsProcessUnrolledNotMultipleOf4) {
    // Test with 1, 2, 3, 5, 7, 9, 11, 13 taps to verify unrolled cleanup loop
    const std::vector<size_t> tap_counts = {1, 2, 3, 5, 7, 9, 11, 13};
    constexpr size_t signal_len = 100;
    std::vector<float> signal(signal_len);
    for (size_t i = 0; i < signal_len; ++i) {
        signal[i] = std::cos(static_cast<float>(i) * 0.15f);
    }

    for (size_t taps : tap_counts) {
        std::vector<float> coeffs(taps);
        for (size_t k = 0; k < taps; ++k) {
            coeffs[k] = static_cast<float>(k + 1) * 0.1f;
        }

        dsp::FirFilter filter(coeffs);
        std::vector<float> standard_out = filter.process(signal);
        std::vector<float> unrolled_out = filter.process_unrolled(signal);

        ASSERT_EQ(standard_out.size(), unrolled_out.size());
        for (size_t i = 0; i < signal_len; ++i) {
            EXPECT_NEAR(standard_out[i], unrolled_out[i], 1e-5f);
        }
    }
}

TEST(FirFilterTest, EmptySignal) {
    const std::vector<float> coeffs = {0.5f, 0.5f};
    dsp::FirFilter filter(coeffs);

    const std::vector<float> empty_signal;
    std::vector<float> proc_out = filter.process(empty_signal);
    std::vector<float> unrolled_out = filter.process_unrolled(empty_signal);

    EXPECT_TRUE(proc_out.empty());
    EXPECT_TRUE(unrolled_out.empty());
}

TEST(FirFilterTest, SignalShorterThanFilter) {
    const std::vector<float> coeffs = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}; // 6 taps
    dsp::FirFilter filter(coeffs);

    const std::vector<float> signal = {1.0f, 2.0f}; // 2 samples

    // Expected:
    // n=0: coeffs[0] * signal[0] = 1*1 = 1
    // n=1: coeffs[0]*signal[1] + coeffs[1]*signal[0] = 1*2 + 2*1 = 4
    std::vector<float> proc_out = filter.process(signal);
    std::vector<float> unrolled_out = filter.process_unrolled(signal);

    ASSERT_EQ(proc_out.size(), 2u);
    ASSERT_EQ(unrolled_out.size(), 2u);

    EXPECT_FLOAT_EQ(proc_out[0], 1.0f);
    EXPECT_FLOAT_EQ(proc_out[1], 4.0f);
    EXPECT_FLOAT_EQ(unrolled_out[0], 1.0f);
    EXPECT_FLOAT_EQ(unrolled_out[1], 4.0f);
}

} // namespace
