CONFIG += release \
    warn_off
HEADERS += src/domain/undo/filelogger.h \
    src/config.h \
    src/domain/domainfacade.h \
    src/domain/filenamevisitor.h \
    src/foundation/logger.h \
    src/presentation/observer.h \
    src/domain/animation/animation.h \
    src/domain/animation/animationmodel.h \
    src/domain/animation/frame.h \
    src/presentation/frontends/nonguifrontend/nonguifrontend.h \
    src/presentation/frontends/qtfrontend/mainwindowgui.h \
    src/presentation/frontends/qtfrontend/qtfrontend.h \
    src/application/runanimationhandler.h \
    src/domain/undo/commandadd.h \
    src/domain/undo/commandmove.h \
    src/domain/undo/commandremove.h \
    src/domain/undo/commandsetimage.h \
    src/domain/undo/commandaddsound.h \
    src/domain/undo/commandrenamesound.h \
    src/domain/undo/commandremovesound.h \
    src/domain/undo/commandaddscene.h \
    src/domain/undo/commandmovescene.h \
    src/domain/undo/commandremovescene.h \
    src/application/camerahandler.h \
    src/presentation/frontends/qtfrontend/frameview.h \
    src/presentation/frontends/qtfrontend/flexiblespinbox.h \
    src/presentation/frontends/qtfrontend/menuframe.h \
    src/presentation/frontends/qtfrontend/flexiblelineedit.h \
    src/presentation/frontends/frontend.h \
    graphics/stopmotion.xpm \
    graphics/stopmotion_logo.xpm \
    graphics/icons/addframeicon.xpm \
    graphics/icons/cameraoff.xpm \
    graphics/icons/cameraon.xpm \
    graphics/icons/capture.xpm \
    graphics/icons/clapper.xpm \
    graphics/icons/closescene.xpm \
    graphics/icons/close.xpm \
    graphics/icons/configureicon.xpm \
    graphics/icons/copyicon.xpm \
    graphics/icons/cuticon.xpm \
    graphics/icons/fastforward.xpm \
    graphics/icons/filenewicon.xpm \
    graphics/icons/fileopenicon.xpm \
    graphics/icons/filesaveasicon.xpm \
    graphics/icons/filesaveicon.xpm \
    graphics/icons/home.xpm \
    graphics/icons/languages.xpm \
    graphics/icons/loop.xpm \
    graphics/icons/newscene.xpm \
    graphics/icons/nextframe.xpm \
    graphics/icons/note.xpm \
    graphics/icons/openscene.xpm \
    graphics/icons/pasteicon.xpm \
    graphics/icons/pause.xpm \
    graphics/icons/play.xpm \
    graphics/icons/previousframe.xpm \
    graphics/icons/quiticon.xpm \
    graphics/icons/redoicon.xpm \
    graphics/icons/removeframeicon.xpm \
    graphics/icons/removescene.xpm \
    graphics/icons/rewind.xpm \
    graphics/icons/steptobeginning.xpm \
    graphics/icons/steptoend.xpm \
    graphics/icons/stop.xpm \
    graphics/icons/undoicon.xpm \
    graphics/icons/videoexport.xpm \
    graphics/icons/whatsthisicon.xpm \
    graphics/icons/windowicon.xpm \
    graphics/icons/gimp.xpm \
    src/application/soundhandler.h \
    src/presentation/frontends/qtfrontend/framepreferencesmenu.h \
    src/application/editmenuhandler.h \
    src/domain/animation/scene.h \
    src/domain/animation/scenevector.h \
    src/technical/projectserializer.h \
    src/technical/packer.h \
    src/presentation/frontends/qtfrontend/licence.h \
    src/technical/audio/audiodriver.h \
    src/technical/audio/audioformat.h \
    src/technical/audio/ossdriver.h \
    src/technical/audio/oggvorbis.h \
    src/technical/grabber/imagegrabber.h \
    src/technical/grabber/commandlinegrabber.h \
    src/presentation/frontends/qtfrontend/imagegrabthread.h \
    src/presentation/frontends/qtfrontend/preferencesmenu.h \
    src/foundation/preferencestool.h \
    src/application/modelhandler.h \
    src/presentation/frontends/qtfrontend/toolsmenu.h \
    src/application/languagehandler.h \
    src/technical/video/videofactory.h \
    src/technical/video/videoencoder.h \
    src/presentation/frontends/qtfrontend/importtab.h \
    src/presentation/frontends/qtfrontend/framebar/framebar.h \
    src/presentation/frontends/qtfrontend/framebar/framethumbview.h \
    src/presentation/frontends/qtfrontend/framebar/scenearrowbutton.h \
    src/presentation/frontends/qtfrontend/framebar/scenethumbview.h \
    src/presentation/frontends/qtfrontend/framebar/thumbview.h \
    src/presentation/frontends/qtfrontend/exporttab.h \
    src/application/externalchangemonitor.h \
    src/presentation/frontends/qtfrontend/helpwindow.h \
    src/technical/util.h \
    src/application/externalcommand.h \
    src/presentation/frontends/qtfrontend/aboutdialog.h \
    src/technical/libng/devices.h \
    src/technical/libng/grab-ng.h \
    src/technical/libng/list.h \
    src/technical/libng/struct-dump.h \
    src/technical/libng/struct-v4l2.h \
    src/technical/libng/struct-v4l.h \
    src/technical/libng/videodev2.h \
    src/technical/libng/videodev.h \
    src/presentation/frontends/qtfrontend/devicetab.h \
    src/domain/undo/command.h \
    src/domain/undo/commandlogger.h \
    src/domain/undo/executor.h
