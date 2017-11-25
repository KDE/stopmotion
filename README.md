Linux Stopmotion
================

Stopmotion is a program to create stop motion animations.
See <http://linuxstopmotion.org> for more info.


INSTALLATION
------------

You need the following packages installed to compile:
- make and gcc (`build-essential`)
- `gdb`
- Qt5 (`qt5-default`)
- tar (`libtar-dev`)
- XML2 (`libxml2-dev`)
- vorbisfile (`libvorbis-dev`)
- `pkg-config`

For example, in Ubuntu or Debian you would open a terminal and type:

    sudo apt install build-essential gdb libqt4-dev qt4-dev-tools libtar-dev libxml2-dev libvorbis-dev pkg-config git vgrabbj uvccapture

In OpenSuSE you first type:

    sudo zypper install -t pattern devel_qt4
    sudo zypper install git libvorbis-devel libxml2-devel

Then open your browser and visit `https://rpm.pbone.net` and find both
`libtar`, `libtar1` and `libtar-devel` for the correct OS version, each
with the exact same version. Then install them like this (changing the
filenames as appropriate):

    sudo rpm -i ~/Downloads/libtar1-1.2.20-2.8.x86_64.rpm ~/Downloads/libtar-1.2.20-2.8.x86_64.rpm ~/Downloads/libtar-devel-1.2.20-2.8.x86_64.rpm

Once the dependencies are installed you can navigate to the source
code directory and type the following to build Stopmotion:

    qmake -qt=4
    sudo make install

If `qmake -qt=4` does not work, you can just use `qmake`.


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
