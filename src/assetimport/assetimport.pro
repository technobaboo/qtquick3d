TARGET = QtDemonAssetImport
MODULE = demonassetimport

MODULE_PLUGIN_TYPES = assetimporters

QT += core-private gui qml demon demonrender

SOURCES = \
    qdemonassetimporterfactory.cpp \
    qdemonassetimportmanager.cpp \
    qdemonmeshutilities.cpp \
    qdemonscenegraphtranslation.cpp \
    qdemonpathutilities.cpp

HEADERS = \
    qtdemonassetimportglobal.h \
    qdemonassetimporter_p.h \
    qdemonassetimporterfactory_p.h \
    qdemonassetimporterplugin_p.h \
    qdemonassetimportmanager.h \
    qdemonmeshutilities_p.h \
    qdemonscenegraphtranslation_p.h \
    qdemonpathutilities.h

DEFINES += QT_BUILD_DEMONASSETIMPORT_LIB

load(qt_module)
