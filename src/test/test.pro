TEMPLATE = app
QT += core
CONFIG += qtestlib debug
HEADERS += ../domain/undo/filelogger.h \
    ../domain/undo/command.h \
    ../domain/undo/commandlogger.h \
    ../domain/undo/executor.h \
    ../domain/undo/addallcommands.h \
    ../presentation/loadcache.h \
    ../domain/undo/commandadd.h \
    ../domain/undo/commandmove.h \
    ../domain/undo/commandremove.h \
    ../domain/undo/commandsetimage.h \
    ../domain/undo/commandaddsound.h \
    ../domain/undo/commandrenamesound.h \
    ../domain/undo/commandremovesound.h \
    ../domain/undo/commandaddscene.h \
    ../domain/undo/commandmovescene.h \
    ../domain/undo/commandremovescene.h \
    ../domain/undo/addallcommands.h\
    ../domain/animation/scene.h \
    ../domain/animation/frame.h \
    ../domain/animation/sound.h \
    ../domain/animation/animation.h \
    ../domain/animation/animationimpl.h \
    ../domain/animation/scenevector.h \
    ../domain/animation/workspacefile.h \
    ../technical/audio/oggvorbis.h \
    ../technical/util.h \
    ../technical/stringiterator.h \
    ../technical/video/videofactory.h \
    ../technical/video/videoencoder.h \
    ../technical/audio/ossdriver.h \
    ../technical/audio/audiodriver.h \
    ../technical/projectserializer.h \
    ../domain/observernotifier.h \
    ../application/externalcommandwithtemporarydirectory.h \
    ../application/externalcommand.h \
    ../foundation/logger.h
HEADERS += texecutor.h \
	tcache.h \
    oomtestutil.h \
    testundo.h \
    testhome.h \
    hash.h \
    tstopmotionundo.h
SOURCES += ../domain/undo/filelogger.cpp \
    ../domain/undo/command.cpp \
    ../domain/undo/commandlogger.cpp \
    ../domain/undo/executor.cpp \
    ../domain/undo/addallcommands.cpp \
    ../domain/undo/random.cpp \
    ../domain/undo/commandadd.cpp \
    ../domain/undo/commandmove.cpp \
    ../domain/undo/commandremove.cpp \
    ../domain/undo/commandsetimage.cpp \
    ../domain/undo/commandaddsound.cpp \
    ../domain/undo/commandrenamesound.cpp \
    ../domain/undo/commandremovesound.cpp \
    ../domain/undo/commandaddscene.cpp \
    ../domain/undo/commandmovescene.cpp \
    ../domain/undo/commandremovescene.cpp \
    ../domain/animation/scene.cpp \
    ../domain/animation/frame.cpp \
    ../domain/animation/sound.cpp \
    ../domain/animation/animation.cpp \
    ../domain/animation/animationimpl.cpp \
    ../domain/animation/scenevector.cpp \
    ../domain/animation/workspacefile.cpp \
    ../technical/audio/oggvorbis.cpp \
    ../technical/audio/audioformat.cpp \
    ../technical/audio/audiodriver.cpp \
    ../technical/util.cpp \
    ../technical/stringiterator.cpp \
    ../technical/video/videofactory.cpp \
    ../technical/video/videoencoder.cpp \
    ../technical/audio/ossdriver.cpp \
    ../technical/projectserializer.cpp \
    ../domain/observernotifier.cpp \
    ../application/externalcommandwithtemporarydirectory.cpp \
    ../application/externalcommand.cpp \
    ../foundation/logger.cpp
SOURCES += \
    tmain.cpp \
    texecutor.cpp \
    tcache.cpp \
    testundo.cpp \
    testhome.cpp \
    hash.cpp \
    tstopmotionundo.cpp
FORMS += 
RESOURCES += 
INCLUDEPATH += ../.. \
    $$system(xml2-config --cflags | sed -e 's/-I//g')
oomteststub.target = oomteststub.o
oomteststub.commands = make -f oomtestutil.mak oomteststub.o
oomteststub.depends = oomteststub.cpp oomtestutil.h
oomtestutil.target = oomtestutil.so
oomtestutil.commands = make -f oomtestutil.mak oomtestutil.so
oomtestutil.depends = oomtestutil.cpp
QMAKE_EXTRA_TARGETS += oomtestutil oomteststub test
QMAKE_CLEAN += oomteststub.o oomtestutil.so
PRE_TARGETDEPS = oomteststub.o
LIBS += oomteststub.o -ldl \
	$$system(sdl-config --libs) \
	$$system(xml2-config --libs) \
	-lSDL_image -ltar -lvorbisfile -lX11
DESTDIR=.
release:OBJECTS_DIR=build/release
release:TARGET = test
debug:OBJECTS_DIR=build/debug
debug:TARGET = test-d
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
test.target=test
test.commands=LD_PRELOAD=./oomtestutil.so ./test-d
test.depends=oomtestutil.so test-d
