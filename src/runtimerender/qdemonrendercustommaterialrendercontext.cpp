#include "qdemonrendercustommaterialrendercontext.h"

#include <QtDemonRender/QDemonRenderTexture2D>

QT_BEGIN_NAMESPACE

QDemonCustomMaterialRenderContext::QDemonCustomMaterialRenderContext(const QDemonRenderLayer &inLayer,
                                                                     const QDemonLayerRenderData &inData,
                                                                     const QVector<QDemonRenderLight *> &inLights,
                                                                     const QDemonRenderCamera &inCamera,
                                                                     const QDemonRenderModel &inModel,
                                                                     const QDemonRenderSubset &inSubset,
                                                                     const QMatrix4x4 &inMvp,
                                                                     const QMatrix4x4 &inWorld,
                                                                     const QMatrix3x3 &inNormal,
                                                                     const QDemonRenderCustomMaterial &inMaterial,
                                                                     const QDemonRef<QDemonRenderTexture2D> &inDepthTex,
                                                                     const QDemonRef<QDemonRenderTexture2D> &inAoTex,
                                                                     QDemonShaderDefaultMaterialKey inMaterialKey,
                                                                     QDemonRenderableImage *inFirstImage,
                                                                     float inOpacity)
    : layer(inLayer)
    , layerData(inData)
    , lights(inLights)
    , camera(inCamera)
    , model(inModel)
    , subset(inSubset)
    , modelViewProjection(inMvp)
    , modelMatrix(inWorld)
    , normalMatrix(inNormal)
    , material(inMaterial)
    , depthTexture(inDepthTex)
    , aoTexture(inAoTex)
    , materialKey(inMaterialKey)
    , firstImage(inFirstImage)
    , opacity(inOpacity)
{
}

QDemonCustomMaterialRenderContext::~QDemonCustomMaterialRenderContext()
{

}


QT_END_NAMESPACE
