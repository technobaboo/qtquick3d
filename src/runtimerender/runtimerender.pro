TARGET = QtDemonRuntimeRender
MODULE = demonruntimerender

include(graphobjects/graphobjects.pri)
include(rendererimpl/rendererimpl.pri)
include(resourcemanager/resourcemanager.pri)

HEADERS += \
    q3dsqmlrender.h \
    Qt3DSOffscreenRenderKey.h \
    Qt3DSOffscreenRenderManager.h \
    Qt3DSOldNBustedRenderPlugin.h \
    Qt3DSRenderableImage.h \
    Qt3DSRenderClippingFrustum.h \
    Qt3DSRenderContextCore.h \
    Qt3DSRenderCustomMaterialRenderContext.h \
    Qt3DSRenderCustomMaterialShaderGenerator.h \
    Qt3DSRenderCustomMaterialSystem.h \
    Qt3DSRenderDefaultMaterialShaderGenerator.h \
    Qt3DSRenderDynamicObjectSystemCommands.h \
    Qt3DSRenderDynamicObjectSystem.h \
    Qt3DSRenderDynamicObjectSystemUtil.h \
    Qt3DSRenderEffectSystem.h \
    Qt3DSRenderer.h \
    Qt3DSRendererUtil.h \
    Qt3DSRenderEulerAngles.h \
    Qt3DSRenderGraphObjectPickQuery.h \
    Qt3DSRenderGraphObjectSerializer.h \
    Qt3DSRenderGraphObjectTypes.h \
    Qt3DSRender.h \
    Qt3DSRenderImageScaler.h \
    Qt3DSRenderImageTextureData.h \
    Qt3DSRenderInputStreamFactory.h \
    Qt3DSRenderLightConstantProperties.h \
    Qt3DSRenderMaterialHelpers.h \
    Qt3DSRenderMaterialShaderGenerator.h \
    Qt3DSRenderMesh.h \
    Qt3DSRenderPathManager.h \
    Qt3DSRenderPathMath.h \
    Qt3DSRenderPathRenderContext.h \
    Qt3DSRenderPixelGraphicsRenderer.h \
    Qt3DSRenderPixelGraphicsTypes.h \
    Qt3DSRenderPluginCInterface.h \
    Qt3DSRenderPluginGraphObject.h \
    Qt3DSRenderPlugin.h \
    Qt3DSRenderPluginPropertyValue.h \
    Qt3DSRenderProfiler.h \
    Qt3DSRenderRay.h \
    Qt3DSRenderRenderList.h \
    Qt3DSRenderRotationHelper.h \
    Qt3DSRenderShaderCache.h \
    Qt3DSRenderShaderCodeGenerator.h \
    Qt3DSRenderShaderCodeGeneratorV2.h \
    Qt3DSRenderShaderKeys.h \
    Qt3DSRenderShadowMap.h \
    Qt3DSRenderString.h \
    Qt3DSRenderSubpresentation.h \
    Qt3DSRenderSubPresentationHelper.h \
    Qt3DSRenderTaggedPointer.h \
    Qt3DSRenderTessModeValues.h \
    Qt3DSRenderTextTextureAtlas.h \
    Qt3DSRenderTextTextureCache.h \
    Qt3DSRenderTextTypes.h \
    Qt3DSRenderTextureAtlas.h \
    Qt3DSRenderThreadPool.h \
    Qt3DSRenderUIPLoader.h \
    Qt3DSRenderUIPSharedTranslation.h \
    Qt3DSRenderWidgets.h \
    Qt3DSTextRenderer.h \
    qtdemonruntimerenderglobal.h \
    qtdemonruntimerenderglobal_p.h

SOURCES += \
    q3dsqmlrender.cpp \
    Qt3DSOffscreenRenderManager.cpp \
    Qt3DSOldNBustedRenderPlugin.cpp \
    Qt3DSOnscreenTextRenderer.cpp \
    Qt3DSQtTextRenderer.cpp \
    Qt3DSRenderClippingFrustum.cpp \
    Qt3DSRenderContextCore.cpp \
    Qt3DSRenderCustomMaterialShaderGenerator.cpp \
    Qt3DSRenderCustomMaterialSystem.cpp \
    Qt3DSRenderDefaultMaterialShaderGenerator.cpp \
    Qt3DSRenderDynamicObjectSystem.cpp \
    Qt3DSRenderEffectSystem.cpp \
    Qt3DSRendererUtil.cpp \
    Qt3DSRenderEulerAngles.cpp \
    Qt3DSRenderGpuProfiler.cpp \
    Qt3DSRenderGraphObjectSerializer.cpp \
    Qt3DSRenderImageScaler.cpp \
    Qt3DSRenderInputStreamFactory.cpp \
    Qt3DSRenderPathManager.cpp \
    Qt3DSRenderPixelGraphicsRenderer.cpp \
    Qt3DSRenderPixelGraphicsTypes.cpp \
    Qt3DSRenderPlugin.cpp \
    Qt3DSRenderRay.cpp \
    Qt3DSRenderRenderList.cpp \
    Qt3DSRenderShaderCache.cpp \
    Qt3DSRenderShaderCodeGenerator.cpp \
    Qt3DSRenderShaderCodeGeneratorV2.cpp \
    Qt3DSRenderShadowMap.cpp \
    Qt3DSRenderSubpresentation.cpp \
    Qt3DSRenderTextTextureAtlas.cpp \
    Qt3DSRenderTextTextureCache.cpp \
    Qt3DSRenderTextureAtlas.cpp \
    Qt3DSRenderThreadPool.cpp \
    Qt3DSRenderUIPLoader.cpp \
    Qt3DSRenderUIPSharedTranslation.cpp \
    Qt3DSRenderWidgets.cpp \
    Qt3DSTextRenderer.cpp

load(qt_module)
