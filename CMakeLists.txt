# Min versions for Qt6
cmake_minimum_required(VERSION 3.16.0)

project(mp4-manipulator VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Start Qt config
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
# End Qt6 config

# Start bento4 config
# We do our own config here rather than using a subdir so that we can avoid
# building all the bento4 apps when we don't need them + it allows us more
# flexibility in our build (bento4 statically links the msvc runtime, and this
# way we can configure that as we please rather than having to use the same
# linkage).
set(BENTO4_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Bento4/Source/C++)
set(BENTO4_SOURCE_CODECS ${BENTO4_SOURCE_ROOT}/Codecs)
set(BENTO4_SOURCE_CORE ${BENTO4_SOURCE_ROOT}/Core)
set(BENTO4_SOURCE_CRYPTO ${BENTO4_SOURCE_ROOT}/Crypto)
set(BENTO4_SOURCE_METADATA ${BENTO4_SOURCE_ROOT}/MetaData)
set(BENTO4_SOURCE_SYSTEM ${BENTO4_SOURCE_ROOT}/System)

# AP4 Library
file(GLOB AP4_SOURCES
  ${BENTO4_SOURCE_CODECS}/*.cpp
  ${BENTO4_SOURCE_CORE}/*.cpp
  ${BENTO4_SOURCE_CRYPTO}/*.cpp
  ${BENTO4_SOURCE_METADATA}/*.cpp
  ${BENTO4_SOURCE_SYSTEM}/StdC/*.cpp
)

# Platform specifics
if(WIN32)
  set(AP4_SOURCES ${AP4_SOURCES} ${BENTO4_SOURCE_SYSTEM}/Win32/Ap4Win32Random.cpp)
else()
  set(AP4_SOURCES ${AP4_SOURCES} ${BENTO4_SOURCE_SYSTEM}/Posix/Ap4PosixRandom.cpp)
endif()

# Includes
set(AP4_INCLUDE_DIRS
  ${BENTO4_SOURCE_CORE}
  ${BENTO4_SOURCE_CODECS}
  ${BENTO4_SOURCE_CRYPTO}
  ${BENTO4_SOURCE_METADATA}
)

add_library(ap4 STATIC ${AP4_SOURCES})
target_include_directories(ap4 PUBLIC ${AP4_INCLUDE_DIRS})
# End bento4 config

# We include headers here for a couple of reasons
# - Automoc won't work properly if we don't (since the headers are in their own 
#   dir)
# - It means the results of cmakes gen code will include the headers. E.g.
#   Visual Studio projects will include the headers.
set(MP4_MANIPULATOR_SOURCES
  include/gui/atom_tree_model.h
  include/gui/main_window.h
  include/parsing/atom_inspector.h
  include/parsing/atom.h
  include/parsing/file_utils.h
  include/parsing/position_aware_atom_factory.h
  source/gui/atom_tree_model.cpp
  source/gui/main_window.cpp
  source/parsing/atom_inspector.cpp
  source/parsing/atom.cpp
  source/parsing/file_utils.cpp
  source/parsing/position_aware_atom_factory.cpp
  source/main.cpp)
if(WIN32)
  add_executable(mp4-manipulator WIN32 ${MP4_MANIPULATOR_SOURCES})
elseif(APPLE)
  add_executable(mp4-manipulator MACOSX_BUNDLE ${MP4_MANIPULATOR_SOURCES})
else()
  add_executable(mp4-manipulator ${MP4_MANIPULATOR_SOURCES})
endif()

target_include_directories(mp4-manipulator PRIVATE include)

target_link_libraries(mp4-manipulator PRIVATE ap4)

target_link_libraries(mp4-manipulator PRIVATE Qt6::Widgets)

# Create imported target for windeployqt
