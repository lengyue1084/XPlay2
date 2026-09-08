# XPlay2 qmake project
#
# The source uses Qt 5's multimedia and OpenGL APIs.  Qt Creator should use
# the locally installed Qt 5.15.2 desktop kit (MinGW 32-bit for the bundled
# FFmpeg binaries, or the matching 64-bit kit when selected).
QT += core gui widgets multimedia opengl openglextensions

TEMPLATE = app
TARGET = XPlay2
CONFIG += debug console c++14

# Keep generated files and binaries out of the source tree's parent folders.
DESTDIR = $$PWD/bin
DEPENDPATH += $$PWD
MOC_DIR = $$OUT_PWD/moc
OBJECTS_DIR = $$OUT_PWD/obj
UI_DIR = $$OUT_PWD/ui
RCC_DIR = $$OUT_PWD/rcc

# FFmpeg is installed alongside the other local Qt projects.  Set
# XPLAY2_FFMPEG_ROOT to override this location on another machine.
FFMPEG_ROOT = $$(XPLAY2_FFMPEG_ROOT)
isEmpty(FFMPEG_ROOT): FFMPEG_ROOT = $$PWD/../../QtProject

contains(QT_ARCH, x86_64) {
    FFMPEG_ARCH = win64
} else {
    FFMPEG_ARCH = win32
}

FFMPEG_INCLUDE_DIR = $$FFMPEG_ROOT/include
FFMPEG_LIB_DIR = $$FFMPEG_ROOT/lib/$$FFMPEG_ARCH

!exists($$FFMPEG_INCLUDE_DIR/libavcodec/avcodec.h) {
    error(FFmpeg headers not found under $$FFMPEG_INCLUDE_DIR. Set XPLAY2_FFMPEG_ROOT to the FFmpeg installation.)
}
!exists($$FFMPEG_LIB_DIR/libavcodec.dll.a) {
    error(FFmpeg MinGW libraries not found under $$FFMPEG_LIB_DIR. Set XPLAY2_FFMPEG_ROOT to the FFmpeg installation.)
}

INCLUDEPATH += $$FFMPEG_INCLUDE_DIR
LIBS += -L$$FFMPEG_LIB_DIR \
    -lavformat \
    -lavcodec \
    -lavutil \
    -lswresample \
    -lswscale

include(XPlay2.pri)
