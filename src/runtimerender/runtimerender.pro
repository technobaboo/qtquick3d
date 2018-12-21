TARGET = QtDemonRuntimeRender
MODULE = demonruntimerender

QT += demon demonrender

include(graphobjects/graphobjects.pri)
include(rendererimpl/rendererimpl.pri)
include(resourcemanager/resourcemanager.pri)

HEADERS += \
    qtdemonruntimerenderglobal.h \
    qtdemonruntimerenderglobal_p.h \
#    qdemonqmlrender.h \
    qdemonoffscreenrenderkey.h \
#    qdemonoffscreenrendermanager.h \
#    qdemonoldnbustedrenderplugin.h \
#    qdemonrenderableimage.h \
    qdemonrenderclippingfrustum.h \
#    qdemonrendercontextcore.h \
#    qdemonrendercustommaterialrendercontext.h \
#    qdemonrendercustommaterialshadergenerator.h \
#    qdemonrendercustommaterialsystem.h \
#    qdemonrenderdefaultmaterialshadergenerator.h \
#    qdemonrenderdynamicobjectsystem.h \
    qdemonrenderdynamicobjectsystemcommands.h \
    qdemonrenderdynamicobjectsystemutil.h \
#    qdemonrendereffectsystem.h \
    qdemonrenderer.h \
#    qdemonrendererutil.h \
    qdemonrendereulerangles.h \
    qdemonrendergraphobjectpickquery.h \
#    qdemonrendergraphobjectserializer.h \
    qdemonrendergraphobjecttypes.h \
    qdemonrenderimagescaler.h \
    qdemonrenderimagetexturedata.h \
    qdemonrenderinputstreamfactory.h \
    qdemonrenderlightconstantproperties.h \
#    qdemonrendermaterialhelpers.h \
#    qdemonrendermaterialshadergenerator.h \
    qdemonrendermesh.h \
#    qdemonrenderpathmanager.h \
#    qdemonrenderpathmath.h \
#    qdemonrenderpathrendercontext.h \
#    qdemonrenderpixelgraphicsrenderer.h \
    qdemonrenderpixelgraphicstypes.h \
#    qdemonrenderplugin.h \
    qdemonrenderplugincinterface.h \
    qdemonrenderplugingraphobject.h \
#    qdemonrenderpluginpropertyvalue.h \
    qdemonrenderprofiler.h \
    qdemonrenderray.h \
    qdemonrenderrenderlist.h \
    qdemonrenderrotationhelper.h \
    qdemonrendershadercache.h \
    qdemonrendershadercodegenerator.h \
#    qdemonrendershadercodegeneratorv2.h \
    qdemonrendershaderkeys.h \
#    qdemonrendershadowmap.h \
#    qdemonrendersubpresentation.h \
#    qdemonrendersubpresentationhelper.h \
    qdemonrendertaggedpointer.h \
    qdemonrendertessmodevalues.h \
    qdemonrendertexttextureatlas.h \
    qdemonrendertexttexturecache.h \
    qdemonrendertexttypes.h \
    qdemonrendertextureatlas.h \
    qdemonrenderthreadpool.h \
#    qdemonrenderuipsharedtranslation.h \
#    qdemonrenderwidgets.h \
    qdemontextrenderer.h \
    qdemonruntimerenderlogging.h

SOURCES += \
#    qdemonqmlrender.cpp \
#    qdemonoffscreenrendermanager.cpp \
#    qdemonoldnbustedrenderplugin.cpp \
    qdemononscreentextrenderer.cpp \
    qdemonqttextrenderer.cpp \
    qdemonrenderclippingfrustum.cpp \
#    qdemonrendercontextcore.cpp \
#    qdemonrendercustommaterialshadergenerator.cpp \
#    qdemonrendercustommaterialsystem.cpp \
#    qdemonrenderdefaultmaterialshadergenerator.cpp \
#    qdemonrenderdynamicobjectsystem.cpp \
#    qdemonrendereffectsystem.cpp \
#    qdemonrendererutil.cpp \
    qdemonrendereulerangles.cpp \
#    qdemonrendergpuprofiler.cpp \
#    qdemonrendergraphobjectserializer.cpp \
    qdemonrenderimagescaler.cpp \
    qdemonrenderinputstreamfactory.cpp \
#    qdemonrenderpathmanager.cpp \
#    qdemonrenderpixelgraphicsrenderer.cpp \
    qdemonrenderpixelgraphicstypes.cpp \
#    qdemonrenderplugin.cpp \
    qdemonrenderray.cpp \
    qdemonrenderrenderlist.cpp \
    qdemonrendershadercache.cpp \
    qdemonrendershadercodegenerator.cpp \
#    qdemonrendershadercodegeneratorv2.cpp \
#    qdemonrendershadowmap.cpp \
#    qdemonrendersubpresentation.cpp \
    qdemonrendertexttextureatlas.cpp \
    qdemonrendertexttexturecache.cpp \
    qdemonrendertextureatlas.cpp \
    qdemonrenderthreadpool.cpp \
#    qdemonrenderuipsharedtranslation.cpp \
#    qdemonrenderwidgets.cpp \
    qdemontextrenderer.cpp \
    qdemonruntimerenderlogging.cpp \
    qdemonrenderer.cpp

load(qt_module)
