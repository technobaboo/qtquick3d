TARGET = QtDemon
MODULE = demon

QT = core gui

DEFINES += QT_BUILD_DEMON_LIB

HEADERS += \
    qtdemonglobal.h \
    qtdemonglobal_p.h \
    qdemonnocopy.h \
    qdemonrefcounted.h \
    qdemonflags.h \
    qdemonbounds3.h \
    qdemontransform.h \
    qdemonplane.h \
    qdemonutils.h \
    qdemondataref.h \
    qdemonoption.h \
    qdemoninvasivelinkedlist.h

SOURCES += \
    qdemonflags.cpp \
    qdemonbounds3.cpp \
    qdemontransform.cpp \
    qdemonplane.cpp \
    qdemonutils.cpp \
    qdemondataref.cpp \
    qdemonnocopy.cpp

load(qt_module)
