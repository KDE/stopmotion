TEMPLATE = app
TARGET = $(OBJECTS_DIR)test
QT += core
CONFIG += qtestlib
HEADERS += ../domain/undo/logger.h \
    ../domain/undo/command.h \
    ../domain/undo/replay.h
HEADERS += treplay.h \
    oomtestutil.h
SOURCES += ../domain/undo/logger.cpp \
    ../domain/undo/command.cpp \
    ../domain/undo/replay.cpp
SOURCES += oomteststub.c \
    tmain.cpp \
    treplay.cpp
FORMS += 
RESOURCES += 
INCLUDEPATH += ../..
LIBS += -ldl
