cmake_minimum_required(VERSION 3.21)
project(XPlay2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.3 REQUIRED COMPONENTS Core Gui Widgets Multimedia OpenGL OpenGLWidgets)

set(FFMPEG_ROOT "$ENV{XPLAY2_FFMPEG_ROOT}" CACHE PATH "FFmpeg root containing include and lib/win64")
if(NOT FFMPEG_ROOT)
    set(FFMPEG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../QtProject")
endif()
set(FFMPEG_INCLUDE_DIR "${FFMPEG_ROOT}/include")
set(FFMPEG_LIB_DIR "${FFMPEG_ROOT}/lib/win64")
if(NOT EXISTS "${FFMPEG_INCLUDE_DIR}/libavcodec/avcodec.h")
    message(FATAL_ERROR "FFmpeg headers not found under ${FFMPEG_INCLUDE_DIR}; set XPLAY2_FFMPEG_ROOT")
endif()

set(XPLAY2_SOURCES
    IVideoCall.h XAudioPlay.cpp XAudioPlay.h XAudioThread.cpp XAudioThread.h
    XDecode.cpp XDecode.h XDecodeThread.cpp XDecodeThread.h XDemux.cpp XDemux.h
    XDemuxThread.cpp XDemuxThread.h XResample.cpp XResample.h
    XVideoThread.cpp XVideoThread.h XVideoWidget.cpp XVideoWidget.h
    XSlider.cpp XSlider.h xplay2.cpp xplay2.h main.cpp xplay2.ui xplay2.qrc)

add_executable(XPlay2 ${XPLAY2_SOURCES})
if(MSVC)
    target_compile_options(XPlay2 PRIVATE /utf-8)
endif()
target_include_directories(XPlay2 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${FFMPEG_INCLUDE_DIR}"
)
target_link_directories(XPlay2 PRIVATE "${FFMPEG_LIB_DIR}")
target_link_libraries(XPlay2 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia
    Qt6::OpenGL Qt6::OpenGLWidgets avformat avcodec avutil swresample swscale)
set_target_properties(XPlay2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
