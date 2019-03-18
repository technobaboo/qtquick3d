TARGET = QtDemon
MODULE = demon

QT = core gui

DEFINES += QT_BUILD_DEMON_LIB

HEADERS += \
    qtdemonglobal.h \
    qtdemonglobal_p.h \
    qdemonbounds3.h \
    qdemonplane.h \
    qdemonutils.h \
    qdemondataref.h \
    qdemonoption.h \
    qdemoninvasivelinkedlist.h \
    qdemonperftimer.h

SOURCES += \
    qdemonbounds3.cpp \
    qdemonplane.cpp \
    qdemonutils.cpp \
    qdemondataref.cpp \
    qdemonperftimer.cpp

load(qt_module)
