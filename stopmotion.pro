# print deprecation warning
message("qmake support is deprecated and will be removed after Linux Stopmotion 0.8.6. Use CMake instead.")

QT += widgets multimedia
CONFIG += debug_and_release
HEADERS += src/domain/undo/filelogger.h \
    src/config.h \
    src/domain/domainfacade.h \
    src/domain/filenamevisitor.h \
    src/domain/observernotifier.h \
    src/foundation/logger.h \
    src/foundation/uiexception.h \
    src/foundation/stringwriter.h \
    src/presentation/observer.h \
    src/domain/animation/animation.h \
    src/domain/animation/animationimpl.h \
    src/domain/animation/frame.h \
    src/domain/animation/sound.h \
    src/presentation/loadcache.h \
    src/presentation/imagecache.h \
    src/presentation/frontends/nonguifrontend/nonguifrontend.h \
    src/presentation/frontends/qtfrontend/mainwindowgui.h \
    src/presentation/frontends/qtfrontend/qtfrontend.h \
    src/application/runanimationhandler.h \
    src/domain/undo/commandmove.h \
    src/domain/undo/commandremove.h \
    src/domain/undo/commandsetimage.h \
    src/domain/undo/commandaddsound.h \
    src/domain/undo/commandrenamesound.h \
    src/domain/undo/commandremovesound.h \
    src/domain/undo/commandaddscene.h \
    src/domain/undo/commandmovescene.h \
    src/domain/undo/commandremovescene.h \
    src/domain/animation/workspacefile.h \
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
    src/presentation/frontends/qtfrontend/licence.h \
    src/technical/audio/audiodriver.h \
    src/technical/audio/audioformat.h \
    src/technical/audio/oggvorbis.h \
    src/technical/audio/qtaudiodriver.h \
    src/technical/grabber/imagegrabber.h \
    src/technical/grabber/commandlinegrabber.h \
    src/presentation/frontends/selection.h \
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
    src/presentation/frontends/qtfrontend/framebar/filenamesfromurlsiterator.h \
    src/presentation/frontends/qtfrontend/framebar/scenearrowbutton.h \
    src/presentation/frontends/qtfrontend/framebar/scenethumbview.h \
    src/presentation/frontends/qtfrontend/framebar/thumbview.h \
    src/presentation/frontends/qtfrontend/framebar/thumbdragger.h \
    src/presentation/frontends/qtfrontend/exporttab.h \
    src/presentation/frontends/qtfrontend/helpwindow.h \
    src/presentation/frontends/qtfrontend/editobserver.h \
    src/technical/util.h \
    src/technical/stringiterator.h \
    src/application/externalcommand.h \
    src/application/externalcommandwithtemporarydirectory.h \
    src/presentation/frontends/qtfrontend/aboutdialog.h \
    src/presentation/frontends/qtfrontend/devicetab.h \
    src/domain/undo/command.h \
    src/domain/undo/undoredoobserver.h \
    src/domain/undo/commandlogger.h \
    src/domain/undo/addallcommands.h \
    src/domain/undo/executor.h
FORMS += src/presentation/frontends/qtfrontend/helpbrowser.ui \
    src/presentation/frontends/qtfrontend/toolsmenu.ui
SOURCES += src/config.cpp \
    src/main.cpp \
    src/domain/domainfacade.cpp \
    src/domain/observernotifier.cpp \
    src/foundation/logger.cpp \
    src/foundation/uiexception.cpp \
    src/foundation/stringwriter.cpp \
    src/domain/animation/animation.cpp \
    src/domain/animation/animationimpl.cpp \
    src/domain/animation/frame.cpp \
    src/domain/animation/sound.cpp \
    src/domain/animation/scenevector.cpp \
    src/domain/animation/errorhandler.cpp \
    src/presentation/imagecache.cpp \
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
    src/domain/undo/filelogger.cpp \
    src/domain/animation/workspacefile.cpp \
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
    src/technical/audio/oggvorbis.cpp \
    src/technical/audio/audioformat.cpp \
    src/technical/audio/qtaudiodriver.cpp \
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
    src/presentation/frontends/qtfrontend/framebar/filenamesfromurlsiterator.cpp \
    src/presentation/frontends/qtfrontend/framebar/scenearrowbutton.cpp \
    src/presentation/frontends/qtfrontend/framebar/scenethumbview.cpp \
    src/presentation/frontends/qtfrontend/framebar/thumbview.cpp \
    src/presentation/frontends/qtfrontend/framebar/thumbdragger.cpp \
    src/presentation/frontends/qtfrontend/exporttab.cpp \
    src/presentation/frontends/qtfrontend/helpwindow.cpp \
    src/presentation/frontends/qtfrontend/editobserver.cpp \
    src/technical/util.cpp \
    src/technical/stringiterator.cpp \
    src/application/externalcommand.cpp \
    src/application/externalcommandwithtemporarydirectory.cpp \
    src/presentation/frontends/qtfrontend/aboutdialog.cpp \
    src/presentation/frontends/qtfrontend/devicetab.cpp \
    src/presentation/frontends/frontend.cpp \
    src/domain/undo/command.cpp \
    src/domain/undo/undoredoobserver.cpp \
    src/domain/undo/commandlogger.cpp \
    src/domain/undo/executor.cpp \
    src/domain/undo/addallcommands.cpp \
    src/domain/undo/random.cpp
