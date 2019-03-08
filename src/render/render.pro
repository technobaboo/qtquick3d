TARGET = QtDemonRender
MODULE = demonrender

QT += demon openglextensions

DEFINES += QT_BUILD_DEMONRENDER_LIB

HEADERS += \
    qdemonrenderocclusionquery.h \
    qdemonrendershaderprogram.h \
    qtdemonrenderglobal.h \
    qtdemonrenderglobal_p.h \
    qdemonrenderbasetypes.h \
    qdemonrenderatomiccounterbuffer.h \
    qdemonrenderattriblayout.h \
    qdemonrenderconstantbuffer.h \
    qdemonrendercontext.h \
    qdemonrenderdatabuffer.h \
    qdemonrenderdepthstencilstate.h \
    qdemonrenderdrawindirectbuffer.h \
    qdemonrenderframebuffer.h \
    backends/gl/qdemonrenderbackendgles2.h \
    backends/gl/qdemonopenglextensions.h \
    backends/gl/qdemonopenglprefix.h \
    backends/gl/qdemonopengltokens.h \
    backends/gl/qdemonopenglutil.h \
    backends/gl/qdemonrenderbackendgl3.h \
    backends/gl/qdemonrenderbackendgl4.h \
    backends/gl/qdemonrenderbackendglbase.h \
    backends/gl/qdemonrenderbackendinputassemblergl.h \
    backends/gl/qdemonrenderbackendrenderstatesgl.h \
    backends/gl/qdemonrenderbackendshaderprogramgl.h \
    backends/software/qdemonrenderbackendnull.h \
    backends/qdemonrenderbackend.h \
    glg/qdemonglimplobjects.h \
    qdemonrenderimagetexture.h \
    qdemonrenderindexbuffer.h \
    qdemonrenderinputassembler.h \
    qdemonrenderpathfontspecification.h \
    qdemonrenderpathfonttext.h \
    qdemonrenderpathrender.h \
    qdemonrenderpathspecification.h \
    qdemonrenderprogrampipeline.h \
    qdemonrenderquerybase.h \
    qdemonrenderrasterizerstate.h \
    qdemonrenderrenderbuffer.h \
    qdemonrendersampler.h \
    qdemonrendershaderconstant.h \
    qdemonrenderstoragebuffer.h \
    qdemonrendersync.h \
    qdemonrendertexture2d.h \
    qdemonrendertexture2darray.h \
    qdemonrendertexturebase.h \
    qdemonrendertexturecube.h \
    qdemonrendertimerquery.h \
    qdemonrendervertexbuffer.h \
    qdemonrenderlogging.h

SOURCES += \
    backends/gl/qdemonopenglextensions.cpp \
    backends/gl/qdemonrenderbackendgles2.cpp \
    backends/gl/qdemonrenderbackendgl3.cpp \
    backends/gl/qdemonrenderbackendgl4.cpp \
    backends/gl/qdemonrenderbackendglbase.cpp \
    backends/gl/qdemonrendercontextgl.cpp \
    backends/software/qdemonrenderbackendnull.cpp \
    qdemonrenderatomiccounterbuffer.cpp \
    qdemonrenderattriblayout.cpp \
    qdemonrenderconstantbuffer.cpp \
    qdemonrendercontext.cpp \
    qdemonrenderdatabuffer.cpp \
    qdemonrenderdepthstencilstate.cpp \
    qdemonrenderdrawindirectbuffer.cpp \
    qdemonrenderframebuffer.cpp \
    qdemonrenderimagetexture.cpp \
    qdemonrenderindexbuffer.cpp \
    qdemonrenderinputassembler.cpp \
    qdemonrenderocclusionquery.cpp \
    qdemonrenderpathfontspecification.cpp \
    qdemonrenderpathfonttext.cpp \
    qdemonrenderpathrender.cpp \
    qdemonrenderpathspecification.cpp \
    qdemonrenderprogrampipeline.cpp \
    qdemonrenderquerybase.cpp \
    qdemonrenderrasterizerstate.cpp \
    qdemonrenderrenderbuffer.cpp \
    qdemonrendersampler.cpp \
    qdemonrendershaderprogram.cpp \
    qdemonrenderstoragebuffer.cpp \
    qdemonrendersync.cpp \
    qdemonrendertexture2d.cpp \
    qdemonrendertexture2darray.cpp \
    qdemonrendertexturebase.cpp \
    qdemonrendertexturecube.cpp \
    qdemonrendertimerquery.cpp \
    qdemonrendervertexbuffer.cpp \
    qdemonrenderlogging.cpp \
    qdemonrendershaderconstant.cpp

load(qt_module)
