mp4-manipulator is a tool for inspecting and manipulating mp4 and other ISO base media file format (BMFF) files.

mp4-manipulator uses the [Bento4](https://github.com/axiomatic-systems/Bento4) library to parse BMFF files, and uses [Qt](https://www.qt.io/) to provide a GUI.

# Usage

Files can be opened via the `File` menu, or by dragging and dropping them on the interface. Multiple files can be opened, each will be given a separate tab.

Opened files can be inspected via the tree interface.

## File manipulation

*Note, this area of functionality is a work in progress. Further functionality is planned.*

When selecting an atom via the right click context menu, the following options are exposed:

- Atoms can be dumped to a file.
- Atoms can be removed from an file. This will reprocess the file using Bento4, and the result will be shown in the tab.

To save a file following mutation, use the save option in the `File` menu.

# Build notes

- Prior to building make sure Qt is on your path or set `CMAKE_PREFIX_PATH` env vars to cmake can find your Qt install. E.g. `CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/`.
- Generate build files with `cmake -B build`
- Build with `cmake --build build/`

# Windows specific build

- Copy qt dlls with `windeployqt.exe` (e.g. `/c/Qt/6.0.0/msvc2019_64/bin/windeployqt.exe build/Debug/` replacing the Qt path based on where Qt is, and `Debug` with whatever kind of build you've performed).

## Building on Windows with clang

Need visual studio env vars set for linker. Can do so by starting a commnad prompt and running something like "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" then doing the config (in gitbash) via

CMAKE_PREFIX_PATH=/c/Qt/6.0.0/msvc2019_64/ /c/Program\ Files/CMake/bin/cmake -G Ninja -DCMAKE_MAKE_PROGRAM=/e/Patches/Ninja/ninja -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build

# MacOS specific build

- Package into an app using `macdeployqt`(e.g. `macdeployqt build/mp4-manipulator.app`).

# Development

## Style

Google style + east const (at time of writing not enforced by clang-format (https://bugs.llvm.org/show_bug.cgi?id=34220), so is on devs to manually do so).

Use smart pointers (e.g. unique_ptr) for ownership. Raw pointers are not owning, though bento4 and Qt complicated this with their own internal styles, so be mindful of that.

### Clang-format

clang-format 12 is currently used for formatting in automation. The in tree .clang-format does what it can, with manual exceptions to style (hopefully) noted in this readme.
