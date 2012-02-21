#configuration
CONFIG +=  exceptions no_mocdepend debug stl qt_no_framework
QT_ARCH = i386
QT_EDITION = Trolltech
QT_CONFIG +=  qt3support phonon accessibility opengl minimal-config small-config medium-config large-config full-config reduce_exports ipv6 clock-gettime clock-monotonic mremap getaddrinfo ipv6ifname getifaddrs inotify system-jpeg system-mng system-png png system-tiff system-freetype system-zlib nis cups iconv glib gstreamer dbus openssl x11sm xshape xrandr xrender fontconfig tablet xkb xmlpatterns svg webkit debug

#versioning
QT_VERSION = 4.4.2
QT_MAJOR_VERSION = 4
QT_MINOR_VERSION = 4
QT_PATCH_VERSION = 2

#namespaces
QT_LIBINFIX = 
QT_NAMESPACE = 

QMAKE_RPATHDIR += "/home/bnilsen/dev/qt-4.4/lib"
