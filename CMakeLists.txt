cmake_minimum_required(VERSION 3.21)
project(XPlay2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.3 REQUIRED COMPONENTS Core Gui Widgets Multimedia OpenGL OpenGLWidgets)

set(_ffmpeg_default_root "$ENV{XPLAY2_FFMPEG_ROOT}")
if(NOT _ffmpeg_default_root)
    set(_ffmpeg_default_root "${CMAKE_CURRENT_SOURCE_DIR}/../../QtProject")
endif()
set(FFMPEG_ROOT "${_ffmpeg_default_root}" CACHE PATH "FFmpeg root containing include and lib/win64")
if(NOT FFMPEG_ROOT)
    set(FFMPEG_ROOT "${_ffmpeg_default_root}" CACHE PATH "FFmpeg root containing include and lib/win64" FORCE)
endif()
set(FFMPEG_INCLUDE_DIR "${FFMPEG_ROOT}/include")
set(FFMPEG_LIB_DIR "${FFMPEG_ROOT}/lib/win64")
set(FFMPEG_BIN_DIR "${FFMPEG_ROOT}/bin/win64")
if(NOT EXISTS "${FFMPEG_INCLUDE_DIR}/libavcodec/avcodec.h")
    message(FATAL_ERROR "FFmpeg headers not found under ${FFMPEG_INCLUDE_DIR}; set XPLAY2_FFMPEG_ROOT")
endif()
if(WIN32 AND NOT EXISTS "${FFMPEG_BIN_DIR}/avcodec-57.dll")
    message(FATAL_ERROR "FFmpeg runtime DLLs not found under ${FFMPEG_BIN_DIR}; set XPLAY2_FFMPEG_ROOT")
endif()

set(XPLAY2_SOURCES
    IVideoCall.h XAudioPlay.cpp XAudioPlay.h XAudioThread.cpp XAudioThread.h
    XDecode.cpp XDecode.h XDecodeThread.cpp XDecodeThread.h XDemux.cpp XDemux.h
    XDemuxThread.cpp XDemuxThread.h XResample.cpp XResample.h
    XVideoThread.cpp XVideoThread.h XVideoWidget.cpp XVideoWidget.h
    XSlider.cpp XSlider.h xplay2.cpp xplay2.h main.cpp xplay2.qrc)

add_executable(XPlay2 ${XPLAY2_SOURCES})
if(MSVC)
    target_compile_options(XPlay2 PRIVATE /utf-8)
elseif(MINGW)
    target_compile_options(XPlay2 PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
target_include_directories(XPlay2 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_include_directories(XPlay2 SYSTEM PRIVATE "${FFMPEG_INCLUDE_DIR}")
target_link_directories(XPlay2 PRIVATE "${FFMPEG_LIB_DIR}")
target_link_libraries(XPlay2 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia
    Qt6::OpenGL Qt6::OpenGLWidgets avformat avcodec avutil swresample swscale)
set_target_properties(XPlay2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(WIN32)
    file(GLOB FFMPEG_RUNTIME_DLLS CONFIGURE_DEPENDS "${FFMPEG_BIN_DIR}/*.dll")
    add_custom_command(TARGET XPlay2 POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            ${FFMPEG_RUNTIME_DLLS} "$<TARGET_FILE_DIR:XPlay2>"
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying FFmpeg runtime DLLs")
endif()
