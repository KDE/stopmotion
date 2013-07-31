TEMPLATE = app
QT += core
CONFIG += qtestlib debug
HEADERS += ../domain/undo/filelogger.h \
    ../domain/undo/command.h \
    ../domain/undo/executor.h
HEADERS += texecutor.h \
    oomtestutil.h
SOURCES += ../domain/undo/filelogger.cpp \
    ../domain/undo/command.cpp \
    ../domain/undo/executor.cpp \
    ../domain/undo/random.cpp
SOURCES += \
    tmain.cpp \
    texecutor.cpp \
    testundo.cpp
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
