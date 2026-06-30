from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake

PACKAGE_NAME = "sorted_array_tower"
EXPORT_LIBS = ["or_else", "add_one"]

class SortedArrayTower(ConanFile):
    name = PACKAGE_NAME

    # Metadata
    description = "Template You're Gonna Hate But Need"
    author = "Your Name"
    license = "MIT"

    # If the package is available from ConanCenter:
    url = "https://github.com/conan-io/conan-center-index"
    
    # Settings
    settings = "os", "compiler", "build_type", "arch"
    
    # Options
    options = {
        "use_modules": [True, False],
    }
    default_options = {
        "use_modules": True,
    }

    def requirements(self):
        # # Put dependencies here. For example:
        # self.requires("zlib/[>=1.3.1 <2.0.0]")
        pass

    def build_requirements(self):
        skip_tests = self.conf.get("tools.build:skip_test", default=False, check_type=bool)
        if not skip_tests:
            self.test_requires("doctest/[>=2.5.2 <3.0.0]")
            self.test_requires("nanobench/[>=4.3.11 <5.0.0]")

    def validate(self):
        check_min_cppstd(self, "20")

    def layout(self):
        cmake_layout(self, build_folder="build")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["SORTED_ARRAY_TOWER_USE_MODULES"] = self.options.use_modules
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        cmake = CMake(self)
        cmake.ctest()

    def package(self):
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self):
        pkg = self.cpp_info
        pkg.set_property("cmake_file_name", PACKAGE_NAME)

        for lib_name in EXPORT_LIBS:
            component = pkg.components[lib_name]
            component.libs = [lib_name]
            component.set_property(
                "cmake_target_name",
                f"{PACKAGE_NAME}::{lib_name}"
            )

            if self.options.use_modules:
                component.srcdirs = ["modules"]