FORMS += src/presentation/frontends/qtfrontend/helpbrowser.ui \
    src/presentation/frontends/qtfrontend/toolsmenu.ui
SOURCES += src/config.cpp \
    src/main.cpp \
    src/domain/domainfacade.cpp \
    src/foundation/logger.cpp \
    src/domain/animation/animation.cpp \
    src/domain/animation/animationmodel.cpp \
    src/domain/animation/frame.cpp \
    src/domain/animation/scenevector.cpp \
    src/presentation/frontends/nonguifrontend/nonguifrontend.cpp \
    src/presentation/frontends/qtfrontend/mainwindowgui.cpp \
    src/presentation/frontends/qtfrontend/qtfrontend.cpp \
    src/application/runanimationhandler.cpp \
    src/domain/undo/commandadd.cpp \
    src/domain/undo/commandmove.cpp \
    src/domain/undo/commandremove.cpp \
    src/domain/undo/commandsetimage.cpp \
    src/domain/undo/commandaddsound.cpp \
    src/domain/undo/commandrenamesound.cpp \
    src/domain/undo/commandremovesound.cpp \
    src/domain/undo/commandaddscene.cpp \
    src/domain/undo/commandmovescene.cpp \
    src/domain/undo/commandremovescene.cpp \
    src/application/camerahandler.cpp \
    src/presentation/frontends/qtfrontend/frameview.cpp \
    src/presentation/frontends/qtfrontend/flexiblespinbox.cpp \
    src/presentation/frontends/qtfrontend/menuframe.cpp \
    src/presentation/frontends/qtfrontend/flexiblelineedit.cpp \
    src/application/soundhandler.cpp \
    src/presentation/frontends/qtfrontend/framepreferencesmenu.cpp \
    src/application/editmenuhandler.cpp \
    src/domain/animation/scene.cpp \
    src/technical/projectserializer.cpp \
    src/technical/packer.c \
    src/technical/audio/ossdriver.cpp \
    src/technical/audio/oggvorbis.cpp \
    src/technical/grabber/imagegrabber.cpp \
    src/technical/grabber/commandlinegrabber.cpp \
    src/presentation/frontends/qtfrontend/imagegrabthread.cpp \
    src/presentation/frontends/qtfrontend/preferencesmenu.cpp \
    src/foundation/preferencestool.cpp \
    src/application/modelhandler.cpp \
    src/presentation/frontends/qtfrontend/toolsmenu.cpp \
    src/application/languagehandler.cpp \
    src/technical/video/videofactory.cpp \
    src/technical/video/videoencoder.cpp \
    src/presentation/frontends/qtfrontend/importtab.cpp \
    src/presentation/frontends/qtfrontend/framebar/framebar.cpp \
    src/presentation/frontends/qtfrontend/framebar/framethumbview.cpp \
    src/presentation/frontends/qtfrontend/framebar/scenearrowbutton.cpp \
    src/presentation/frontends/qtfrontend/framebar/scenethumbview.cpp \
    src/presentation/frontends/qtfrontend/framebar/thumbview.cpp \
    src/presentation/frontends/qtfrontend/exporttab.cpp \
    src/application/externalchangemonitor.cpp \
    src/presentation/frontends/qtfrontend/helpwindow.cpp \
    src/technical/util.cpp \
    src/application/externalcommand.cpp \
    src/presentation/frontends/qtfrontend/aboutdialog.cpp \
    src/technical/libng/devices.c \
    src/technical/libng/grab-ng.c \
    src/technical/libng/struct-dump.c \
    src/technical/libng/struct-v4l2.c \
    src/technical/libng/struct-v4l.c \
    src/technical/libng/plugins/drv0-v4l2.c \
    src/technical/libng/plugins/drv1-v4l.c \
    src/presentation/frontends/qtfrontend/devicetab.cpp \
    src/domain/undo/command.cpp \
    src/domain/undo/commandlogger.cpp \
    src/domain/undo/executor.cpp \
    src/domain/undo/random.cpp
