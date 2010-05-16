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
    Emulator.cpp \
    qscreen.cpp \
    qkeyboardview.cpp \
    qdebugview.cpp \
    qdisasmview.cpp \
    qmemoryview.cpp \
    qconsoleview.cpp
HEADERS += mainwindow.h \
    util/WavPcmFile.h \
    stdafx.h \
    Common.h \
    emubase/Processor.h \
    emubase/Emubase.h \
    emubase/Disasm.h \
    emubase/Defines.h \
    emubase/Board.h \
    main.h \
    Emulator.h \
    qscreen.h \
    qkeyboardview.h \
    qdebugview.h \
    qdisasmview.h \
    qmemoryview.h \
    qconsoleview.h
FORMS += mainwindow.ui
RESOURCES += QtBkBtl.qrc
