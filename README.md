Linux Stopmotion
================

Stopmotion is a program to create stop motion animations.
See <http://linuxstopmotion.org> for more info.


INSTALLATION
------------

You need the following packages installed to compile:
- make and gcc (`build-essential`)
- `gdb`
- Qt4 (`libqt4-dev` and `qt4-dev-tools`)
- tar (`libtar-dev`)
- XML2 (`libxml2-dev`)
- vorbisfile (`libvorbis-dev`)
- `pkg-config`

Then you can type the following to build:

    qmake
    sudo make install

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
