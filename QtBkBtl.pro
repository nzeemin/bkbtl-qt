# -------------------------------------------------
# Project created by QtCreator 2010-05-11T20:45:54
# -------------------------------------------------
TARGET = QtBkBtl
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    util/WavPcmFile.cpp \
    Common.cpp \
    emubase/Processor.cpp \
    emubase/Floppy.cpp \
    emubase/Disasm.cpp \
    emubase/Board.cpp \
    emubase/SoundAY.cpp \
    Emulator.cpp \
    qscreen.cpp \
    qkeyboardview.cpp \
    qdebugview.cpp \
    qdisasmview.cpp \
    qconsoleview.cpp \
    qmemoryview.cpp \
    qteletypeview.cpp \
    qtapeview.cpp \
    qsoundout.cpp \
    qscripting.cpp \
    qdialogs.cpp \    
    settings.cpp
HEADERS += mainwindow.h \
    stdafx.h \
    util/WavPcmFile.h \
    Common.h \
    emubase/Processor.h \
    emubase/Emubase.h \
    emubase/Defines.h \
    emubase/Board.h \
    Emulator.h \
    main.h \
    qscreen.h \
    qkeyboardview.h \
    qdebugview.h \
    qdisasmview.h \
    qconsoleview.h \
    qmemoryview.h \
    qteletypeview.h \
    qtapeview.h \
    qsoundout.h \
    qscripting.h \
    qdialogs.h
FORMS += mainwindow.ui
RESOURCES += QtBkBtl.qrc
QT += widgets
QT += script
QT += multimedia
DEFINES -= UNICODE _UNICODE
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
