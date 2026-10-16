#include <gtest/gtest.h>
#include "utils/benchmark.hpp"
#include <thread>
#include <chrono>

namespace {

TEST(BenchmarkTest, ExecutionScope) {
    EXPECT_NO_THROW({
        utils::Benchmark b("Test Scope Timer");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
}

} // namespace