TRANSLATIONS += translations/stopmotion_no_nb.ts \
    translations/stopmotion_no_nn.ts \
    translations/stopmotion_no_se.ts \
    translations/stopmotion_de.ts \
    translations/stopmotion_se.ts \
    translations/stopmotion_fr.ts \
    translations/stopmotion_br.ts \
    translations/stopmotion_dk.ts \
    translations/stopmotion_fi.ts \
    translations/stopmotion_gr.ts \
    translations/stopmotion_ru.ts \
    translations/stopmotion_es.ts \
    translations/stopmotion_kl.ts \
    translations/stopmotion_cz.ts \
    translations/stopmotion_sl.ts \
    translations/stopmotion_pt.ts \
    translations/stopmotion_it.ts \
    translations/stopmotion_tr.ts
DEPENDPATH += src \
    src/domain \
    src/foundation \
    src/presentation \
    src/domain/animation \
    src/presentation/frontends/nonguifrontend \
    src/presentation/frontends/qtfrontend
INCLUDEPATH += . \
    src \
    src/presentation/frontends/qtfrontend \
    src/presentation/frontends/nonguifrontend \
    src/foundation \
    src/domain \
    src/domain/animation \
    src/presentation \
    $$system(sdl-config --cflags | sed -e 's/-I//g') \
    $$system(xml2-config --cflags | sed -e 's/-I//g')
LIBS += $$system(sdl-config --libs) \
        $$system(xml2-config --libs) \
        -lSDL_image -ltar -lvorbisfile -linotifytools -lX11

DISTFILES += src/config.cpp.in \
    graphics/stopmotion.png \
    stopmotion.pro.in \
    stopmotion.desktop \
    stopmotion.mime \
    stopmotion.1 \
    configure \
    AUTHORS \
    README \
    COPYING \
    $$system(ls -x manual/*.html) \
    $$system(ls -x manual/graphic/*.png) \
    $$system(ls -x manual/icons/*.png) \
    $$system(ls -x manual/screenshots/*.png) \
    $$system(ls -x translations/*.qm)
DISTFILES -= stopmotion.pro \
    src/config.cpp
DEFINES += NO_DEBUG
TEMPLATE = app

release:DESTDIR=.
release:OBJECTS_DIR=build/release
debug:DESTDIR=build/debug
debug:OBJECTS_DIR=build/debug
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

target.path = /usr/bin
translations.path = /usr/share/stopmotion/translations
translations.files = translations/*.qm
htmldoc.path = /usr/share/doc/stopmotion/html
htmldoc.files = manual/*.html
htmldoc.extra = $(INSTALL_DIR) \
    manual/graphic \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html;
htmldoc.extra += $(INSTALL_DIR) \
    manual/icons \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html;
htmldoc.extra += $(INSTALL_DIR) \
    manual/screenshots \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html;

# Dummy target to fix permissions.
dummy.path = /usr/bin
dummy.extra += chmod \
    644 \
    $(INSTALL_ROOT)/usr/share/stopmotion/translations/*.qm \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/*.html \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/graphic/* \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/icons/* \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/screenshots/*;
dummy.extra += chmod \
    755 \
    $(INSTALL_ROOT)/usr/share/stopmotion/translations \
    $(INSTALL_ROOT)/usr/bin/$(QMAKE_TARGET) \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/graphic \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/icons \
    $(INSTALL_ROOT)/usr/share/doc/stopmotion/html/screenshots;
INSTALLS += target \
    translations \
    htmldoc \
    dummy
QMAKE_STRIP = :
