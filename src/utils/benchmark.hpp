#pragma once
#include <chrono>
#include <string>
#include <fmt/core.h>

namespace utils {

class Benchmark {
public:
    explicit Benchmark(const std::string& name)
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {}

    ~Benchmark() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        fmt::print("[Benchmark] {:<30} took {:>8} microseconds ({:.3f} ms)\n", 
                   name_, duration, static_cast<double>(duration) / 1000.0);
    }

private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace utils
