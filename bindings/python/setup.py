from glob import glob
import os
import subprocess
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext as _build_ext

WHISPER_ENABLE_COREML = False


def get_library_paths():
    return glob(os.path.join("build", "libwhisper.*"))


class build_ext(_build_ext):
    def run(self):
        # Run CMake to build the libwhisper library
        cmake_dir = os.path.realpath(os.path.join("..", ".."))
        pkg_build_dir = os.path.realpath("build")
        os.makedirs(pkg_build_dir, exist_ok=True)

        extra_cmake_flags = []
        if WHISPER_ENABLE_COREML:
            extra_cmake_flags.append("-DWHISPER_COREML=1")

        subprocess.check_call(["cmake", "-DCMAKE_BUILD_TYPE=Release", *extra_cmake_flags, "-Wno-dev", cmake_dir], cwd=pkg_build_dir)
        subprocess.check_call(["cmake", "--build", ".", "--target", "whisper"], cwd=pkg_build_dir)
        super().run()


ext_modules = [
    Pybind11Extension(
        "pywhisper",
        ["pybind.cpp"],
        include_dirs=[".", "../.."],
        extra_link_args=["-Lbuild", "-lwhisper"],
    ),
]

setup(
    name="pywhisper",
    version="0.0.1",
    description="Python bindings for whisper.cpp",
    author="Jan Huenermann",
    url="https://github.com/ggerganov/whisper.cpp",
    license="MIT",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules,
    data_files=[("lib", get_library_paths())]
)
