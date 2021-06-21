if(EXISTS "${DL_LIBRARY_PATH}" AND "${Qt5Test_FOUND}")
  add_library(oomtestutil
    SHARED EXCLUDE_FROM_ALL
    oomtestutil.cpp)
  target_compile_definitions(oomtestutil
    PUBLIC
    _GNU_SOURCE)
  set_target_properties(oomtestutil
    PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
  target_link_libraries(oomtestutil
    PUBLIC
    ${DL_LIBRARY_PATH})

  add_library(oomteststub
    EXCLUDE_FROM_ALL
    oomteststub.cpp)
  target_compile_definitions(oomteststub
    PUBLIC
    _GNU_SOURCE)
  set_target_properties(oomteststub
    PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
  target_link_libraries(oomteststub
    PUBLIC
    ${DL_LIBRARY_PATH})

  add_executable(test-stopmotion
    tmain.cpp
    toggvorbis.cpp
    texecutor.cpp
    tcache.cpp
    testundo.cpp
    testhome.cpp
    fakefiles.cpp
    hash.cpp
    tworkspace.cpp
    tstopmotionundo.cpp
    ../domain/undo/filelogger.cpp
    ../domain/undo/command.cpp
    ../domain/undo/commandlogger.cpp
    ../domain/undo/executor.cpp
    ../domain/undo/addallcommands.cpp
    ../domain/undo/random.cpp
    ../domain/undo/commandadd.cpp
    ../domain/undo/commandmove.cpp
    ../domain/undo/commandremove.cpp
    ../domain/undo/commandsetimage.cpp
    ../domain/undo/commandaddsound.cpp
    ../domain/undo/commandrenamesound.cpp
    ../domain/undo/commandremovesound.cpp
    ../domain/undo/commandaddscene.cpp
    ../domain/undo/commandmovescene.cpp
    ../domain/undo/commandremovescene.cpp
    ../domain/animation/scene.cpp
    ../domain/animation/frame.cpp
    ../domain/animation/sound.cpp
    ../domain/animation/animation.cpp
    ../domain/animation/animationimpl.cpp
    ../domain/animation/scenevector.cpp
    ../domain/animation/workspacefile.cpp
    ../domain/animation/errorhandler.cpp
    ../domain/observernotifier.cpp
    ../technical/audio/audioformat.cpp
    ../technical/util.cpp
    ../technical/stringiterator.cpp
    ../technical/video/videofactory.cpp
    ../technical/video/videoencoder.cpp
    ../technical/audio/qtaudiodriver.cpp
    ../technical/projectserializer.cpp
    ../presentation/frontends/frontend.cpp
    ../application/externalcommandwithtemporarydirectory.cpp
    ../application/externalcommand.cpp
    ../foundation/stringwriter.cpp
    ../foundation/uiexception.cpp
    ../foundation/logger.cpp
    )

set_target_properties(test-stopmotion
  PROPERTIES
  EXCLUDE_FROM_ALL true)

# link to Qt libraries
target_link_libraries(test-stopmotion
  PUBLIC
  Qt5::Core
  Qt5::Widgets
  Qt5::Multimedia
  Qt5::Test
  )
# link to LibXml2
target_link_libraries(test-stopmotion
  PUBLIC LibXml2::LibXml2)
# link to libVorbisFile, libtar
target_link_libraries(test-stopmotion
  PUBLIC
  oomtestutil oomteststub
  ${VORBISFILE_LIBRARY_PATH}
  ${TAR_LIBRARY_PATH}
  ${DL_LIBRARY_PATH}
  )

target_include_directories(test-stopmotion
  PUBLIC
  ${CMAKE_SOURCE_DIR})

# add test executable to CTest suite
add_test(
  NAME test-stopmotion
  COMMAND test-stopmotion)

# link files required for test
file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
     COPY_ON_ERROR SYMBOLIC)

endif()
