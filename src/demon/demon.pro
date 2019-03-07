TARGET = QtDemon
MODULE = demon

QT = core gui

DEFINES += QT_BUILD_DEMON_LIB

HEADERS += \
    qtdemonglobal.h \
    qtdemonglobal_p.h \
    qdemonbounds3.h \
    qdemontransform.h \
    qdemonplane.h \
    qdemonutils.h \
    qdemondataref.h \
    qdemonoption.h \
    qdemoninvasivelinkedlist.h \
    qdemoninvasiveset.h \
    qdemondiscriminatedunion.h \
    qdemonperftimer.h \
    qdemontime.h

SOURCES += \
    qdemonbounds3.cpp \
    qdemontransform.cpp \
    qdemonplane.cpp \
    qdemonutils.cpp \
    qdemondataref.cpp \
    qdemonperftimer.cpp \
    qdemontime.cpp

load(qt_module)
