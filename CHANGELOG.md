0.9.0 (2025-05-25)
==================

Changes
* Port to Qt 6 (!25, thanks to Florian Satzger and Mark Penner)


0.8.7 (2024-06-09)
==================

Changes
* The project is now officially called to KDE Stopmotion.
* Support for qmake has been removed. Use CMake instead.

Features
* Port serialization to libarchive. libtar is abandoned. (thanks to
  Bastian Germann)

Bugfixes
* The .sto files miss the tar trailer. (#16, thanks to Bastian Germann
  for providing a fix)

Improvements
* Use pkg-config to find dependencies vorbisfile and xml2 (thanks to
  Barak Pearlmutter)
* Remove code that relies on deprecation in Qt 5, this is a
  preparation to move to Qt 6.


0.8.6 (2023-01-8)
=================

Changes
* New build system using CMake. The qmake one is deprecated and will be removed.

Features
* The test executable can be executed as a CMake test target
  (make test-stopmotion && make test).

Bugfixes

Improvements
* Fixed various warnings from Clang, GCC, and Qt 5.15.
