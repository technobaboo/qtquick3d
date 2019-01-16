TARGET = obj
QT += demonassetimport-private

PLUGIN_TYPE = assetimporters
PLUGIN_CLASS_NAME = ObjAssetImporterPlugin

load(qt_plugin)

HEADERS += \
    objassetimporterplugin.h \
    objassetimporter.h
SOURCES += \
    objassetimporterplugin.cpp \
    objassetimporter.cpp

OTHER_FILES += obj.json
