TARGET = uip
QT += demonassetimport-private

PLUGIN_TYPE = assetimporters
PLUGIN_CLASS_NAME = UipAssetImporterPlugin

load(qt_plugin)

OTHER_FILES += uip.json

HEADERS += \
    uipassetimporterplugin.h \
    uipimporter.h \
    abstractxmlparser.h \
    uiaparser.h \
    uipparser.h \
    uippresentation.h \
    enummaps.h \
    datamodelparser.h

SOURCES += \
    uipassetimporterplugin.cpp \
    uipimporter.cpp \
    abstractxmlparser.cpp \
    uiaparser.cpp \
    uipparser.cpp \
    uippresentation.cpp \
    enummaps.cpp \
    datamodelparser.cpp

RESOURCES += \
    metadata.qrc
