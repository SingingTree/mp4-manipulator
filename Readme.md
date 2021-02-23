## Build notes

### Windows specific

- Generate build files with `CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/ /c/Program\ Files/CMake/bin/cmake -B build`
- Build with `CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/ /c/Program\ Files/CMake/bin/cmake --build build/`
- If on Windows copy qt dlls with `/c/Qt/6.0.0/msvc2019_64/bin/windeployqt.exe build/Debug/helloworld.exe` or similar

#### Building on Windows with clang

Need visual studio env vars set for linker. Can do so by starting a commnad prompt and running something like "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" then doing the config (in gitbash) via

CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/ /c/Program\ Files/CMake/bin/cmake -G Ninja -DCMAKE_MAKE_PROGRAM=/e/Patches/Ninja/ninja -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build

## Style

Google sytle (should be enforced by the included .clang-format), east const (at time of writing not enforced by clang-format).

Use smart pointers (e.g. unique_ptr) for ownership. Raw pointers are not owning, thought bento4 and Qt may use them as such internally. 