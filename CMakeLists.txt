###
# general project setup
###
cmake_minimum_required(VERSION 3.13)

project(linux-stopmotion
  VERSION 0.8.6
  LANGUAGES CXX)

# set project options required by Qt
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

###
# find external dependencies
###

# Qt
find_package(Qt5 5.11
  COMPONENTS Widgets Multimedia LinguistTools
  )

# LibXml2
find_package(LibXml2
  REQUIRED)

# libtar
find_file(TAR_INCLUDE_PATH
  NAMES libtar.h
  REQUIRED)
find_library(TAR_LIBRARY_PATH
  NAMES tar
  REQUIRED)

# libVorbis
find_file(VORBISFILE_INCLUDE_PATH
  NAMES "vorbis/vorbisfile.h"
  REQUIRED)
find_library(VORBISFILE_LIBRARY_PATH
  NAMES vorbisfile
  REQUIRED)

include(FeatureSummary)
feature_summary(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND)

###
# handle translation files
###
qt5_add_translation(QM_FILES
  translations/stopmotion_ar.ts
  translations/stopmotion_br.ts
  translations/stopmotion_cz.ts
  translations/stopmotion_de.ts
  translations/stopmotion_dk.ts
  translations/stopmotion_es.ts
  translations/stopmotion_kl.ts
  translations/stopmotion_no_nb.ts
  translations/stopmotion_no_nn.ts
  translations/stopmotion_no_se.ts
  translations/stopmotion_pt.ts
  translations/stopmotion_ru.ts
  translations/stopmotion_fi.ts
  translations/stopmotion_fr.ts
  translations/stopmotion_gr.ts
  translations/stopmotion_he.ts
  translations/stopmotion_it.ts
  translations/stopmotion_sl.ts
  translations/stopmotion_sv.ts
  translations/stopmotion_tr.ts
  translations/stopmotion_zh_CN.ts
  translations/stopmotion_zh_TW.ts
  )
configure_file(translations/translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)

###
# create executable and link libraries
###
add_executable(stopmotion
  src/main.cpp
  src/config.cpp
  src/domain/domainfacade.cpp
  src/domain/observernotifier.cpp
  src/foundation/logger.cpp
  src/foundation/uiexception.cpp
  src/foundation/stringwriter.cpp
  src/domain/animation/animation.cpp
  src/domain/animation/animationimpl.cpp
  src/domain/animation/frame.cpp
  src/domain/animation/sound.cpp
  src/domain/animation/scenevector.cpp
  src/domain/animation/errorhandler.cpp
  src/presentation/imagecache.cpp
  src/presentation/frontends/nonguifrontend/nonguifrontend.cpp
  src/presentation/frontends/qtfrontend/mainwindowgui.cpp
  src/presentation/frontends/qtfrontend/qtfrontend.cpp
  src/application/runanimationhandler.cpp
  src/domain/undo/commandadd.cpp
  src/domain/undo/commandmove.cpp
  src/domain/undo/commandremove.cpp
  src/domain/undo/commandsetimage.cpp
  src/domain/undo/commandaddsound.cpp
  src/domain/undo/commandrenamesound.cpp
  src/domain/undo/commandremovesound.cpp
  src/domain/undo/commandaddscene.cpp
  src/domain/undo/commandmovescene.cpp
  src/domain/undo/commandremovescene.cpp
  src/domain/undo/filelogger.cpp
  src/domain/animation/workspacefile.cpp
  src/application/camerahandler.cpp
  src/presentation/frontends/qtfrontend/frameview.cpp
  src/presentation/frontends/qtfrontend/flexiblespinbox.cpp
  src/presentation/frontends/qtfrontend/menuframe.cpp
  src/presentation/frontends/qtfrontend/flexiblelineedit.cpp
  src/application/soundhandler.cpp
  src/presentation/frontends/qtfrontend/framepreferencesmenu.cpp
  src/application/editmenuhandler.cpp
  src/domain/animation/scene.cpp
  src/technical/projectserializer.cpp
  src/technical/audio/oggvorbis.cpp
  src/technical/audio/audioformat.cpp
  src/technical/audio/qtaudiodriver.cpp
  src/technical/grabber/imagegrabber.cpp
  src/technical/grabber/commandlinegrabber.cpp
  src/presentation/frontends/qtfrontend/imagegrabthread.cpp
  src/presentation/frontends/qtfrontend/preferencesmenu.cpp
  src/foundation/preferencestool.cpp
  src/application/modelhandler.cpp
  src/presentation/frontends/qtfrontend/toolsmenu.cpp
  src/application/languagehandler.cpp
  src/technical/video/videofactory.cpp
  src/technical/video/videoencoder.cpp
  src/presentation/frontends/qtfrontend/importtab.cpp
  src/presentation/frontends/qtfrontend/framebar/framebar.cpp
  src/presentation/frontends/qtfrontend/framebar/framethumbview.cpp
  src/presentation/frontends/qtfrontend/framebar/filenamesfromurlsiterator.cpp
  src/presentation/frontends/qtfrontend/framebar/scenearrowbutton.cpp
  src/presentation/frontends/qtfrontend/framebar/scenethumbview.cpp
  src/presentation/frontends/qtfrontend/framebar/thumbview.cpp
  src/presentation/frontends/qtfrontend/framebar/thumbdragger.cpp
  src/presentation/frontends/qtfrontend/exporttab.cpp
  src/presentation/frontends/qtfrontend/helpwindow.cpp
  src/presentation/frontends/qtfrontend/editobserver.cpp
  src/technical/util.cpp
  src/technical/stringiterator.cpp
  src/application/externalcommand.cpp
  src/application/externalcommandwithtemporarydirectory.cpp
  src/presentation/frontends/qtfrontend/aboutdialog.cpp
  src/presentation/frontends/qtfrontend/devicetab.cpp
  src/presentation/frontends/frontend.cpp
  src/domain/undo/command.cpp
  src/domain/undo/undoredoobserver.cpp
  src/domain/undo/commandlogger.cpp
  src/domain/undo/executor.cpp
  src/domain/undo/addallcommands.cpp
  src/domain/undo/random.cpp
  ${CMAKE_BINARY_DIR}/translations.qrc
  ${QM_FILES}
  )

# link to Qt libraries
target_link_libraries(stopmotion
  PUBLIC
  Qt5::Core
  Qt5::Widgets
  Qt5::Multimedia
  )
# link to LibXml2
target_link_libraries(stopmotion
  PUBLIC LibXml2::LibXml2)
# link to libVorbis, libtar
target_link_libraries(stopmotion
  PUBLIC
  ${VORBISFILE_LIBRARY_PATH}
  ${TAR_LIBRARY_PATH}
  )

target_include_directories(stopmotion
  PUBLIC
  .
  src
  src/domain
  src/domain/animation
  src/foundation
  src/presentation
  src/presentation/frontends/nonguifrontend
  src/presentation/frontends/qtfrontend
  )
