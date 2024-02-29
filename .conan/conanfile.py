from conan import ConanFile
import os

class OpenCvCoursesRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("zlib/1.2.11")
        self.requires("gtest/1.14.0")
        self.requires("benchmark/1.8.3")
        self.requires("opencv/4.8.1")
        self.requires("libpng/1.6.40", force=True)

    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")
        
    def layout(self):
        # We make the assumption that if the compiler is msvc the
        # CMake generator is multi-config
        multi = True if self.settings.get_safe("compiler") == "msvc" else False
        if multi:
            self.folders.generators = os.path.join("build", "generators")
            self.folders.build = "build"
        else:
            self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
            self.folders.build = os.path.join("build", str(self.settings.build_type))

        