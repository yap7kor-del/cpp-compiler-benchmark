from conan import ConanFile
from conan.tools.cmake import cmake_layout

class ConanDemo(ConanFile):
    name = "conan_demo"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"

    # Required libraries for our intermediate application & CI pipeline testing
    requires = [
        "fmt/11.0.2",       # For beautiful, modern, high-speed terminal formatting
    ]

    # Test requirements (only built/linked during testing phases in CI)
    test_requires = [
        "gtest/1.15.0",     # Google Test framework for validating our DSP and Math logic
    ]

    generators = (
        "CMakeToolchain",   # Generates conan_toolchain.cmake containing compiler/arch flags
        "CMakeDeps"         # Generates find-package files (fmt-config.cmake, GTest-config.cmake)
    )

    def layout(self):
        # Configures modern Conan 2.x subfolders structure for build and source files
        cmake_layout(self)
