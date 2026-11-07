
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.18.1)

project("langsu")

find_package(cxx REQUIRED CONFIG)
link_libraries(cxx::cxx)

add_library(langsu
        SHARED
        jni.cc
        langsu.cc
        )

# The uapi headers live once, at <repo>/uapi. Upstream linked them into this
# directory with a symlink, which git cannot materialise on Windows without
# Developer Mode (it lands as a plain file holding the target path, and the
# build then fails). Adding the repo root as an include directory instead keeps
# a single source of truth without relying on symlink support.
get_filename_component(LANGSU_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
target_include_directories(langsu PRIVATE . "${LANGSU_REPO_ROOT}")

find_library(log-lib log)

target_link_libraries(langsu ${log-lib})

add_library(adbroot SHARED adbroot.cc)
target_link_libraries(adbroot ${log-lib})
