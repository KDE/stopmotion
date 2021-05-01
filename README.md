Linux Stopmotion
================

Stopmotion is a program to create stop motion animations.
See <http://linuxstopmotion.org> for more info.


INSTALLATION
------------

You need the following packages installed to compile:
- C++ compiler (g++ or Clang)
- CMake
- pkg-config
- Qt5 >= 5.11 (development modules base, multimedia) (`qtbase5-dev`, `qttools5-dev-tools`, `qtmultimedia5-dev` and `libqt5multimedia5-plugins`)
- libtar (`libtar-dev`)
- libXML2 (`libxml2-dev`)
- vorbisfile (`libvorbis-dev`)

For example, in Ubuntu or Debian you would open a terminal and type:

    sudo apt install build-essential gdb qtbase5-dev qttest5-dev-tools libtar-dev libxml2-dev libvorbis-dev pkg-config git vgrabbj uvccapture

In openSUSE you type:

    sudo zypper install cmake g++ libQt5Core-devel libQt5Multimedia5 libtar-devel libvorbisfile-devel libxml2-devel

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

The mailing list is at `linuxstopmotion-users-devs@lists.sourceforge.net`

Your help is very much appreciated.

DEVELOPMENT
-----------

Attention, this section is outdated and does not reflect the changes by the switch to CMake.

After qmake, the following make targets are available:

- `make`: build a release version of the project, called stopmotion
- `make debug`: build a debug version of the project, called stopmotion-d
- `make all`: both debug and release versions
- `make test`: build and run the unit tests
- `make clean`: remove all build products
- `make install`: install stopmotion
- `make uninstall`: remove installation

To install in a specific location:

    qmake PREFIX=/specific/location HTML_PATH=/docs/go/here TRANSLATIONS_PATH=/translations/go/here
    sudo make install

The defines are optional. `PREFIX` defaults to `/usr/local`.
`HTML_PATH` defaults to `${PREFIX}/share/doc/stopmotion/html`.
`TRANSLATIONS_PATH` defaults to `${PREFIX}/share/doc/stopmotion/translations`.

To put the files somewhere before copying them for installation (after `qmake`):

    make INSTALL_ROOT=/somewhere/writeable install
    cp -fr /somewhere/writeable/* /

`PREFIX` is where the files will ultimately end up. `make install` will put them in
`$(INSTALL_ROOT)$(PREFIX)`

Please see `src/domain/undo/README.md` for some notes on Stopmotion's
architecture.
