TARGET = QtDemonRuntimeRender
MODULE = demonruntimerender

QT += demon demonrender demonassetimport-private

include(graphobjects/graphobjects.pri)
include(rendererimpl/rendererimpl.pri)
include(resourcemanager/resourcemanager.pri)

HEADERS += \
    qdemonrendergpuprofiler.h \
    qtdemonruntimerenderglobal.h \
    qtdemonruntimerenderglobal_p.h \
#    qdemonqmlrender.h \
    qdemonoffscreenrenderkey.h \
    qdemonoffscreenrendermanager.h \
#    qdemonoldnbustedrenderplugin.h \
    qdemonrenderableimage.h \
    qdemonrenderclippingfrustum.h \
    qdemonrendercontextcore.h \
    qdemonrendercustommaterialrendercontext.h \
    qdemonrendercustommaterialsystem.h \
    qdemonrenderdefaultmaterialshadergenerator.h \
    qdemonrenderdynamicobjectsystem.h \
    qdemonrenderdynamicobjectsystemcommands.h \
    qdemonrenderdynamicobjectsystemutil.h \
    qdemonrendereffectsystem.h \
    qdemonrenderer.h \
    qdemonrendererutil.h \
    qdemonrendereulerangles.h \
    qdemonrendergraphobjectpickquery.h \
#    qdemonrendergraphobjectserializer.h \
    qdemonrenderimagescaler.h \
    qdemonrenderimagetexturedata.h \
    qdemonrenderinputstreamfactory.h \
    qdemonrenderlightconstantproperties.h \
    qdemonrendermaterialshadergenerator.h \
    qdemonrendermesh.h \
    qdemonrenderpathmanager.h \
    qdemonrenderpathmath.h \
    qdemonrenderpathrendercontext.h \
    qdemonrenderpixelgraphicsrenderer.h \
    qdemonrenderpixelgraphicstypes.h \
#    qdemonrenderplugin.h \
#    qdemonrenderplugincinterface.h \
#    qdemonrenderplugingraphobject.h \
#    qdemonrenderpluginpropertyvalue.h \
    qdemonrenderray.h \
    qdemonrenderrenderlist.h \
    qdemonrendershadercache.h \
    qdemonrendershadercodegenerator.h \
    qdemonrendershadercodegeneratorv2.h \
    qdemonrendershaderkeys.h \
    qdemonrendershadowmap.h \
    qdemonrendersubpresentation.h \
    qdemonrendersubpresentationhelper.h \
    qdemonrendertessmodevalues.h \
    qdemonrenderthreadpool.h \
#    qdemonrenderuipsharedtranslation.h \
    qdemonrenderwidgets.h \
    qdemonruntimerenderlogging.h \
    qdemonperframeallocator.h

SOURCES += \
#    qdemonqmlrender.cpp \
    qdemonoffscreenrendermanager.cpp \
#    qdemonoldnbustedrenderplugin.cpp \
    qdemonrenderclippingfrustum.cpp \
    qdemonrendercontextcore.cpp \
    qdemonrendercustommaterialshadergenerator.cpp \
    qdemonrendercustommaterialsystem.cpp \
    qdemonrenderdefaultmaterialshadergenerator.cpp \
    qdemonrenderdynamicobjectsystem.cpp \
    qdemonrendereffectsystem.cpp \
    qdemonrendererutil.cpp \
    qdemonrendereulerangles.cpp \
    qdemonrendergpuprofiler.cpp \
#    qdemonrendergraphobjectserializer.cpp \
    qdemonrenderimagescaler.cpp \
    qdemonrenderinputstreamfactory.cpp \
    qdemonrendermaterialshadergenerator.cpp \
    qdemonrenderpathmanager.cpp \
    qdemonrenderpixelgraphicsrenderer.cpp \
#    qdemonrenderplugin.cpp \
    qdemonrenderray.cpp \
    qdemonrenderrenderlist.cpp \
    qdemonrendershadercache.cpp \
    qdemonrendershadercodegenerator.cpp \
    qdemonrendershadercodegeneratorv2.cpp \
    qdemonrendershadowmap.cpp \
    qdemonrendersubpresentation.cpp \
    qdemonrenderthreadpool.cpp \
#    qdemonrenderuipsharedtranslation.cpp \
    qdemonrenderwidgets.cpp \
    qdemonruntimerenderlogging.cpp \
    qdemonrenderer.cpp \
    qdemonrendercustommaterialrendercontext.cpp

RESOURCES += res.qrc

load(qt_module)
