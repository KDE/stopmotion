# SPDX-FileCopyrightText: 2021-2026 Linuxstopmotion contributors; see the AUTHORS file for details
# SPDX-License-Identifier: GPL-2.0-or-later

###
# general project setup
###
cmake_minimum_required(VERSION 3.22)

project(stopmotion
  LANGUAGES CXX)
set(PROJECT_VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use KDE's build settings from extra-cmake-modules
find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs6)
include(KDECMakeSettings)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
include(KDECompilerSettings NO_POLICY_SCOPE)
kde_enable_exceptions()
# not yet ready for these Qt definitions set by KDECompilerSettings
remove_definitions(
  -DQT_NO_CAST_FROM_ASCII
  -DQT_NO_KEYWORDS
  )

###
# find external dependencies
###

# Qt
find_package(Qt6 6.5
  COMPONENTS Widgets Multimedia LinguistTools
  REQUIRED)

# LibXml2
find_package(LibXml2
  REQUIRED)

# KArchive, to read and write project files
find_package(KF6Archive 6.0
  REQUIRED)

# libVorbisFile
find_file(VORBISFILE_INCLUDE_PATH
  NAMES "vorbis/vorbisfile.h"
  REQUIRED)
find_library(VORBISFILE_LIBRARY_PATH
  NAMES vorbisfile
  REQUIRED)

if(BUILD_TESTING)
  find_package(Qt6 6.5 REQUIRED
    COMPONENTS Test
  )
endif ()

include(FeatureSummary)
feature_summary(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND)

add_compile_definitions(STOPMOTION_VERSION="${PROJECT_VERSION}")

# where translations and the manual are installed and looked up at runtime;
# packagers can override the latter with TRANSLATIONS_PATH and HTML_PATH
set(STOPMOTION_TRANSLATIONS_INSTALL_DIR ${KDE_INSTALL_DATADIR}/stopmotion/translations)
set(STOPMOTION_DOC_INSTALL_DIR ${KDE_INSTALL_DATAROOTDIR}/doc/stopmotion)
if(NOT TRANSLATIONS_PATH)
  set(TRANSLATIONS_PATH ${CMAKE_INSTALL_PREFIX}/${STOPMOTION_TRANSLATIONS_INSTALL_DIR})
endif()
if(NOT HTML_PATH)
  set(HTML_PATH ${CMAKE_INSTALL_PREFIX}/${STOPMOTION_DOC_INSTALL_DIR}/manual)
endif()
add_compile_definitions(TRANSLATIONS_PATH="${TRANSLATIONS_PATH}")
add_compile_definitions(HTML_PATH="${HTML_PATH}")

add_subdirectory(src)
add_subdirectory(translations)

###
# handle translation files
###
file(GLOB TS_FILES "translations/stopmotion_*.ts")
qt_add_translation(QM_FILES "${TS_FILES}")

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
  translations.qrc
  ${QM_FILES}
  )

# link to Qt libraries
target_link_libraries(stopmotion
  PUBLIC
  Qt6::Core
  Qt6::Widgets
  Qt6::Multimedia
  )
# link to LibXml2
target_link_libraries(stopmotion
  PUBLIC LibXml2::LibXml2)
# link to libVorbis, KArchive
target_link_libraries(stopmotion
  PUBLIC
  ${VORBISFILE_LIBRARY_PATH}
  KF6::Archive
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

###
# handle installation
###
install(TARGETS stopmotion ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES stopmotion.desktop
  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES graphics/stopmotion.png
  DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/32x32/apps)
install(FILES graphics/stopmotion.svg
  DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES ${QM_FILES}
  DESTINATION ${STOPMOTION_TRANSLATIONS_INSTALL_DIR})
install(FILES stopmotion.1
  DESTINATION ${KDE_INSTALL_MANDIR}/man1)
install(DIRECTORY manual
  DESTINATION ${STOPMOTION_DOC_INSTALL_DIR})
