This is currently a work in progress.

## Build notes

- Prior to building make sure Qt is on your path or set `CMAKE_PREFIX_PATH` env vars to cmake can find your Qt install. E.g. `CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/`.
- Generate build files with `cmake -B build`
- Build with `cmake --build build/`

### Windows specific

- Copy qt dlls with `windeployqt.exe` (e.g. `/c/Qt/6.0.0/msvc2019_64/bin/windeployqt.exe build/Debug/` replacing the Qt path based on where Qt is, and `Debug` with whatever kind of build you've performed).

#### Building on Windows with clang

Need visual studio env vars set for linker. Can do so by starting a commnad prompt and running something like "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" then doing the config (in gitbash) via

CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/ /c/Program\ Files/CMake/bin/cmake -G Ninja -DCMAKE_MAKE_PROGRAM=/e/Patches/Ninja/ninja -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build

### MacOS specific

- Package into an app using `macdeployqt`(e.g. `macdeployqt build/mp4-manipulator.app`).

## Style

Google style (should be enforced by the included .clang-format), east const (at time of writing not enforced by clang-format).

Use smart pointers (e.g. unique_ptr) for ownership. Raw pointers are not owning, thought bento4 and Qt may use them as such internally.

### Clang-format

clang-format 12 is currently used for formatting.