# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(lib_src_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp/${ANDROID_ABI})

# 打印日志
message("当前CMake的路径是：${CMAKE_SOURCE_DIR}")
message("当前 CMAKE_ANDROID_ARCH_ABI 的路径是：${CMAKE_ANDROID_ARCH_ABI}")

include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)

file(GLOB native_srcs ${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp
        ${CMAKE_SOURCE_DIR}/src/main/cpp/*.hpp)

# Declares and names the project.

project("myffmpegtest")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        ffmpeg_lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${native_srcs})

add_library(avcodec SHARED IMPORTED)
add_library(avformat SHARED IMPORTED)
add_library(avutil SHARED IMPORTED)
add_library(swresample SHARED IMPORTED)
add_library(swscale SHARED IMPORTED)
add_library(avfilter SHARED IMPORTED)

set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavcodec.so)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavformat.so)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavutil.so)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswresample.so)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswscale.so)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavfilter.so)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library( # Sets the name of the path variable.
        graphicslib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        jnigraphics)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ffmpeg_lib

        # 先把有依赖的库，先依赖进来


        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        avcodec
        avformat
        avutil
        swresample
        swscale
        avfilter)
















