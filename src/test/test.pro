TEMPLATE = app
TARGET = test
QT += core
CONFIG += qtestlib
HEADERS += treplay.h
SOURCES += tmain.cpp \
    treplay.cpp
SOURCES += ../domain/undo/command.cpp \
    ../domain/undo/replay.cpp
FORMS += 
RESOURCES += 
