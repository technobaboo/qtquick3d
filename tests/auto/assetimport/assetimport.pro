QT += testlib
QT += gui demonassetimport core

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_assetimport.cpp
