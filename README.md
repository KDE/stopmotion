KDE Stopmotion
==============

Stopmotion is a program to create stop motion animations.
See <https://invent.kde.org/multimedia/stopmotion> for more information.

Formerly it was known under the name of _Linux Stopmotion_, there is
still documentation at <http://linuxstopmotion.org>.


INSTALLATION
------------

You need the following packages installed to compile:
- C++ compiler (g++ or Clang)
- CMake >= 3.16
- pkg-config
- Qt6 >= 6.3 (development modules base, multimedia) (`qtbase5-dev`, `qttools5-dev-tools`, `qttools5-dev`, `qtmultimedia5-dev` and `libqt5multimedia5-plugins`)
- libarchive (`libarchive-dev`)
- libXML2 (`libxml2-dev`)
- vorbisfile (`libvorbis-dev`)

For example, in Ubuntu or Debian you would open a terminal and type:

    sudo apt install build-essential gdb qtbase5-dev qttools5-dev qttest5-dev-tools libarchive-dev libxml2-dev libvorbis-dev pkg-config git vgrabbj uvccapture

In openSUSE you type:

    sudo zypper install cmake gcc-c++ libQt5Core-devel libQt5Widgets-devel libQt5Multimedia5 libarchive-devel libvorbis-devel libxml2-devel

Once the dependencies are installed you can navigate to the source
code directory and type the following to build Stopmotion:

    mkdir build-cmake
    cd build-cmake
    cmake ..
    cmake --build .

If you want to install, make sure the user has sufficient rights (or switch to su)

    cmake --install .


WHO CAN USE IT
--------------
Everyone can use this application as it is distributed under the
GPL licence (See the file `COPYING` for more details).


DO YOU WANT TO CONTRIBUTE?
--------------------------
Do not hesitate to contact us if you want to contribute with translations,
graphics, code, +++

The mailing list is at `stopmotion@kde.org`

Your help is very much appreciated.

DEVELOPMENT
-----------

After `cmake`, the following make targets are available:

- `make`: build the project, the resulting executable is called `stopmotion`
- `make test-stopmotion`: build unit tests
- `make test`: run the unit tests
- `make clean`: remove all build products
- `make install`: install stopmotion

You can tell CMake to install in a specific location:

    cmake -DCMAKE_INSTALL_PREFIX=<specific/location> <path/to/source/dir>

You can also specify the path to the translation and HTML documentation path
by setting `-DTRANSLATION_PATH=` and `-DHTML_PATH=`.

Please see `src/domain/undo/README.md` for some notes on Stopmotion's
architecture.
