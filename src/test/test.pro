TEMPLATE = app
QT += core
CONFIG += qtestlib debug
HEADERS += ../domain/undo/logger.h \
    ../domain/undo/command.h \
    ../domain/undo/replay.h
HEADERS += treplay.h \
    oomtestutil.h
SOURCES += ../domain/undo/logger.cpp \
    ../domain/undo/command.cpp \
    ../domain/undo/replay.cpp
SOURCES += \
    tmain.cpp \
    treplay.cpp
FORMS += 
RESOURCES += 
INCLUDEPATH += ../..
oomtestutil.target = oomteststub.o
oomtestutil.commands = make -f oomtestutil.mak
oomtestutil.depends =
QMAKE_EXTRA_TARGETS += oomtestutil
QMAKE_CLEAN += oomteststub.o oomtestutil.so
PRE_TARGETDEPS = oomteststub.o
LIBS += oomteststub.o -ldl
DESTDIR=.
release:OBJECTS_DIR=build/release
release:TARGET = test
debug:OBJECTS_DIR=build/debug
debug:TARGET = test-d
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