TRANSLATIONS += $$system(ls -1 translations/*.ts)
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
    src/presentation
LIBS += -ltar -lvorbisfile

QM_FILES = $$TRANSLATIONS
QM_FILES~= s:^translations/:build/:g
QM_FILES~= s:[.]ts$:.qm:g

DISTFILES += src/config.cpp.in \
    graphics/stopmotion.png \
    stopmotion.pro.in \
    stopmotion.desktop \
    stopmotion.mime \
    stopmotion.1 \
    configure \
    AUTHORS \
    README.md \
    COPYING \
    $$system(ls -1 manual/*.html) \
    $$system(ls -1 manual/graphic/*.png) \
    $$system(ls -1 manual/icons/*.png) \
    $$system(ls -1 manual/screenshots/*.png) \
    $$QM_FILES
DISTFILES -= stopmotion.pro \
             src/config.cpp \
             stopmotion-d

CONFIG += link_pkgconfig

PKGCONFIG += vorbisfile libxml-2.0

isEmpty(PREFIX) {
	PREFIX =/usr/local
}

isEmpty(HTML_PATH) {
	HTML_PATH =$${PREFIX}/share/doc/stopmotion/html
}

isEmpty(TRANSLATIONS_PATH) {
	TRANSLATIONS_PATH =$${PREFIX}/share/stopmotion/translations
}

DEFINES += TRANSLATIONS_PATH=\'\"$${TRANSLATIONS_PATH}\"\'
DEFINES += HTML_PATH=\'\"$${HTML_PATH}\"\'
TEMPLATE = app
DESTDIR=.

CONFIG(release,debug|release) {
	TARGET=stopmotion
	OBJECTS_DIR=build/release
	DEFINES+= NO_DEBUG
}
CONFIG(debug,debug|release) {
	TARGET=stopmotion-d
	OBJECTS_DIR=build/debug
}
MOC_DIR = build
RCC_DIR = build
UI_DIR = build

target.path = $${PREFIX}/bin

translations.path = $${TRANSLATIONS_PATH}
translations.files = $${QM_FILES}
translations.CONFIG += no_check_exist

docgraphics.path = $${HTML_PATH}/graphic
docgraphics.files = manual/graphic/*.png

docicons.path = $${HTML_PATH}/icons
docicons.files = manual/icons/*.png

docscreens.path = $${HTML_PATH}/screenshots
docscreens.files = manual/screenshots/*.png

htmldoc.path = $${HTML_PATH}
htmldoc.files = manual/*.html

icon.path = $${PREFIX}/share/icons
icon.files = graphics/stopmotion.svg

desktop.path = $${PREFIX}/share/applications
desktop.files = stopmotion.desktop

INSTALLS += target translations icon desktop \
	docgraphics docicons docscreens htmldoc

QMAKE_STRIP=:

# remove old Qt4 generated files, if present for any reason
QMAKE_CLEAN += ui_helpbrowser.h ui_toolsmenu.h

# Build translations
QMAKE_EXTRA_COMPILERS += lrelease
lrelease.input    = TRANSLATIONS
lrelease.output   = build/${QMAKE_FILE_BASE}.qm
lrelease.commands = $$[QT_INSTALL_BINS]/lrelease ${QMAKE_FILE_IN} -qm build/${QMAKE_FILE_BASE}.qm
lrelease.CONFIG  += no_link target_predeps

# Build and run tests with make test
QMAKE_EXTRA_TARGETS += test
test.commands = cd src/test && $(QMAKE) -spec $$QMAKESPEC && make test
