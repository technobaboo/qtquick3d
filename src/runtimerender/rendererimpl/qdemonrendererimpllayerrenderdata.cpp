/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendererimpl.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrenderkey.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugingraphobject.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <QtDemon/qdemonperftimer.h>
#include <QtDemonRuntimeRender/qdemonrendermaterialhelpers.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrendertexttextureatlas.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrendererutil.h>

#define QDEMON_CACHED_POST_EFFECT
const float QDEMON_DEGREES_TO_RADIANS = 0.0174532925199f;
const float QDEMON_PI = 3.1415926535897f;
const float QDEMON_HALFPI = 1.57079632679489661923f;

QT_BEGIN_NAMESPACE

QDemonLayerRenderData::QDemonLayerRenderData(QDemonRenderLayer &inLayer, const QDemonRef<QDemonRendererImpl> &inRenderer)
    : QDemonLayerRenderPreparationData(inLayer, inRenderer)
    , m_layerTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_temporalAATexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerDepthTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerPrepassDepthTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerWidgetTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerSsaoTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerMultisampleTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerMultisamplePrepassDepthTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerMultisampleWidgetTexture(inRenderer->getDemonContext()->getResourceManager())
    , m_layerCachedTexture(nullptr)
    , m_advancedBlendDrawTexture(nullptr)
    , m_advancedBlendBlendTexture(nullptr)
    , m_advancedModeDrawFB(nullptr)
    , m_advancedModeBlendFB(nullptr)
    , m_progressiveAAPassIndex(0)
    , m_temporalAAPassIndex(0)
    , m_nonDirtyTemporalAAPassIndex(0)
    , m_textScale(1.0f)
    , m_depthBufferFormat(QDemonRenderTextureFormat::Unknown)
{
}

QDemonLayerRenderData::~QDemonLayerRenderData()
{
    QDemonRef<QDemonResourceManagerInterface> theResourceManager(renderer->getDemonContext()->getResourceManager());
    if (m_layerCachedTexture && m_layerCachedTexture != m_layerTexture.getTexture())
        theResourceManager->release(m_layerCachedTexture);
    if (m_advancedModeDrawFB) {
        m_advancedModeDrawFB = nullptr;
    }
    if (m_advancedModeBlendFB) {
        m_advancedModeBlendFB = nullptr;
    }
    if (m_advancedBlendBlendTexture)
        m_advancedBlendBlendTexture = nullptr;
    if (m_advancedBlendDrawTexture)
        m_advancedBlendDrawTexture = nullptr;
}
void QDemonLayerRenderData::prepareForRender(const QSize &inViewportDimensions)
{
    QDemonLayerRenderPreparationData::prepareForRender(inViewportDimensions);
    QDemonLayerRenderPreparationResult &thePrepResult(*layerPrepResult);
    QDemonRef<QDemonResourceManagerInterface> theResourceManager(renderer->getDemonContext()->getResourceManager());
    // at that time all values shoud be updated
    renderer->updateCbAoShadow(&layer, camera, m_layerDepthTexture);

    // Generate all necessary lighting keys

    if (thePrepResult.flags.wasLayerDataDirty()) {
        m_progressiveAAPassIndex = 0;
    }

    // Get rid of the layer texture if we aren't rendering to texture this frame.
    if (m_layerTexture.getTexture() && !thePrepResult.flags.shouldRenderToTexture()) {
        if (m_layerCachedTexture && m_layerCachedTexture != m_layerTexture.getTexture()) {
            theResourceManager->release(m_layerCachedTexture);
            m_layerCachedTexture = nullptr;
        }

        m_layerTexture.releaseTexture();
        m_layerDepthTexture.releaseTexture();
        m_layerWidgetTexture.releaseTexture();
        m_layerSsaoTexture.releaseTexture();
        m_layerMultisampleTexture.releaseTexture();
        m_layerMultisamplePrepassDepthTexture.releaseTexture();
        m_layerMultisampleWidgetTexture.releaseTexture();
    }

    if (needsWidgetTexture() == false)
        m_layerWidgetTexture.releaseTexture();

    if (m_layerDepthTexture.getTexture() && !thePrepResult.flags.requiresDepthTexture())
        m_layerDepthTexture.releaseTexture();

    if (m_layerSsaoTexture.getTexture() && !thePrepResult.flags.requiresSsaoPass())
        m_layerSsaoTexture.releaseTexture();

    renderer->layerNeedsFrameClear(*this);

    // Clean up the texture cache if layer dimensions changed
    if (inViewportDimensions.width() != m_previousDimensions.width()
        || inViewportDimensions.height() != m_previousDimensions.height()) {
        m_layerTexture.releaseTexture();
        m_layerDepthTexture.releaseTexture();
        m_layerSsaoTexture.releaseTexture();
        m_layerWidgetTexture.releaseTexture();
        m_layerPrepassDepthTexture.releaseTexture();
        m_temporalAATexture.releaseTexture();
        m_layerMultisampleTexture.releaseTexture();
        m_layerMultisamplePrepassDepthTexture.releaseTexture();
        m_layerMultisampleWidgetTexture.releaseTexture();

        m_previousDimensions.setWidth(inViewportDimensions.width());
        m_previousDimensions.setHeight(inViewportDimensions.height());

        theResourceManager->destroyFreeSizedResources();

        // Effect system uses different resource manager, so clean that up too
        renderer->getDemonContext()->getEffectSystem()->getResourceManager()->destroyFreeSizedResources();
    }
}

QDemonRenderTextureFormat QDemonLayerRenderData::getDepthBufferFormat()
{
    if (m_depthBufferFormat == QDemonRenderTextureFormat::Unknown) {
        quint32 theExistingDepthBits = renderer->getContext()->getDepthBits();
        quint32 theExistingStencilBits = renderer->getContext()->getStencilBits();
        switch (theExistingDepthBits) {
        case 32:
            m_depthBufferFormat = QDemonRenderTextureFormat::Depth32;
            break;
        case 24:
            //  check if we have stencil bits
            if (theExistingStencilBits > 0)
                m_depthBufferFormat = QDemonRenderTextureFormat::Depth24Stencil8; // currently no stencil usage
            // should be Depth24Stencil8 in
            // this case
            else
                m_depthBufferFormat = QDemonRenderTextureFormat::Depth24;
            break;
        case 16:
            m_depthBufferFormat = QDemonRenderTextureFormat::Depth16;
            break;
        default:
            Q_ASSERT(false);
            m_depthBufferFormat = QDemonRenderTextureFormat::Depth16;
            break;
        }
    }
    return m_depthBufferFormat;
}

QDemonRenderFrameBufferAttachment QDemonLayerRenderData::getFramebufferDepthAttachmentFormat(QDemonRenderTextureFormat depthFormat)
{
    QDemonRenderFrameBufferAttachment fmt = QDemonRenderFrameBufferAttachment::Depth;

    switch (depthFormat.format) {
    case QDemonRenderTextureFormat::Depth16:
    case QDemonRenderTextureFormat::Depth24:
    case QDemonRenderTextureFormat::Depth32:
        fmt = QDemonRenderFrameBufferAttachment::Depth;
        break;
    case QDemonRenderTextureFormat::Depth24Stencil8:
        fmt = QDemonRenderFrameBufferAttachment::DepthStencil;
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    return fmt;
}

void QDemonLayerRenderData::renderAoPass()
{
    renderer->beginLayerDepthPassRender(*this);

    auto theContext = renderer->getContext();
    QDemonRef<QDemonDefaultAoPassShader> shader = renderer->getDefaultAoPassShader(getShaderFeatureSet());
    if (shader == nullptr)
        return;

    // Set initial state
    theContext->setBlendingEnabled(false);
    theContext->setDepthWriteEnabled(false);
    theContext->setDepthTestEnabled(false);
    theContext->setActiveShader(shader->shader);

    // Setup constants
    shader->cameraDirection.set(cameraDirection);
    shader->viewMatrix.set(camera->globalTransform);

    shader->depthTexture.set(m_layerDepthTexture.getTexture().data());
    shader->depthSamplerSize.set(
            QVector2D(m_layerDepthTexture->getTextureDetails().width, m_layerDepthTexture->getTextureDetails().height));

    // Important uniforms for AO calculations
    QVector2D theCameraProps = QVector2D(camera->clipNear, camera->clipFar);
    shader->cameraProperties.set(theCameraProps);
    shader->aoShadowParams.set();

    // Draw a fullscreen quad
    renderer->renderQuad();

    renderer->endLayerDepthPassRender();
}

void QDemonLayerRenderData::renderFakeDepthMapPass(QDemonRenderTexture2D *theDepthTex, QDemonRenderTextureCube *theDepthCube)
{
    renderer->beginLayerDepthPassRender(*this);

    auto theContext = renderer->getContext();
    QDemonRef<QDemonDefaultAoPassShader> shader = theDepthTex ? renderer->getFakeDepthShader(getShaderFeatureSet())
                                                              : renderer->getFakeCubeDepthShader(getShaderFeatureSet());
    if (shader == nullptr)
        return;

    // Set initial state
    theContext->setBlendingEnabled(false);
    theContext->setDepthWriteEnabled(false);
    theContext->setDepthTestEnabled(false);
    theContext->setActiveShader(shader->shader);

    // Setup constants
    shader->cameraDirection.set(cameraDirection);
    shader->viewMatrix.set(camera->globalTransform);

    shader->depthTexture.set(theDepthTex);
    shader->cubeTexture.set(theDepthCube);
    shader->depthSamplerSize.set(QVector2D(theDepthTex->getTextureDetails().width, theDepthTex->getTextureDetails().height));

    // Important uniforms for AO calculations
    QVector2D theCameraProps = QVector2D(camera->clipNear, camera->clipFar);
    shader->cameraProperties.set(theCameraProps);
    shader->aoShadowParams.set();

    // Draw a fullscreen quad
    renderer->renderQuad();
}

namespace {

void computeFrustumBounds(const QDemonRenderCamera &inCamera, const QRectF &inViewPort, QVector3D &ctrBound, QVector3D camVerts[8])
{
    QVector3D camEdges[4];

    const float *dataPtr(inCamera.globalTransform.constData());
    QVector3D camX(dataPtr[0], dataPtr[1], dataPtr[2]);
    QVector3D camY(dataPtr[4], dataPtr[5], dataPtr[6]);
    QVector3D camZ(dataPtr[8], dataPtr[9], dataPtr[10]);

    float tanFOV = tanf(inCamera.verticalFov(inViewPort) * 0.5f);
    float asTanFOV = tanFOV * inViewPort.width() / inViewPort.height();
    camEdges[0] = -asTanFOV * camX + tanFOV * camY + camZ;
    camEdges[1] = asTanFOV * camX + tanFOV * camY + camZ;
    camEdges[2] = asTanFOV * camX - tanFOV * camY + camZ;
    camEdges[3] = -asTanFOV * camX - tanFOV * camY + camZ;

    for (int i = 0; i < 4; ++i) {
        camEdges[i].setX(-camEdges[i].x());
        camEdges[i].setY(-camEdges[i].y());
    }

    camVerts[0] = inCamera.position + camEdges[0] * inCamera.clipNear;
    camVerts[1] = inCamera.position + camEdges[0] * inCamera.clipFar;
    camVerts[2] = inCamera.position + camEdges[1] * inCamera.clipNear;
    camVerts[3] = inCamera.position + camEdges[1] * inCamera.clipFar;
    camVerts[4] = inCamera.position + camEdges[2] * inCamera.clipNear;
    camVerts[5] = inCamera.position + camEdges[2] * inCamera.clipFar;
    camVerts[6] = inCamera.position + camEdges[3] * inCamera.clipNear;
    camVerts[7] = inCamera.position + camEdges[3] * inCamera.clipFar;

    ctrBound = camVerts[0];
    for (int i = 1; i < 8; ++i) {
        ctrBound += camVerts[i];
    }
    ctrBound *= 0.125f;
}

void setupCameraForShadowMap(const QVector2D &inCameraVec,
                             QDemonRenderContext & /*inContext*/,
                             const QRectF &inViewport,
                             const QDemonRenderCamera &inCamera,
                             const QDemonRenderLight *inLight,
                             QDemonRenderCamera &theCamera)
{
    // setup light matrix
    quint32 mapRes = 1 << inLight->m_shadowMapRes;
    QRectF theViewport(0.0f, 0.0f, (float)mapRes, (float)mapRes);
    theCamera.clipNear = 1.0f;
    theCamera.clipFar = inLight->m_shadowMapFar;
    // Setup camera projection
    QVector3D inLightPos = inLight->getGlobalPos();
    QVector3D inLightDir = inLight->getDirection();

    if (inLight->flags.isLeftHanded())
        inLightPos.setZ(-inLightPos.z());

    inLightPos -= inLightDir * inCamera.clipNear;
    theCamera.fov = inLight->m_shadowMapFov * QDEMON_DEGREES_TO_RADIANS;

    if (inLight->m_lightType == RenderLightTypes::Directional) {
        QVector3D frustBounds[8], boundCtr;
        computeFrustumBounds(inCamera, inViewport, boundCtr, frustBounds);

        QVector3D forward = inLightDir;
        forward.normalize();
        QVector3D right = QVector3D::crossProduct(forward, QVector3D(0, 1, 0));
        right.normalize();
        QVector3D up = QVector3D::crossProduct(right, forward);
        up.normalize();

        // Calculate bounding box of the scene camera frustum
        float minDistanceZ = std::numeric_limits<float>::max();
        float maxDistanceZ = -std::numeric_limits<float>::max();
        float minDistanceY = std::numeric_limits<float>::max();
        float maxDistanceY = -std::numeric_limits<float>::max();
        float minDistanceX = std::numeric_limits<float>::max();
        float maxDistanceX = -std::numeric_limits<float>::max();
        for (int i = 0; i < 8; ++i) {
            float distanceZ = QVector3D::dotProduct(frustBounds[i], forward);
            if (distanceZ < minDistanceZ)
                minDistanceZ = distanceZ;
            if (distanceZ > maxDistanceZ)
                maxDistanceZ = distanceZ;
            float distanceY = QVector3D::dotProduct(frustBounds[i], up);
            if (distanceY < minDistanceY)
                minDistanceY = distanceY;
            if (distanceY > maxDistanceY)
                maxDistanceY = distanceY;
            float distanceX = QVector3D::dotProduct(frustBounds[i], right);
            if (distanceX < minDistanceX)
                minDistanceX = distanceX;
            if (distanceX > maxDistanceX)
                maxDistanceX = distanceX;
        }

        // Apply bounding box parameters to shadow map camera projection matrix
        // so that the whole scene is fit inside the shadow map
        inLightPos = boundCtr;
        theViewport.setHeight(abs(maxDistanceY - minDistanceY));
        theViewport.setWidth(abs(maxDistanceX - minDistanceX));
        theCamera.clipNear = -abs(maxDistanceZ - minDistanceZ);
        theCamera.clipFar = abs(maxDistanceZ - minDistanceZ);
    }

    theCamera.flags.setLeftHanded(false);

    theCamera.flags.setFlag(QDemonNodeFlagValues::Orthographic, inLight->m_lightType == RenderLightTypes::Directional);
    theCamera.parent = nullptr;
    theCamera.pivot = inLight->pivot;

    if (inLight->m_lightType != RenderLightTypes::Point) {
        theCamera.lookAt(inLightPos, QVector3D(0, 1.0, 0), inLightPos + inLightDir);
    } else {
        theCamera.lookAt(inLightPos, QVector3D(0, 1.0, 0), QVector3D(0, 0, 0));
    }

    theCamera.calculateGlobalVariables(theViewport, QVector2D(theViewport.width(), theViewport.height()));
}
}

void setupCubeShadowCameras(const QDemonRenderLight *inLight, QDemonRenderCamera inCameras[6])
{
    // setup light matrix
    quint32 mapRes = 1 << inLight->m_shadowMapRes;
    QRectF theViewport(0.0f, 0.0f, (float)mapRes, (float)mapRes);
    QVector3D rotOfs[6];

    Q_ASSERT(inLight != nullptr);
    Q_ASSERT(inLight->m_lightType != RenderLightTypes::Directional);

    QVector3D inLightPos = inLight->getGlobalPos();
    if (inLight->flags.isLeftHanded())
        inLightPos.setZ(-inLightPos.z());

    rotOfs[0] = QVector3D(0.f, -QDEMON_HALFPI, QDEMON_PI);
    rotOfs[1] = QVector3D(0.f, QDEMON_HALFPI, QDEMON_PI);
    rotOfs[2] = QVector3D(QDEMON_HALFPI, 0.f, 0.f);
    rotOfs[3] = QVector3D(-QDEMON_HALFPI, 0.f, 0.f);
    rotOfs[4] = QVector3D(0.f, QDEMON_PI, -QDEMON_PI);
    rotOfs[5] = QVector3D(0.f, 0.f, QDEMON_PI);

    for (int i = 0; i < 6; ++i) {
        inCameras[i].flags.setLeftHanded(false);

        inCameras[i].flags.setFlag(QDemonNodeFlagValues::Orthographic, false);
        inCameras[i].parent = nullptr;
        inCameras[i].pivot = inLight->pivot;
        inCameras[i].clipNear = 1.0f;
        inCameras[i].clipFar = qMax<float>(2.0f, inLight->m_shadowMapFar);
        inCameras[i].fov = inLight->m_shadowMapFov * QDEMON_DEGREES_TO_RADIANS;

        inCameras[i].position = inLightPos;
        inCameras[i].rotation = rotOfs[i];
        inCameras[i].calculateGlobalVariables(theViewport, QVector2D(theViewport.width(), theViewport.height()));
    }

    /*
        if ( inLight->m_LightType == RenderLightTypes::Point ) return;

        QVector3D viewDirs[6];
        QVector3D viewUp[6];
        QMatrix3x3 theDirMatrix( inLight->m_GlobalTransform.getUpper3x3() );

        viewDirs[0] = theDirMatrix.transform( QVector3D( 1.f, 0.f, 0.f ) );
        viewDirs[2] = theDirMatrix.transform( QVector3D( 0.f, -1.f, 0.f ) );
        viewDirs[4] = theDirMatrix.transform( QVector3D( 0.f, 0.f, 1.f ) );
        viewDirs[0].normalize();  viewDirs[2].normalize();  viewDirs[4].normalize();
        viewDirs[1] = -viewDirs[0];
        viewDirs[3] = -viewDirs[2];
        viewDirs[5] = -viewDirs[4];

        viewUp[0] = viewDirs[2];
        viewUp[1] = viewDirs[2];
        viewUp[2] = viewDirs[5];
        viewUp[3] = viewDirs[4];
        viewUp[4] = viewDirs[2];
        viewUp[5] = viewDirs[2];

        for (int i = 0; i < 6; ++i)
        {
                inCameras[i].LookAt( inLightPos, viewUp[i], inLightPos + viewDirs[i] );
                inCameras[i].CalculateGlobalVariables( theViewport, QVector2D( theViewport.m_Width,
        theViewport.m_Height ) );
        }
        */
}

inline void renderRenderableShadowMapPass(QDemonLayerRenderData &inData,
                                          QDemonRenderableObject &inObject,
                                          const QVector2D &inCameraProps,
                                          const TShaderFeatureSet &,
                                          quint32 lightIndex,
                                          const QDemonRenderCamera &inCamera)
{
    QDemonShadowMapEntry *pEntry = inData.shadowMapManager->getShadowMapEntry(lightIndex);

    if (inObject.renderableFlags.isDefaultMaterialMeshSubset())
        static_cast<QDemonSubsetRenderableBase &>(inObject).renderShadowMapPass(inCameraProps, inData.lights[lightIndex], inCamera, pEntry);
    else if (inObject.renderableFlags.isCustomMaterialMeshSubset()) {
        static_cast<QDemonSubsetRenderableBase &>(inObject).renderShadowMapPass(inCameraProps, inData.lights[lightIndex], inCamera, pEntry);
    } else if (inObject.renderableFlags.isPath()) {
        static_cast<QDemonPathRenderable &>(inObject).renderShadowMapPass(inCameraProps, inData.lights[lightIndex], inCamera, pEntry);
    }
}

void QDemonLayerRenderData::renderShadowCubeBlurPass(QDemonResourceFrameBuffer *theFB,
                                                     const QDemonRef<QDemonRenderTextureCube> &target0,
                                                     const QDemonRef<QDemonRenderTextureCube> &target1,
                                                     float filterSz,
                                                     float clipFar)
{
    auto theContext = renderer->getContext();

    QDemonRef<QDemonShadowmapPreblurShader> shaderX = renderer->getCubeShadowBlurXShader();
    QDemonRef<QDemonShadowmapPreblurShader> shaderY = renderer->getCubeShadowBlurYShader();

    if (shaderX == nullptr)
        return;
    if (shaderY == nullptr)
        return;
    // if ( theShader == nullptr ) return;

    // Enable drawing to 6 color attachment buffers for cubemap passes
    qint32 buffers[6] = { 0, 1, 2, 3, 4, 5 };
    QDemonConstDataRef<qint32> bufferList(buffers, 6);
    theContext->setDrawBuffers(bufferList);

    // Attach framebuffer targets
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color0, target1, QDemonRenderTextureCubeFace::CubePosX);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color1, target1, QDemonRenderTextureCubeFace::CubeNegX);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color2, target1, QDemonRenderTextureCubeFace::CubePosY);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color3, target1, QDemonRenderTextureCubeFace::CubeNegY);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color4, target1, QDemonRenderTextureCubeFace::CubePosZ);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color5, target1, QDemonRenderTextureCubeFace::CubeNegZ);

    // Set initial state
    theContext->setBlendingEnabled(false);
    theContext->setDepthWriteEnabled(false);
    theContext->setDepthTestEnabled(false);
    // theContext.SetColorWritesEnabled(true);
    theContext->setActiveShader(shaderX->shader);

    shaderX->cameraProperties.set(QVector2D(filterSz, clipFar));
    shaderX->depthCube.set(target0.data());

    // Draw a fullscreen quad
    renderer->renderQuad();

    theContext->setActiveShader(shaderY->shader);

    // Lather, Rinse, and Repeat for the Y-blur pass
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color0, target0, QDemonRenderTextureCubeFace::CubePosX);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color1, target0, QDemonRenderTextureCubeFace::CubeNegX);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color2, target0, QDemonRenderTextureCubeFace::CubePosY);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color3, target0, QDemonRenderTextureCubeFace::CubeNegY);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color4, target0, QDemonRenderTextureCubeFace::CubePosZ);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color5, target0, QDemonRenderTextureCubeFace::CubeNegZ);

    shaderY->cameraProperties.set(QVector2D(filterSz, clipFar));
    shaderY->depthCube.set(target1.data());

    // Draw a fullscreen quad
    renderer->renderQuad();

    theContext->setDepthWriteEnabled(true);
    theContext->setDepthTestEnabled(true);
    // theContext.SetColorWritesEnabled(false);

    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color0,
                         QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFace::CubePosX);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color1,
                         QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFace::CubeNegX);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color2,
                         QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFace::CubePosY);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color3,
                         QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFace::CubeNegY);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color4,
                         QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFace::CubePosZ);
    (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color5,
                         QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFace::CubeNegZ);

    theContext->setDrawBuffers(toConstDataRef((qint32)0));
}

void QDemonLayerRenderData::renderShadowMapBlurPass(QDemonResourceFrameBuffer *theFB,
                                                    const QDemonRef<QDemonRenderTexture2D> &target0,
                                                    const QDemonRef<QDemonRenderTexture2D> &target1,
                                                    float filterSz,
                                                    float clipFar)
{
    auto theContext = renderer->getContext();

    QDemonRef<QDemonShadowmapPreblurShader> shaderX = renderer->getOrthoShadowBlurXShader();
    QDemonRef<QDemonShadowmapPreblurShader> shaderY = renderer->getOrthoShadowBlurYShader();

    if (shaderX == nullptr)
        return;
    if (shaderY == nullptr)
        return;

    // Attach framebuffer target
    (*theFB)->attach(QDemonRenderFrameBufferAttachment::Color0, target1);
    //(*theFB)->Attach( QDemonRenderFrameBufferAttachments::DepthStencil, *target1 );

    // Set initial state
    theContext->setBlendingEnabled(false);
    theContext->setDepthWriteEnabled(false);
    theContext->setDepthTestEnabled(false);
    theContext->setColorWritesEnabled(true);
    theContext->setActiveShader(shaderX->shader);

    shaderX->cameraProperties.set(QVector2D(filterSz, clipFar));
    shaderX->depthMap.set(target0.data());

    // Draw a fullscreen quad
    renderer->renderQuad();

    (*theFB)->attach(QDemonRenderFrameBufferAttachment::Color0, target0);
    //(*theFB)->Attach( QDemonRenderFrameBufferAttachments::DepthStencil, *target0 );
    theContext->setActiveShader(shaderY->shader);

    shaderY->cameraProperties.set(QVector2D(filterSz, clipFar));
    shaderY->depthMap.set(target1.data());

    // Draw a fullscreen quad
    renderer->renderQuad();

    theContext->setDepthWriteEnabled(true);
    theContext->setDepthTestEnabled(true);
    theContext->setColorWritesEnabled(false);

    //(*theFB)->Attach( QDemonRenderFrameBufferAttachments::DepthStencil,
    // QDemonRenderTextureOrRenderBuffer() );
    (*theFB)->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
}

void QDemonLayerRenderData::renderShadowMapPass(QDemonResourceFrameBuffer *theFB)
{
    QDemonStackPerfTimer ___timer(renderer->getDemonContext()->getPerfTimer(), Q_FUNC_INFO);

    if (camera == nullptr || !getShadowMapManager())
        return;

    // Check if we have anything to render
    if (opaqueObjects.size() == 0 || lights.size() == 0)
        return;

    renderer->beginLayerDepthPassRender(*this);

    auto theRenderContext = renderer->getContext();

    // we may change the viewport
    QDemonRenderContextScopedProperty<QRect> __viewport(*theRenderContext, &QDemonRenderContext::getViewport, &QDemonRenderContext::setViewport);

    // disable color writes
    // theRenderContext.SetColorWritesEnabled( false );
    theRenderContext->setColorWritesEnabled(true);
    theRenderContext->setDepthWriteEnabled(true);
    theRenderContext->setCullingEnabled(false);
    theRenderContext->setClearColor(QVector4D(1.0, 1.0, 1.0, 1.0));

    // we render the shadow map with a slight offset to prevent shadow acne and cull the front
    // faces
    QDemonRef<QDemonRenderRasterizerState> rsdefaultstate = theRenderContext->createRasterizerState(0.0, 0.0, QDemonRenderFace::Back);
    QDemonRef<QDemonRenderRasterizerState> rsstate = theRenderContext->createRasterizerState(1.5, 2.0, QDemonRenderFace::Front);
    theRenderContext->setRasterizerState(rsstate);

    QDemonRenderClearFlags clearFlags(QDemonRenderClearValues::Depth | QDemonRenderClearValues::Stencil
                                      | QDemonRenderClearValues::Color);

    for (int i = 0; i < lights.size(); i++) {
        // don't render shadows when not casting
        if (lights[i]->m_castShadow == false)
            continue;
        QDemonShadowMapEntry *pEntry = shadowMapManager->getShadowMapEntry(i);
        if (pEntry && pEntry->m_depthMap && pEntry->m_depthCopy && pEntry->m_depthRender) {
            QDemonRenderCamera theCamera;

            QVector2D theCameraProps = QVector2D(camera->clipNear, camera->clipFar);
            setupCameraForShadowMap(theCameraProps, *renderer->getContext(), __viewport.m_initialValue, *camera, lights[i], theCamera);
            // we need this matrix for the final rendering
            theCamera.calculateViewProjectionMatrix(pEntry->m_lightVP);
            pEntry->m_lightView = mat44::getInverse(theCamera.globalTransform);

            QDemonTextureDetails theDetails(pEntry->m_depthMap->getTextureDetails());
            theRenderContext->setViewport(QRect(0, 0, (quint32)theDetails.width, (quint32)theDetails.height));

            (*theFB)->attach(QDemonRenderFrameBufferAttachment::Color0, pEntry->m_depthMap);
            (*theFB)->attach(QDemonRenderFrameBufferAttachment::DepthStencil, pEntry->m_depthRender);
            theRenderContext->clear(clearFlags);

            runRenderPass(renderRenderableShadowMapPass, false, true, true, i, theCamera);
            renderShadowMapBlurPass(theFB, pEntry->m_depthMap, pEntry->m_depthCopy, lights[i]->m_shadowFilter, lights[i]->m_shadowMapFar);
        } else if (pEntry && pEntry->m_depthCube && pEntry->m_cubeCopy && pEntry->m_depthRender) {
            QDemonRenderCamera theCameras[6];

            setupCubeShadowCameras(lights[i], theCameras);

            // pEntry->m_LightView = m_Lights[i]->m_LightType == RenderLightTypes::Point ?
            // QMatrix4x4::createIdentity()
            //	: m_Lights[i]->m_GlobalTransform;
            pEntry->m_lightView = QMatrix4x4();

            QDemonTextureDetails theDetails(pEntry->m_depthCube->getTextureDetails());
            theRenderContext->setViewport(QRect(0, 0, (quint32)theDetails.width, (quint32)theDetails.height));

            // int passes = m_Lights[i]->m_LightType == RenderLightTypes::Point ? 6 : 5;
            int passes = 6;
            for (int k = 0; k < passes; ++k) {
                // theCameras[k].CalculateViewProjectionMatrix( pEntry->m_LightCubeVP[k] );
                pEntry->m_lightCubeView[k] = mat44::getInverse(theCameras[k].globalTransform);
                theCameras[k].calculateViewProjectionMatrix(pEntry->m_lightVP);

                // Geometry shader multiplication really doesn't work unless you have a
                // 6-layered 3D depth texture...
                // Otherwise, you have no way to depth test while rendering...
                // which more or less completely defeats the purpose of having a cubemap render
                // target.
                QDemonRenderTextureCubeFace curFace = (QDemonRenderTextureCubeFace)(k + 1);
                //(*theFB)->AttachFace( QDemonRenderFrameBufferAttachments::DepthStencil,
                //*pEntry->m_DepthCube, curFace );
                (*theFB)->attach(QDemonRenderFrameBufferAttachment::DepthStencil, pEntry->m_depthRender);
                (*theFB)->attachFace(QDemonRenderFrameBufferAttachment::Color0, pEntry->m_depthCube, curFace);
                (*theFB)->isComplete();
                theRenderContext->clear(clearFlags);

                runRenderPass(renderRenderableShadowMapPass, false, true, true, i, theCameras[k]);
            }

            renderShadowCubeBlurPass(theFB,
                                     pEntry->m_depthCube,
                                     pEntry->m_cubeCopy,
                                     lights[i]->m_shadowFilter,
                                     lights[i]->m_shadowMapFar);
        }
    }

    (*theFB)->attach(QDemonRenderFrameBufferAttachment::Depth, QDemonRenderTextureOrRenderBuffer());
    (*theFB)->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());

    // enable color writes
    theRenderContext->setColorWritesEnabled(true);
    theRenderContext->setCullingEnabled(true);
    theRenderContext->setClearColor(QVector4D(0.0, 0.0, 0.0, 0.0));
    // reset rasterizer state
    theRenderContext->setRasterizerState(rsdefaultstate);

    renderer->endLayerDepthPassRender();
}

inline void renderRenderableDepthPass(QDemonLayerRenderData &inData,
                                      QDemonRenderableObject &inObject,
                                      const QVector2D &inCameraProps,
                                      const TShaderFeatureSet &,
                                      quint32,
                                      const QDemonRenderCamera &inCamera)
{
    if (inObject.renderableFlags.isDefaultMaterialMeshSubset())
        static_cast<QDemonSubsetRenderable &>(inObject).renderDepthPass(inCameraProps);
    else if (inObject.renderableFlags.IsText())
        static_cast<QDemonTextRenderable &>(inObject).renderDepthPass(inCameraProps);
    else if (inObject.renderableFlags.isCustomMaterialMeshSubset()) {
        static_cast<QDemonCustomMaterialRenderable &>(inObject).renderDepthPass(inCameraProps, inData.layer, inData.lights, inCamera, nullptr);
    } else if (inObject.renderableFlags.isPath()) {
        static_cast<QDemonPathRenderable &>(inObject).renderDepthPass(inCameraProps, inData.layer, inData.lights, inCamera, nullptr);
    } else {
        Q_ASSERT(false);
    }
}

void QDemonLayerRenderData::renderDepthPass(bool inEnableTransparentDepthWrite)
{
    QDemonStackPerfTimer ___timer(renderer->getDemonContext()->getPerfTimer(), Q_FUNC_INFO);
    if (camera == nullptr)
        return;

    // Avoid running this method if possible.
    if ((inEnableTransparentDepthWrite == false && (opaqueObjects.size() == 0 || layer.flags.isLayerEnableDepthPrepass() == false))
        || layer.flags.isLayerEnableDepthTest() == false)
        return;

    renderer->beginLayerDepthPassRender(*this);

    auto theRenderContext = renderer->getContext();

    // disable color writes
    theRenderContext->setColorWritesEnabled(false);
    theRenderContext->setDepthWriteEnabled(true);

    QDemonRenderClearFlags clearFlags(QDemonRenderClearValues::Stencil | QDemonRenderClearValues::Depth);
    theRenderContext->clear(clearFlags);

    runRenderPass(renderRenderableDepthPass, false, true, inEnableTransparentDepthWrite, 0, *camera);

    // enable color writes
    theRenderContext->setColorWritesEnabled(true);

    renderer->endLayerDepthPassRender();
}

inline void renderRenderable(QDemonLayerRenderData &inData,
                             QDemonRenderableObject &inObject,
                             const QVector2D &inCameraProps,
                             const TShaderFeatureSet &inFeatureSet,
                             quint32,
                             const QDemonRenderCamera &inCamera)
{
    if (inObject.renderableFlags.isDefaultMaterialMeshSubset())
        static_cast<QDemonSubsetRenderable &>(inObject).render(inCameraProps, inFeatureSet);
    else if (inObject.renderableFlags.IsText())
        static_cast<QDemonTextRenderable &>(inObject).render(inCameraProps);
    else if (inObject.renderableFlags.isCustomMaterialMeshSubset()) {
        // PKC : Need a better place to do this.
        QDemonCustomMaterialRenderable &theObject = static_cast<QDemonCustomMaterialRenderable &>(inObject);
        if (!inData.layer.lightProbe && theObject.material.m_iblProbe)
            inData.setShaderFeature("QDEMON_ENABLE_LIGHT_PROBE", theObject.material.m_iblProbe->m_textureData.m_texture != nullptr);
        else if (inData.layer.lightProbe)
            inData.setShaderFeature("QDEMON_ENABLE_LIGHT_PROBE", inData.layer.lightProbe->m_textureData.m_texture != nullptr);

        static_cast<QDemonCustomMaterialRenderable &>(inObject).render(inCameraProps,
                                                                       inData,
                                                                       inData.layer,
                                                                       inData.lights,
                                                                       inCamera,
                                                                       inData.m_layerDepthTexture,
                                                                       inData.m_layerSsaoTexture,
                                                                       inFeatureSet);
    } else if (inObject.renderableFlags.isPath()) {
        static_cast<QDemonPathRenderable &>(inObject).render(inCameraProps,
                                                             inData.layer,
                                                             inData.lights,
                                                             inCamera,
                                                             inData.m_layerDepthTexture,
                                                             inData.m_layerSsaoTexture,
                                                             inFeatureSet);
    } else {
        Q_ASSERT(false);
    }
}

void QDemonLayerRenderData::runRenderPass(TRenderRenderableFunction inRenderFn,
                                          bool inEnableBlending,
                                          bool inEnableDepthWrite,
                                          bool inEnableTransparentDepthWrite,
                                          quint32 indexLight,
                                          const QDemonRenderCamera &inCamera,
                                          QDemonResourceFrameBuffer *theFB)
{
    auto theRenderContext = renderer->getContext();
    theRenderContext->setDepthFunction(QDemonRenderBoolOp::LessThanOrEqual);
    theRenderContext->setBlendingEnabled(false);
    QVector2D theCameraProps = QVector2D(camera->clipNear, camera->clipFar);
    auto theOpaqueObjects = getOpaqueRenderableObjects();
    bool usingDepthBuffer = layer.flags.isLayerEnableDepthTest() && theOpaqueObjects.size() > 0;

    if (usingDepthBuffer) {
        theRenderContext->setDepthTestEnabled(true);
        theRenderContext->setDepthWriteEnabled(inEnableDepthWrite);
    } else {
        theRenderContext->setDepthWriteEnabled(false);
        theRenderContext->setDepthTestEnabled(false);
    }

    for (quint32 idx = 0, end = theOpaqueObjects.size(); idx < end; ++idx) {
        QDemonRenderableObject &theObject(*theOpaqueObjects[idx]);
        QDemonScopedLightsListScope lightsScope(lights, lightDirections, sourceLightDirections, theObject.scopedLights);
        setShaderFeature(cgLightingFeatureName, lights.empty() == false);
        inRenderFn(*this, theObject, theCameraProps, getShaderFeatureSet(), indexLight, inCamera);
    }

    // transparent objects
    if (inEnableBlending || layer.flags.isLayerEnableDepthTest() == false) {
        theRenderContext->setBlendingEnabled(true && inEnableBlending);
        theRenderContext->setDepthWriteEnabled(inEnableTransparentDepthWrite);

        auto theTransparentObjects = getTransparentRenderableObjects();
        // Assume all objects have transparency if the layer's depth test enabled flag is true.
        if (layer.flags.isLayerEnableDepthTest() == true) {
            for (quint32 idx = 0, end = theTransparentObjects.size(); idx < end; ++idx) {
                QDemonRenderableObject &theObject(*theTransparentObjects[idx]);
                if (!(theObject.renderableFlags.isCompletelyTransparent())) {
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    // SW fallback for advanced blend modes.
                    // Renders transparent objects to a separate FBO and blends them in shader
                    // with the opaque items and background.
                    DefaultMaterialBlendMode blendMode = DefaultMaterialBlendMode::Normal;
                    if (theObject.renderableFlags.isDefaultMaterialMeshSubset())
                        blendMode = static_cast<QDemonSubsetRenderable &>(theObject).getBlendingMode();
                    bool useBlendFallback = (blendMode == DefaultMaterialBlendMode::Overlay
                                             || blendMode == DefaultMaterialBlendMode::ColorBurn
                                             || blendMode == DefaultMaterialBlendMode::ColorDodge)
                            && !theRenderContext->isAdvancedBlendHwSupported()
                            && !theRenderContext->isAdvancedBlendHwSupportedKHR() && m_layerPrepassDepthTexture.getTexture();
                    if (useBlendFallback)
                        setupDrawFB(true);
#endif
                    QDemonScopedLightsListScope lightsScope(lights, lightDirections, sourceLightDirections, theObject.scopedLights);
                    setShaderFeature(cgLightingFeatureName, lights.empty() == false);

                    inRenderFn(*this, theObject, theCameraProps, getShaderFeatureSet(), indexLight, inCamera);
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    // SW fallback for advanced blend modes.
                    // Continue blending after transparent objects have been rendered to a FBO
                    if (useBlendFallback) {
                        blendAdvancedToFB(blendMode, true, theFB);
                        // restore blending status
                        theRenderContext->setBlendingEnabled(inEnableBlending);
                        // restore depth test status
                        theRenderContext->setDepthTestEnabled(usingDepthBuffer);
                        theRenderContext->setDepthWriteEnabled(inEnableTransparentDepthWrite);
                    }
#endif
                }
            }
        }
        // If the layer doesn't have depth enabled then we have to render via an alternate route
        // where the transparent objects vector could have both opaque and transparent objects.
        else {
            for (quint32 idx = 0, end = theTransparentObjects.size(); idx < end; ++idx) {
                QDemonRenderableObject &theObject(*theTransparentObjects[idx]);
                if (!(theObject.renderableFlags.isCompletelyTransparent())) {
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    DefaultMaterialBlendMode blendMode = DefaultMaterialBlendMode::Normal;
                    if (theObject.renderableFlags.isDefaultMaterialMeshSubset())
                        blendMode = static_cast<QDemonSubsetRenderable &>(theObject).getBlendingMode();
                    bool useBlendFallback = (blendMode == DefaultMaterialBlendMode::Overlay
                                             || blendMode == DefaultMaterialBlendMode::ColorBurn
                                             || blendMode == DefaultMaterialBlendMode::ColorDodge)
                            && !theRenderContext->isAdvancedBlendHwSupported()
                            && !theRenderContext->isAdvancedBlendHwSupportedKHR();

                    if (theObject.renderableFlags.hasTransparency()) {
                        theRenderContext->setBlendingEnabled(true && inEnableBlending);
                        // If we have SW fallback for blend mode, render to a FBO and blend back.
                        // Slow as this must be done per-object (transparent and opaque items
                        // are mixed, not batched)
                        if (useBlendFallback)
                            setupDrawFB(false);
                    }
#endif
                    QDemonScopedLightsListScope lightsScope(lights, lightDirections, sourceLightDirections, theObject.scopedLights);
                    setShaderFeature(cgLightingFeatureName, lights.empty() == false);
                    inRenderFn(*this, theObject, theCameraProps, getShaderFeatureSet(), indexLight, inCamera);
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    if (useBlendFallback) {
                        blendAdvancedToFB(blendMode, false, theFB);
                        // restore blending status
                        theRenderContext->setBlendingEnabled(inEnableBlending);
                    }
#endif
                }
            }
        }
    }
}

void QDemonLayerRenderData::render(QDemonResourceFrameBuffer *theFB)
{
    QDemonStackPerfTimer ___timer(renderer->getDemonContext()->getPerfTimer(), Q_FUNC_INFO);
    if (camera == nullptr)
        return;

    renderer->beginLayerRender(*this);
    runRenderPass(renderRenderable, true, !layer.flags.isLayerEnableDepthPrepass(), false, 0, *camera, theFB);
    renderer->endLayerRender();
}

void QDemonLayerRenderData::createGpuProfiler()
{
    //        if (renderer->getContext().isTimerQuerySupported()) {
    //            m_layerProfilerGpu = QDemonRenderProfilerInterface::createGpuProfiler(
    //                        renderer->getContext().getFoundation(), renderer.getDemonContext(),
    //                        renderer->getContext());
    //        }
}

void QDemonLayerRenderData::startProfiling(QString &nameID, bool sync)
{
    if (m_layerProfilerGpu) {
        m_layerProfilerGpu->startTimer(nameID, false, sync);
    }
}

void QDemonLayerRenderData::endProfiling(QString &nameID)
{
    if (m_layerProfilerGpu) {
        m_layerProfilerGpu->endTimer(nameID);
    }
}

void QDemonLayerRenderData::startProfiling(const char *nameID, bool sync)
{
    if (m_layerProfilerGpu) {
        QString theStr(QString::fromLocal8Bit(nameID));
        m_layerProfilerGpu->startTimer(theStr, false, sync);
    }
}

void QDemonLayerRenderData::endProfiling(const char *nameID)
{
    if (m_layerProfilerGpu) {
        QString theStr(QString::fromLocal8Bit(nameID));
        m_layerProfilerGpu->endTimer(theStr);
    }
}

void QDemonLayerRenderData::addVertexCount(quint32 count)
{
    if (m_layerProfilerGpu) {
        m_layerProfilerGpu->addVertexCount(count);
    }
}

// Assumes the viewport is setup appropriately to render the widget.
void QDemonLayerRenderData::renderRenderWidgets()
{
    if (camera) {
        auto theContext = renderer->getContext();
        for (int idx = 0, end = iRenderWidgets.size(); idx < end; ++idx) {
            QDemonRenderWidgetInterface &theWidget = *iRenderWidgets[idx];
            theWidget.render(*renderer, *theContext);
        }
    }
}

#ifdef ADVANCED_BLEND_SW_FALLBACK
void QDemonLayerRenderData::blendAdvancedEquationSwFallback(const QDemonRef<QDemonRenderTexture2D> &drawTexture,
                                                            const QDemonRef<QDemonRenderTexture2D> &layerTexture,
                                                            AdvancedBlendModes blendMode)
{
    auto theContext = renderer->getContext();
    QDemonRef<QDemonAdvancedModeBlendShader> shader = renderer->getAdvancedBlendModeShader(blendMode);
    if (shader == nullptr)
        return;

    theContext->setActiveShader((shader->shader));

    shader->baseLayer.set(layerTexture.data());
    shader->blendLayer.set(drawTexture.data());
    // Draw a fullscreen quad
    renderer->renderQuad();
}

void QDemonLayerRenderData::setupDrawFB(bool depthEnabled)
{
    auto theRenderContext = renderer->getContext();
    // create drawing FBO and texture, if not existing
    if (!m_advancedModeDrawFB)
        m_advancedModeDrawFB = theRenderContext->createFrameBuffer();
    if (!m_advancedBlendDrawTexture) {
        m_advancedBlendDrawTexture = theRenderContext->createTexture2D();
        QRect theViewport = renderer->getDemonContext()->getRenderList()->getViewport();
        m_advancedBlendDrawTexture->setTextureData(QDemonDataRef<quint8>(), 0, theViewport.width(), theViewport.height(), QDemonRenderTextureFormat::RGBA8);
        m_advancedModeDrawFB->attach(QDemonRenderFrameBufferAttachment::Color0, m_advancedBlendDrawTexture);
        // Use existing depth prepass information when rendering transparent objects to a FBO
        if (depthEnabled)
            m_advancedModeDrawFB->attach(QDemonRenderFrameBufferAttachment::Depth, m_layerPrepassDepthTexture.getTexture());
    }
    theRenderContext->setRenderTarget(m_advancedModeDrawFB);
    // make sure that depth testing is on in order to render just the
    // depth-passed objects (=transparent objects) and leave background intact
    if (depthEnabled)
        theRenderContext->setDepthTestEnabled(true);
    theRenderContext->setBlendingEnabled(false);
    // clear color commonly is the layer background, make sure that it is all-zero here
    QVector4D originalClrColor = theRenderContext->getClearColor();
    theRenderContext->setClearColor(QVector4D(0.0, 0.0, 0.0, 0.0));
    theRenderContext->clear(QDemonRenderClearValues::Color);
    theRenderContext->setClearColor(originalClrColor);
}
void QDemonLayerRenderData::blendAdvancedToFB(DefaultMaterialBlendMode blendMode, bool depthEnabled, QDemonResourceFrameBuffer *theFB)
{
    auto theRenderContext = renderer->getContext();
    QRect theViewport = renderer->getDemonContext()->getRenderList()->getViewport();
    AdvancedBlendModes advancedMode;

    switch (blendMode) {
    case DefaultMaterialBlendMode::Overlay:
        advancedMode = AdvancedBlendModes::Overlay;
        break;
    case DefaultMaterialBlendMode::ColorBurn:
        advancedMode = AdvancedBlendModes::ColorBurn;
        break;
    case DefaultMaterialBlendMode::ColorDodge:
        advancedMode = AdvancedBlendModes::ColorDodge;
        break;
    default:
        Q_UNREACHABLE();
    }
    // create blending FBO and texture if not existing
    if (!m_advancedModeBlendFB)
        m_advancedModeBlendFB = theRenderContext->createFrameBuffer();
    if (!m_advancedBlendBlendTexture) {
        m_advancedBlendBlendTexture = theRenderContext->createTexture2D();
        m_advancedBlendBlendTexture->setTextureData(QDemonDataRef<quint8>(), 0, theViewport.width(), theViewport.height(), QDemonRenderTextureFormat::RGBA8);
        m_advancedModeBlendFB->attach(QDemonRenderFrameBufferAttachment::Color0, m_advancedBlendBlendTexture);
    }
    theRenderContext->setRenderTarget(m_advancedModeBlendFB);

    // Blend transparent objects with SW fallback shaders.
    // Disable depth testing as transparent objects have already been
    // depth-checked; here we want to run shader for all layer pixels
    if (depthEnabled) {
        theRenderContext->setDepthTestEnabled(false);
        theRenderContext->setDepthWriteEnabled(false);
    }
    blendAdvancedEquationSwFallback(m_advancedBlendDrawTexture, m_layerTexture, advancedMode);
    theRenderContext->setRenderTarget(*theFB);
    // setup read target
    theRenderContext->setReadTarget(m_advancedModeBlendFB);
    theRenderContext->setReadBuffer(QDemonReadFace::Color0);
    theRenderContext->blitFramebuffer(0,
                                      0,
                                      theViewport.width(),
                                      theViewport.height(),
                                      0,
                                      0,
                                      theViewport.width(),
                                      theViewport.height(),
                                      QDemonRenderClearValues::Color,
                                      QDemonRenderTextureMagnifyingOp::Nearest);
}
#endif

void QDemonLayerRenderData::renderToViewport()
{
    if (layerPrepResult->isLayerVisible()) {
        if (getOffscreenRenderer()) {
            if (layer.background == LayerBackground::Color) {
                lastFrameOffscreenRenderer->renderWithClear(createOffscreenRenderEnvironment(),
                                                            *renderer->getContext(),
                                                            renderer->getDemonContext()->getPresentationScaleFactor(),
                                                            QDemonRenderScene::AlwaysClear,
                                                            layer.clearColor,
                                                            &layer);
            } else {
                lastFrameOffscreenRenderer->render(createOffscreenRenderEnvironment(),
                                                   *renderer->getContext(),
                                                   renderer->getDemonContext()->getPresentationScaleFactor(),
                                                   QDemonRenderScene::ClearIsOptional,
                                                   &layer);
            }
        } else {
            renderDepthPass(false);
            render();
            renderRenderWidgets();
        }
    }
}
// These are meant to be pixel offsets, so you need to divide them by the width/height
// of the layer respectively.
const QVector2D s_VertexOffsets[QDemonLayerRenderPreparationData::MAX_AA_LEVELS] = {
    QVector2D(-0.170840f, -0.553840f), // 1x
    QVector2D(0.162960f, -0.319340f), // 2x
    QVector2D(0.360260f, -0.245840f), // 3x
    QVector2D(-0.561340f, -0.149540f), // 4x
    QVector2D(0.249460f, 0.453460f), // 5x
    QVector2D(-0.336340f, 0.378260f), // 6x
    QVector2D(0.340000f, 0.166260f), // 7x
    QVector2D(0.235760f, 0.527760f), // 8x
};

// Blend factors are in the form of (frame blend factor, accumulator blend factor)
const QVector2D s_BlendFactors[QDemonLayerRenderPreparationData::MAX_AA_LEVELS] = {
    QVector2D(0.500000f, 0.500000f), // 1x
    QVector2D(0.333333f, 0.666667f), // 2x
    QVector2D(0.250000f, 0.750000f), // 3x
    QVector2D(0.200000f, 0.800000f), // 4x
    QVector2D(0.166667f, 0.833333f), // 5x
    QVector2D(0.142857f, 0.857143f), // 6x
    QVector2D(0.125000f, 0.875000f), // 7x
    QVector2D(0.111111f, 0.888889f), // 8x
};

const QVector2D s_TemporalVertexOffsets[QDemonLayerRenderPreparationData::MAX_TEMPORAL_AA_LEVELS] = { QVector2D(.3f, .3f),
                                                                                                      QVector2D(-.3f, -.3f) };

static inline void offsetProjectionMatrix(QMatrix4x4 &inProjectionMatrix, QVector2D inVertexOffsets)
{
    inProjectionMatrix(3, 0) = inProjectionMatrix(3, 0) + inProjectionMatrix(3, 3) * inVertexOffsets.x();
    inProjectionMatrix(3, 1) = inProjectionMatrix(3, 1) + inProjectionMatrix(3, 3) * inVertexOffsets.y();
}

// Render this layer's data to a texture.  Required if we have any effects,
// prog AA, or if forced.
void QDemonLayerRenderData::renderToTexture()
{
    Q_ASSERT(layerPrepResult->flags.shouldRenderToTexture());
    QDemonLayerRenderPreparationResult &thePrepResult(*layerPrepResult);
    auto theRenderContext = renderer->getContext();
    QSize theLayerTextureDimensions = thePrepResult.getTextureDimensions();
    QSize theLayerOriginalTextureDimensions = theLayerTextureDimensions;
    QDemonRenderTextureFormat DepthTextureFormat = QDemonRenderTextureFormat::Depth24Stencil8;
    QDemonRenderTextureFormat ColorTextureFormat = QDemonRenderTextureFormat::RGBA8;
    if (thePrepResult.lastEffect && theRenderContext->getRenderContextType() != QDemonRenderContextType::GLES2) {
        if (layer.background != LayerBackground::Transparent)
            ColorTextureFormat = QDemonRenderTextureFormat::R11G11B10;
        else
            ColorTextureFormat = QDemonRenderTextureFormat::RGBA16F;
    }
    QDemonRenderTextureFormat ColorSSAOTextureFormat = QDemonRenderTextureFormat::RGBA8;

    bool needsRender = false;
    qint32 sampleCount = 1;
    // check multsample mode and MSAA texture support
    if (layer.multisampleAAMode != AAModeValues::NoAA && theRenderContext->areMultisampleTexturesSupported())
        sampleCount = qint32(layer.multisampleAAMode);

    bool isMultisamplePass = false;
    if (theRenderContext->getRenderContextType() != QDemonRenderContextType::GLES2)
        isMultisamplePass = (sampleCount > 1) || (layer.multisampleAAMode == AAModeValues::SSAA);

    QDemonRenderTextureTargetType thFboAttachTarget = QDemonRenderTextureTargetType::Texture2D;

    // If the user has disabled all layer caching this has the side effect of disabling the
    // progressive AA algorithm.
    if (thePrepResult.flags.wasLayerDataDirty() || thePrepResult.flags.wasDirty()
        || renderer->isLayerCachingEnabled() == false || thePrepResult.flags.shouldRenderToTexture()) {
        m_progressiveAAPassIndex = 0;
        m_nonDirtyTemporalAAPassIndex = 0;
        needsRender = true;
    }

    QDemonResourceTexture2D *renderColorTexture = &m_layerTexture;
    QDemonResourceTexture2D *renderPrepassDepthTexture = &m_layerPrepassDepthTexture;
    QDemonResourceTexture2D *renderWidgetTexture = &m_layerWidgetTexture;
    QDemonRenderContextScopedProperty<bool> __multisampleEnabled(*theRenderContext,
                                                                 &QDemonRenderContext::isMultisampleEnabled,
                                                                 &QDemonRenderContext::setMultisampleEnabled);
    theRenderContext->setMultisampleEnabled(false);
    if (isMultisamplePass) {
        renderColorTexture = &m_layerMultisampleTexture;
        renderPrepassDepthTexture = &m_layerMultisamplePrepassDepthTexture;
        renderWidgetTexture = &m_layerMultisampleWidgetTexture;
        // for SSAA we don't use MS textures
        if (layer.multisampleAAMode != AAModeValues::SSAA)
            thFboAttachTarget = QDemonRenderTextureTargetType::Texture2D_MS;
    }
    quint32 maxTemporalPassIndex = layer.temporalAAEnabled ? 2 : 0;

    // If all the dimensions match then we do not have to re-render the layer.
    if (m_layerTexture.textureMatches(theLayerTextureDimensions.width(), theLayerTextureDimensions.height(), ColorTextureFormat)
        && (!thePrepResult.flags.requiresDepthTexture()
            || m_layerDepthTexture.textureMatches(theLayerTextureDimensions.width(), theLayerTextureDimensions.height(), DepthTextureFormat))
        && m_progressiveAAPassIndex >= thePrepResult.maxAAPassIndex
        && m_nonDirtyTemporalAAPassIndex >= maxTemporalPassIndex && needsRender == false) {
        return;
    }

    // adjust render size for SSAA
    if (layer.multisampleAAMode == AAModeValues::SSAA) {
        qint32 ow, oh;
        QDemonRendererUtil::getSSAARenderSize(theLayerOriginalTextureDimensions.width(),
                                              theLayerOriginalTextureDimensions.height(),
                                              ow,
                                              oh);
        theLayerTextureDimensions = QSize(ow, oh);
    }

    // If our pass index == thePreResult.m_MaxAAPassIndex then
    // we shouldn't get into here.

    QDemonRef<QDemonResourceManagerInterface> theResourceManager = renderer->getDemonContext()->getResourceManager();
    bool hadLayerTexture = true;

    if (renderColorTexture->ensureTexture(theLayerTextureDimensions.width(), theLayerTextureDimensions.height(), ColorTextureFormat, sampleCount)) {
        m_progressiveAAPassIndex = 0;
        m_nonDirtyTemporalAAPassIndex = 0;
        hadLayerTexture = false;
    }

    if (thePrepResult.flags.requiresDepthTexture()) {
        // The depth texture doesn't need to be multisample, the prepass depth does.
        if (m_layerDepthTexture.ensureTexture(theLayerTextureDimensions.width(), theLayerTextureDimensions.height(), DepthTextureFormat)) {
            // Depth textures are generally not bilinear filtered.
            m_layerDepthTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Nearest);
            m_layerDepthTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Nearest);
            m_progressiveAAPassIndex = 0;
            m_nonDirtyTemporalAAPassIndex = 0;
        }
    }

    if (thePrepResult.flags.requiresSsaoPass()) {
        if (m_layerSsaoTexture.ensureTexture(theLayerTextureDimensions.width(), theLayerTextureDimensions.height(), ColorSSAOTextureFormat)) {
            m_layerSsaoTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            m_layerSsaoTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            m_progressiveAAPassIndex = 0;
            m_nonDirtyTemporalAAPassIndex = 0;
        }
    }

    Q_ASSERT(!thePrepResult.flags.requiresDepthTexture() || m_layerDepthTexture.getTexture());
    Q_ASSERT(!thePrepResult.flags.requiresSsaoPass() || m_layerSsaoTexture.getTexture());

    QDemonResourceTexture2D theLastLayerTexture(theResourceManager);
    QDemonRef<QDemonLayerProgAABlendShader> theBlendShader = nullptr;
    quint32 aaFactorIndex = 0;
    bool isProgressiveAABlendPass = m_progressiveAAPassIndex && m_progressiveAAPassIndex < thePrepResult.maxAAPassIndex;
    bool isTemporalAABlendPass = layer.temporalAAEnabled && m_progressiveAAPassIndex == 0;

    if (isProgressiveAABlendPass || isTemporalAABlendPass) {
        theBlendShader = renderer->getLayerProgAABlendShader();
        if (theBlendShader) {
            m_layerTexture.ensureTexture(theLayerOriginalTextureDimensions.width(),
                                         theLayerOriginalTextureDimensions.height(),
                                         ColorTextureFormat);
            QVector2D theVertexOffsets;
            if (isProgressiveAABlendPass) {
                theLastLayerTexture.stealTexture(m_layerTexture);
                aaFactorIndex = (m_progressiveAAPassIndex - 1);
                theVertexOffsets = s_VertexOffsets[aaFactorIndex];
            } else {
                if (m_temporalAATexture.getTexture())
                    theLastLayerTexture.stealTexture(m_temporalAATexture);
                else {
                    if (hadLayerTexture) {
                        theLastLayerTexture.stealTexture(m_layerTexture);
                    }
                }
                theVertexOffsets = s_TemporalVertexOffsets[m_temporalAAPassIndex];
                ++m_temporalAAPassIndex;
                ++m_nonDirtyTemporalAAPassIndex;
                m_temporalAAPassIndex = m_temporalAAPassIndex % MAX_TEMPORAL_AA_LEVELS;
            }
            if (theLastLayerTexture.getTexture()) {
                theVertexOffsets.setX(theVertexOffsets.x() / (theLayerOriginalTextureDimensions.width() / 2.0f));
                theVertexOffsets.setY(theVertexOffsets.y() / (theLayerOriginalTextureDimensions.height() / 2.0f));
                // Run through all models and update MVP.
                // run through all texts and update MVP.
                // run through all path and update MVP.

                // TODO - optimize this exact matrix operation.
                for (qint32 idx = 0, end = modelContexts.size(); idx < end; ++idx) {
                    QMatrix4x4 &originalProjection(modelContexts[idx]->modelViewProjection);
                    offsetProjectionMatrix(originalProjection, theVertexOffsets);
                }
                for (qint32 idx = 0, end = opaqueObjects.size(); idx < end; ++idx) {
                    if (opaqueObjects[idx]->renderableFlags.isPath()) {
                        QDemonPathRenderable &theRenderable = static_cast<QDemonPathRenderable &>(*opaqueObjects[idx]);
                        offsetProjectionMatrix(theRenderable.m_mvp, theVertexOffsets);
                    }
                }
                for (qint32 idx = 0, end = transparentObjects.size(); idx < end; ++idx) {
                    if (transparentObjects[idx]->renderableFlags.IsText()) {
                        QDemonTextRenderable &theRenderable = static_cast<QDemonTextRenderable &>(*transparentObjects[idx]);
                        offsetProjectionMatrix(theRenderable.modelViewProjection, theVertexOffsets);
                    } else if (transparentObjects[idx]->renderableFlags.isPath()) {
                        QDemonPathRenderable &theRenderable = static_cast<QDemonPathRenderable &>(*transparentObjects[idx]);
                        offsetProjectionMatrix(theRenderable.m_mvp, theVertexOffsets);
                    }
                }
            }
        }
    }
    if (theLastLayerTexture.getTexture() == nullptr) {
        isProgressiveAABlendPass = false;
        isTemporalAABlendPass = false;
    }
    // Sometimes we will have stolen the render texture.
    renderColorTexture->ensureTexture(theLayerTextureDimensions.width(), theLayerTextureDimensions.height(), ColorTextureFormat, sampleCount);

    if (!isTemporalAABlendPass)
        m_temporalAATexture.releaseTexture();

    // Allocating a frame buffer can cause it to be bound, so we need to save state before this
    // happens.
    QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuf(*theRenderContext,
                                                                                     &QDemonRenderContext::getRenderTarget,
                                                                                     &QDemonRenderContext::setRenderTarget);
    // Match the bit depth of the current render target to avoid popping when we switch from aa
    // to non aa layers
    // We have to all this here in because once we change the FB by allocating an FB we are
    // screwed.
    QDemonRenderTextureFormat theDepthFormat(getDepthBufferFormat());
    QDemonRenderFrameBufferAttachment theDepthAttachmentFormat(getFramebufferDepthAttachmentFormat(theDepthFormat));

    // Definitely disable the scissor rect if it is running right now.
    QDemonRenderContextScopedProperty<bool> __scissorEnabled(*theRenderContext,
                                                             &QDemonRenderContext::isScissorTestEnabled,
                                                             &QDemonRenderContext::setScissorTestEnabled,
                                                             false);
    QDemonResourceFrameBuffer theFB(theResourceManager);
    // Allocates the frame buffer which has the side effect of setting the current render target
    // to that frame buffer.
    // TODO:
    theFB.ensureFrameBuffer();

    bool hasDepthObjects = opaqueObjects.size() > 0;
    bool requiresDepthStencilBuffer = hasDepthObjects || thePrepResult.flags.requiresStencilBuffer();
    QRect theNewViewport(0, 0, theLayerTextureDimensions.width(), theLayerTextureDimensions.height());
    {
        theRenderContext->setRenderTarget(theFB);
        QDemonRenderContextScopedProperty<QRect> __viewport(*theRenderContext,
                                                            &QDemonRenderContext::getViewport,
                                                            &QDemonRenderContext::setViewport,
                                                            theNewViewport);
        QVector4D clearColor(0.0, 0.0, 0.0, 0.0);
        if (layer.background == LayerBackground::Color)
            clearColor = QVector4D(layer.clearColor, 1.0);

        QDemonRenderContextScopedProperty<QVector4D> __clearColor(*theRenderContext,
                                                                  &QDemonRenderContext::getClearColor,
                                                                  &QDemonRenderContext::setClearColor,
                                                                  clearColor);
        if (requiresDepthStencilBuffer) {
            if (renderPrepassDepthTexture->ensureTexture(theLayerTextureDimensions.width(),
                                                         theLayerTextureDimensions.height(),
                                                         theDepthFormat,
                                                         sampleCount)) {
                (*renderPrepassDepthTexture)->setMinFilter(QDemonRenderTextureMinifyingOp::Nearest);
                (*renderPrepassDepthTexture)->setMagFilter(QDemonRenderTextureMagnifyingOp::Nearest);
            }
        }

        if (thePrepResult.flags.requiresDepthTexture() && m_progressiveAAPassIndex == 0) {
            // Setup FBO with single depth buffer target.
            // Note this does not use multisample.
            QDemonRenderFrameBufferAttachment theAttachment = getFramebufferDepthAttachmentFormat(DepthTextureFormat);
            theFB->attach(theAttachment, m_layerDepthTexture.getTexture());

            // In this case transparent objects also may write their depth.
            renderDepthPass(true);
            theFB->attach(theAttachment, QDemonRenderTextureOrRenderBuffer());
        }

        if (thePrepResult.flags.requiresSsaoPass() && m_progressiveAAPassIndex == 0 && camera != nullptr) {
            startProfiling("AO pass", false);
            // Setup FBO with single color buffer target
            theFB->attach(QDemonRenderFrameBufferAttachment::Color0, m_layerSsaoTexture.getTexture());
            theRenderContext->clear(QDemonRenderClearValues::Color);
            renderAoPass();
            theFB->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
            endProfiling("AO pass");
        }

        if (thePrepResult.flags.requiresShadowMapPass() && m_progressiveAAPassIndex == 0) {
            // shadow map path
            renderShadowMapPass(&theFB);
        }

        if (sampleCount > 1) {
            theRenderContext->setMultisampleEnabled(true);
        }

        QDemonRenderClearFlags clearFlags = QDemonRenderClearValues::Color;

        // render depth prepass
        if (renderPrepassDepthTexture->getTexture()) {
            theFB->attach(theDepthAttachmentFormat, renderPrepassDepthTexture->getTexture(), thFboAttachTarget);

            if (layer.flags.isLayerEnableDepthPrepass()) {
                startProfiling("Depth pass", false);
                renderDepthPass(false);
                endProfiling("Depth pass");
            } else {
                clearFlags |= (QDemonRenderClearValues::Depth);
                clearFlags |= (QDemonRenderClearValues::Stencil);
                // enable depth write for the clear below
                theRenderContext->setDepthWriteEnabled(true);
            }
        }

        theFB->attach(QDemonRenderFrameBufferAttachment::Color0, renderColorTexture->getTexture(), thFboAttachTarget);
        if (layer.background != LayerBackground::Unspecified)
            theRenderContext->clear(clearFlags);

        // We don't clear the depth buffer because the layer render code we are about to call
        // will do this.
        startProfiling("Render pass", false);
        render(&theFB);
        // Debug measure to view the depth map to ensure we're rendering it correctly.
        // if (m_Layer.m_TemporalAAEnabled) {
        //    RenderFakeDepthMapPass(m_ShadowMapManager->GetShadowMapEntry(0)->m_DepthMap,
        //                           m_ShadowMapManager->GetShadowMapEntry(0)->m_DepthCube);
        //}
        endProfiling("Render pass");

        // Now before going further, we downsample and resolve the multisample information.
        // This allows all algorithms running after
        // this point to run unchanged.
        if (isMultisamplePass) {
            if (layer.multisampleAAMode != AAModeValues::SSAA) {
                // Resolve the FBO to the layer texture
                QDemonRendererUtil::resolveMutisampleFBOColorOnly(theResourceManager,
                                                                  m_layerTexture,
                                                                  *theRenderContext,
                                                                  theLayerTextureDimensions.width(),
                                                                  theLayerTextureDimensions.height(),
                                                                  ColorTextureFormat,
                                                                  theFB.getFrameBuffer());

                theRenderContext->setMultisampleEnabled(false);
            } else {
                // Resolve the FBO to the layer texture
                QDemonRendererUtil::resolveSSAAFBOColorOnly(theResourceManager,
                                                            m_layerTexture,
                                                            theLayerOriginalTextureDimensions.width(),
                                                            theLayerOriginalTextureDimensions.height(),
                                                            *theRenderContext,
                                                            theLayerTextureDimensions.width(),
                                                            theLayerTextureDimensions.height(),
                                                            ColorTextureFormat,
                                                            theFB);
            }
        }

        // CN - when I tried to get anti-aliased widgets I lost all transparency on the widget
        // layer which made it overwrite the object you were
        // manipulating.  When I tried to use parallel nsight on it the entire studio
        // application crashed on startup.
        if (needsWidgetTexture()) {
            m_layerWidgetTexture.ensureTexture(theLayerTextureDimensions.width(),
                                               theLayerTextureDimensions.height(),
                                               QDemonRenderTextureFormat::RGBA8);
            theRenderContext->setRenderTarget(theFB);
            theFB->attach(QDemonRenderFrameBufferAttachment::Color0, m_layerWidgetTexture.getTexture());
            theFB->attach(getFramebufferDepthAttachmentFormat(DepthTextureFormat), m_layerDepthTexture.getTexture());
            theRenderContext->setClearColor(QVector4D(0.0, 0.0, 0.0, 0.0));
            theRenderContext->clear(QDemonRenderClearValues::Color);
            // We should already have the viewport and everything setup for this.
            renderRenderWidgets();
        }

        if (theLastLayerTexture.getTexture() != nullptr && (isProgressiveAABlendPass || isTemporalAABlendPass)) {
            theRenderContext->setViewport(
                    QRect(0, 0, theLayerOriginalTextureDimensions.width(), theLayerOriginalTextureDimensions.height()));
            QDemonResourceTexture2D targetTexture(theResourceManager,
                                                  theLayerOriginalTextureDimensions.width(),
                                                  theLayerOriginalTextureDimensions.height(),
                                                  ColorTextureFormat);
            theFB->attach(theDepthAttachmentFormat, QDemonRenderTextureOrRenderBuffer());
            theFB->attach(QDemonRenderFrameBufferAttachment::Color0, targetTexture.getTexture());
            QVector2D theBlendFactors;
            if (isProgressiveAABlendPass)
                theBlendFactors = s_BlendFactors[aaFactorIndex];
            else
                theBlendFactors = QVector2D(.5f, .5f);

            theRenderContext->setDepthTestEnabled(false);
            theRenderContext->setBlendingEnabled(false);
            theRenderContext->setCullingEnabled(false);
            theRenderContext->setActiveShader(theBlendShader->shader);
            theBlendShader->accumSampler.set(theLastLayerTexture.getTexture().data());
            theBlendShader->lastFrame.set(m_layerTexture.getTexture().data());
            theBlendShader->blendFactors.set(theBlendFactors);
            renderer->renderQuad();
            theFB->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
            if (isTemporalAABlendPass)
                m_temporalAATexture.stealTexture(m_layerTexture);
            m_layerTexture.stealTexture(targetTexture);
        }

        m_layerTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        m_layerTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);

        // Don't remember why needs widget texture is false here.
        // Should have commented why progAA plus widgets is a fail.
        if (m_progressiveAAPassIndex < thePrepResult.maxAAPassIndex && needsWidgetTexture() == false)
            ++m_progressiveAAPassIndex;

            // now we render all post effects
#ifdef QDEMON_CACHED_POST_EFFECT
        applyLayerPostEffects();
#endif

        if (m_layerPrepassDepthTexture.getTexture()) {
            // Detach any depth buffers.
            theFB->attach(theDepthAttachmentFormat, QDemonRenderTextureOrRenderBuffer(), thFboAttachTarget);
        }

        theFB->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer(), thFboAttachTarget);
        // Let natural scoping rules destroy the other stuff.
    }
}

void QDemonLayerRenderData::applyLayerPostEffects()
{
    if (layer.firstEffect == nullptr) {
        if (m_layerCachedTexture) {
            QDemonRef<QDemonResourceManagerInterface> theResourceManager(renderer->getDemonContext()->getResourceManager());
            theResourceManager->release(m_layerCachedTexture);
            m_layerCachedTexture = nullptr;
        }
        return;
    }

    QDemonRef<QDemonEffectSystemInterface> theEffectSystem(renderer->getDemonContext()->getEffectSystem());
    QDemonRef<QDemonResourceManagerInterface> theResourceManager(renderer->getDemonContext()->getResourceManager());
    // we use the non MSAA buffer for the effect
    QDemonRef<QDemonRenderTexture2D> theLayerColorTexture = m_layerTexture.getTexture();
    QDemonRef<QDemonRenderTexture2D> theLayerDepthTexture = m_layerDepthTexture.getTexture();

    QDemonRef<QDemonRenderTexture2D> theCurrentTexture = theLayerColorTexture;
    for (QDemonRenderEffect *theEffect = layer.firstEffect; theEffect; theEffect = theEffect->m_nextEffect) {
        if (theEffect->flags.isActive() && camera) {
            startProfiling(theEffect->className, false);

            QDemonRef<QDemonRenderTexture2D> theRenderedEffect = theEffectSystem->renderEffect(
                    QDemonEffectRenderArgument(theEffect,
                                               theCurrentTexture,
                                               QVector2D(camera->clipNear, camera->clipFar),
                                               theLayerDepthTexture,
                                               m_layerPrepassDepthTexture));

            endProfiling(theEffect->className);

            // If the texture came from rendering a chain of effects, then we don't need it
            // after this.
            if (theCurrentTexture != theLayerColorTexture)
                theResourceManager->release(theCurrentTexture);

            theCurrentTexture = theRenderedEffect;

            if (!theRenderedEffect) {
                QString errorMsg = QObject::tr("Failed to compile \"%1\" effect.\nConsider"
                                               " removing it from the presentation.")
                                           .arg(theEffect->className);
                qFatal(errorMsg.toUtf8());
                break;
            }
        }
    }

    if (m_layerCachedTexture && m_layerCachedTexture != m_layerTexture.getTexture()) {
        theResourceManager->release(m_layerCachedTexture);
        m_layerCachedTexture = nullptr;
    }

    if (theCurrentTexture != m_layerTexture.getTexture())
        m_layerCachedTexture = theCurrentTexture;
}

inline bool anyCompletelyNonTransparentObjects(const TRenderableObjectList &inObjects)
{
    for (int idx = 0, end = inObjects.size(); idx < end; ++idx) {
        if (inObjects[idx]->renderableFlags.isCompletelyTransparent() == false)
            return true;
    }
    return false;
}

void QDemonLayerRenderData::runnableRenderToViewport(const QDemonRef<QDemonRenderFrameBuffer> &theFB)
{
    // If we have an effect, an opaque object, or any transparent objects that aren't completely
    // transparent
    // or an offscreen renderer or a layer widget texture
    // Then we can't possible affect the resulting render target.
    bool needsToRender = layer.firstEffect != nullptr || opaqueObjects.empty() == false
            || anyCompletelyNonTransparentObjects(transparentObjects) || getOffscreenRenderer()
            || m_layerWidgetTexture.getTexture() || m_boundingRectColor.hasValue() || layer.background == LayerBackground::Color;

    if (needsToRender == false)
        return;

    auto theContext = renderer->getContext();
    theContext->resetStates();

    QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __fbo(*theContext,
                                                                                &QDemonRenderContext::getRenderTarget,
                                                                                &QDemonRenderContext::setRenderTarget);
    QRect theCurrentViewport = theContext->getViewport();
    QDemonRenderContextScopedProperty<QRect> __viewport(*theContext, &QDemonRenderContext::getViewport, &QDemonRenderContext::setViewport);
    QDemonRenderContextScopedProperty<bool> theScissorEnabled(*theContext,
                                                              &QDemonRenderContext::isScissorTestEnabled,
                                                              &QDemonRenderContext::setScissorTestEnabled);
    QDemonRenderContextScopedProperty<QRect> theScissorRect(*theContext,
                                                            &QDemonRenderContext::getScissorRect,
                                                            &QDemonRenderContext::setScissorRect);
    QDemonLayerRenderPreparationResult &thePrepResult(*layerPrepResult);
    QRectF theScreenRect(thePrepResult.getLayerToPresentationViewport());

    bool blendingEnabled = layer.background == LayerBackground::Transparent;
    if (!thePrepResult.flags.shouldRenderToTexture()) {
        theContext->setViewport(layerPrepResult->getLayerToPresentationViewport().toRect());
        theContext->setScissorTestEnabled(true);
        theContext->setScissorRect(layerPrepResult->getLayerToPresentationScissorRect().toRect());
        if (layer.background == LayerBackground::Color) {
            QDemonRenderContextScopedProperty<QVector4D> __clearColor(*theContext,
                                                                      &QDemonRenderContext::getClearColor,
                                                                      &QDemonRenderContext::setClearColor,
                                                                      QVector4D(layer.clearColor, 0.0f));
            theContext->clear(QDemonRenderClearValues::Color);
        }
        renderToViewport();
    } else {
        // First, render the layer along with whatever progressive AA is appropriate.
        // The render graph should have taken care of the render to texture step.
#ifdef QDEMON_CACHED_POST_EFFECT
        QDemonRef<QDemonRenderTexture2D> theLayerColorTexture = (m_layerCachedTexture) ? m_layerCachedTexture : m_layerTexture;
#else
        // Then render all but the last effect
        IEffectSystem &theEffectSystem(m_Renderer.GetDemonContext().GetEffectSystem());
        IResourceManager &theResourceManager(m_Renderer.GetDemonContext().GetResourceManager());
        // we use the non MSAA buffer for the effect
        QDemonRenderTexture2D *theLayerColorTexture = m_LayerTexture;
        QDemonRenderTexture2D *theLayerDepthTexture = m_LayerDepthTexture;

        QDemonRenderTexture2D *theCurrentTexture = theLayerColorTexture;
        for (SEffect *theEffect = m_Layer.m_FirstEffect; theEffect && theEffect != thePrepResult.m_LastEffect;
             theEffect = theEffect->m_NextEffect) {
            if (theEffect->m_Flags.IsActive() && m_Camera) {
                StartProfiling(theEffect->m_ClassName, false);

                QDemonRenderTexture2D *theRenderedEffect = theEffectSystem.RenderEffect(
                        SEffectRenderArgument(*theEffect,
                                              *theCurrentTexture,
                                              QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar),
                                              theLayerDepthTexture,
                                              m_LayerPrepassDepthTexture));

                EndProfiling(theEffect->m_ClassName);

                // If the texture came from rendering a chain of effects, then we don't need it
                // after this.
                if (theCurrentTexture != theLayerColorTexture)
                    theResourceManager.Release(*theCurrentTexture);

                theCurrentTexture = theRenderedEffect;
            }
        }
#endif
        // Now the last effect or straight to the scene if we have no last effect
        // There are two cases we need to consider here.  The first is when we shouldn't
        // transform
        // the result and thus we need to setup an MVP that just maps to the viewport width and
        // height.
        // The second is when we are expected to render to the scene using some global
        // transform.
        QMatrix4x4 theFinalMVP;
        QDemonRenderCamera theTempCamera;
        QRect theLayerViewport(thePrepResult.getLayerToPresentationViewport().toRect());
        QRect theLayerClip(thePrepResult.getLayerToPresentationScissorRect().toRect());

        {
            QMatrix3x3 ignored;
            QMatrix4x4 theViewProjection;
            // We could cache these variables
            theTempCamera.flags.setOrthographic(true);
            theTempCamera.markDirty(NodeTransformDirtyFlag::TransformIsDirty);
            // Move the camera back far enough that we can see everything
            float theCameraSetback(10);
            // Attempt to ensure the layer can never be clipped.
            theTempCamera.position.setZ(-theCameraSetback);
            theTempCamera.clipFar = 2.0f * theCameraSetback;
            // Render the layer texture to the entire viewport.
            QDemonCameraGlobalCalculationResult
                    theResult = theTempCamera.calculateGlobalVariables(theLayerViewport,
                                                                       QVector2D((float)theLayerViewport.width(),
                                                                                 (float)theLayerViewport.height()));
            theTempCamera.calculateViewProjectionMatrix(theViewProjection);
            QDemonGraphNode theTempNode;
            theFinalMVP = theViewProjection;
            QDemonRenderBlendFunctionArgument blendFunc;
            QDemonRenderBlendEquationArgument blendEqu;

            switch (layer.blendType) {
            case LayerBlendTypes::Screen:
                blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::SrcAlpha,
                                                              QDemonRenderDstBlendFunc::One,
                                                              QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            case LayerBlendTypes::Multiply:
                blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::DstColor,
                                                              QDemonRenderDstBlendFunc::Zero,
                                                              QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            case LayerBlendTypes::Add:
                blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::One,
                                                              QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            case LayerBlendTypes::Subtract:
                blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::One,
                                                              QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::ReverseSubtract,
                                                             QDemonRenderBlendEquation::ReverseSubtract);
                break;
            case LayerBlendTypes::Overlay:
                // SW fallback doesn't use blend equation
                // note blend func is not used here anymore
                if (theContext->isAdvancedBlendHwSupported() || theContext->isAdvancedBlendHwSupportedKHR()) {
                    blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Overlay,
                                                                 QDemonRenderBlendEquation::Overlay);
                }
                break;
            case LayerBlendTypes::ColorBurn:
                // SW fallback doesn't use blend equation
                // note blend func is not used here anymore
                if (theContext->isAdvancedBlendHwSupported() || theContext->isAdvancedBlendHwSupportedKHR()) {
                    blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::ColorBurn,
                                                                 QDemonRenderBlendEquation::ColorBurn);
                }
                break;
            case LayerBlendTypes::ColorDodge:
                // SW fallback doesn't use blend equation
                // note blend func is not used here anymore
                if (theContext->isAdvancedBlendHwSupported() || theContext->isAdvancedBlendHwSupportedKHR()) {
                    blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::ColorDodge,
                                                                 QDemonRenderBlendEquation::ColorDodge);
                }
                break;
            default:
                blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                                                              QDemonRenderSrcBlendFunc::One,
                                                              QDemonRenderDstBlendFunc::OneMinusSrcAlpha);
                blendEqu = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            }
            theContext->setBlendFunction(blendFunc);
            theContext->setBlendEquation(blendEqu);
            theContext->setBlendingEnabled(blendingEnabled);
            theContext->setDepthTestEnabled(false);
        }

        {
            theContext->setScissorTestEnabled(true);
            theContext->setViewport(theLayerViewport);
            theContext->setScissorRect(theLayerClip);

            // Remember the camera we used so we can get a valid pick ray
            m_sceneCamera = theTempCamera;
            theContext->setDepthTestEnabled(false);
#ifndef QDEMON_CACHED_POST_EFFECT
            if (thePrepResult.m_LastEffect && m_Camera) {
                StartProfiling(thePrepResult.m_LastEffect->m_ClassName, false);
                // inUseLayerMPV is true then we are rendering directly to the scene and thus we
                // should enable blending
                // for the final render pass.  Else we should leave it.
                theEffectSystem.RenderEffect(SEffectRenderArgument(*thePrepResult.m_LastEffect,
                                                                   *theCurrentTexture,
                                                                   QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar),
                                                                   theLayerDepthTexture,
                                                                   m_LayerPrepassDepthTexture),
                                             theFinalMVP,
                                             blendingEnabled);
                EndProfiling(thePrepResult.m_LastEffect->m_ClassName);
                // If the texture came from rendering a chain of effects, then we don't need it
                // after this.
                if (theCurrentTexture != theLayerColorTexture)
                    theResourceManager.Release(*theCurrentTexture);
            } else
#endif
            {
                theContext->setCullingEnabled(false);
                theContext->setBlendingEnabled(blendingEnabled);
                theContext->setDepthTestEnabled(false);
#ifdef ADVANCED_BLEND_SW_FALLBACK
                QDemonRef<QDemonRenderTexture2D> screenTexture = renderer->getLayerBlendTexture();
                QDemonRef<QDemonRenderFrameBuffer> blendFB = renderer->getBlendFB();

                // Layer blending for advanced blending modes if SW fallback is needed
                // rendering to FBO and blending with separate shader
                if (screenTexture) {
                    // Blending is enabled only if layer background has been chosen transparent
                    // Layers with advanced blending modes
                    if (blendingEnabled
                        && (layer.blendType == LayerBlendTypes::Overlay || layer.blendType == LayerBlendTypes::ColorBurn
                            || layer.blendType == LayerBlendTypes::ColorDodge)) {
                        theContext->setScissorTestEnabled(false);
                        theContext->setBlendingEnabled(false);

                        // Get part matching to layer from screen texture and
                        // use that for blending
                        QDemonRef<QDemonRenderTexture2D> blendBlitTexture;
                        blendBlitTexture = theContext->createTexture2D();
                        blendBlitTexture->setTextureData(QDemonDataRef<quint8>(),
                                                         0,
                                                         theLayerViewport.width(),
                                                         theLayerViewport.height(),
                                                         QDemonRenderTextureFormat::RGBA8);
                        QDemonRef<QDemonRenderFrameBuffer> blitFB;
                        blitFB = theContext->createFrameBuffer();
                        blitFB->attach(QDemonRenderFrameBufferAttachment::Color0,
                                       QDemonRenderTextureOrRenderBuffer(blendBlitTexture));
                        blendFB->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer(screenTexture));
                        theContext->setRenderTarget(blitFB);
                        theContext->setReadTarget(blendFB);
                        theContext->setReadBuffer(QDemonReadFace::Color0);
                        theContext->blitFramebuffer(theLayerViewport.x(),
                                                    theLayerViewport.y(),
                                                    theLayerViewport.width() + theLayerViewport.x(),
                                                    theLayerViewport.height() + theLayerViewport.y(),
                                                    0,
                                                    0,
                                                    theLayerViewport.width(),
                                                    theLayerViewport.height(),
                                                    QDemonRenderClearValues::Color,
                                                    QDemonRenderTextureMagnifyingOp::Nearest);

                        QDemonRef<QDemonRenderTexture2D> blendResultTexture;
                        blendResultTexture = theContext->createTexture2D();
                        blendResultTexture->setTextureData(QDemonDataRef<quint8>(),
                                                           0,
                                                           theLayerViewport.width(),
                                                           theLayerViewport.height(),
                                                           QDemonRenderTextureFormat::RGBA8);
                        QDemonRef<QDemonRenderFrameBuffer> resultFB;
                        resultFB = theContext->createFrameBuffer();
                        resultFB->attach(QDemonRenderFrameBufferAttachment::Color0,
                                         QDemonRenderTextureOrRenderBuffer(blendResultTexture));
                        theContext->setRenderTarget(resultFB);

                        AdvancedBlendModes advancedMode;
                        switch (layer.blendType) {
                        case LayerBlendTypes::Overlay:
                            advancedMode = AdvancedBlendModes::Overlay;
                            break;
                        case LayerBlendTypes::ColorBurn:
                            advancedMode = AdvancedBlendModes::ColorBurn;
                            break;
                        case LayerBlendTypes::ColorDodge:
                            advancedMode = AdvancedBlendModes::ColorDodge;
                            break;
                        default:
                            advancedMode = AdvancedBlendModes::None;
                            break;
                        }

                        theContext->setViewport(QRect(0, 0, theLayerViewport.width(), theLayerViewport.height()));
                        blendAdvancedEquationSwFallback(theLayerColorTexture, blendBlitTexture, advancedMode);
                        // blitFB->release();
                        // save blending result to screen texture for use with other layers
                        theContext->setViewport(theLayerViewport);
                        theContext->setRenderTarget(blendFB);
                        renderer->renderQuad(QVector2D((float)theLayerViewport.width(), (float)theLayerViewport.height()),
                                             theFinalMVP,
                                             *blendResultTexture);
                        // render the blended result
                        theContext->setRenderTarget(theFB);
                        theContext->setScissorTestEnabled(true);
                        renderer->renderQuad(QVector2D((float)theLayerViewport.width(), (float)theLayerViewport.height()),
                                             theFinalMVP,
                                             *blendResultTexture);
                        // resultFB->release();
                    } else {
                        // Layers with normal blending modes
                        // save result for future use
                        theContext->setViewport(theLayerViewport);
                        theContext->setScissorTestEnabled(false);
                        theContext->setBlendingEnabled(true);
                        theContext->setRenderTarget(blendFB);
                        renderer->renderQuad(QVector2D((float)theLayerViewport.width(), (float)theLayerViewport.height()),
                                             theFinalMVP,
                                             *theLayerColorTexture);
                        theContext->setRenderTarget(theFB);
                        theContext->setScissorTestEnabled(true);
                        theContext->setViewport(theCurrentViewport);
                        renderer->renderQuad(QVector2D((float)theLayerViewport.width(), (float)theLayerViewport.height()),
                                             theFinalMVP,
                                             *theLayerColorTexture);
                    }
                } else {
                    // No advanced blending SW fallback needed
                    renderer->renderQuad(QVector2D((float)theLayerViewport.width(), (float)theLayerViewport.height()),
                                         theFinalMVP,
                                         *theLayerColorTexture);
                }
#else
                renderer->renderQuad(QVector2D((float)theLayerViewport.m_Width, (float)theLayerViewport.m_Height),
                                     theFinalMVP,
                                     *theLayerColorTexture);
#endif
            }
            if (m_layerWidgetTexture.getTexture()) {
                theContext->setBlendingEnabled(false);
                renderer->setupWidgetLayer();
                QDemonLayerRenderPreparationResult &thePrepResult(*layerPrepResult);
                QRectF thePresRect(thePrepResult.getPresentationViewport());
                QRectF theLayerRect(thePrepResult.getLayerToPresentationViewport());

                // Ensure we remove any offsetting in the layer rect that was caused simply by
                // the
                // presentation rect offsetting but then use a new rect.
                QRectF theWidgetLayerRect(theLayerRect.x() - thePresRect.x(),
                                          theLayerRect.y() - thePresRect.y(),
                                          theLayerRect.width(),
                                          theLayerRect.height());
                theContext->setScissorTestEnabled(false);
                theContext->setViewport(theWidgetLayerRect.toRect());
                renderer->renderQuad(QVector2D((float)theLayerViewport.width(), (float)theLayerViewport.height()),
                                     theFinalMVP,
                                     *m_layerWidgetTexture);
            }
        }
    } // End offscreen render code.

    if (m_boundingRectColor.hasValue()) {
        QDemonRenderContextScopedProperty<QRect> __viewport(*theContext, &QDemonRenderContext::getViewport, &QDemonRenderContext::setViewport);
        QDemonRenderContextScopedProperty<bool> theScissorEnabled(*theContext,
                                                                  &QDemonRenderContext::isScissorTestEnabled,
                                                                  &QDemonRenderContext::setScissorTestEnabled);
        QDemonRenderContextScopedProperty<QRect> theScissorRect(*theContext,
                                                                &QDemonRenderContext::getScissorRect,
                                                                &QDemonRenderContext::setScissorRect);
        renderer->setupWidgetLayer();
        // Setup a simple viewport to render to the entire presentation viewport.
        theContext->setViewport(QRect(0,
                                      0,
                                      (quint32)thePrepResult.getPresentationViewport().width(),
                                      (quint32)thePrepResult.getPresentationViewport().height()));

        QRectF thePresRect(thePrepResult.getPresentationViewport());

        // Remove any offsetting from the presentation rect since the widget layer is a
        // stand-alone fbo.
        QRectF theWidgetScreenRect(theScreenRect.x() - thePresRect.x(),
                                   theScreenRect.y() - thePresRect.y(),
                                   theScreenRect.width(),
                                   theScreenRect.height());
        theContext->setScissorTestEnabled(false);
        renderer->drawScreenRect(theWidgetScreenRect, *m_boundingRectColor);
    }
    theContext->setBlendFunction(QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::One,
                                                                   QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                                                                   QDemonRenderSrcBlendFunc::One,
                                                                   QDemonRenderDstBlendFunc::OneMinusSrcAlpha));
    theContext->setBlendEquation(QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add));
}

void QDemonLayerRenderData::addLayerRenderStep()
{
    QDemonStackPerfTimer __perfTimer(renderer->getDemonContext()->getPerfTimer(), Q_FUNC_INFO);
    Q_ASSERT(camera);
    if (!camera)
        return;

    QDemonRef<QDemonRenderListInterface> theGraph(renderer->getDemonContext()->getRenderList());

    QRect theCurrentViewport = theGraph->getViewport();
    if (!layerPrepResult.hasValue())
        prepareForRender(QSize(theCurrentViewport.width(), theCurrentViewport.height()));
}

void QDemonLayerRenderData::prepareForRender()
{
    // When we render to the scene itself (as opposed to an offscreen buffer somewhere)
    // then we use the MVP of the layer somewhat.
    QRect theViewport = renderer->getDemonContext()->getRenderList()->getViewport();
    prepareForRender(QSize((quint32)theViewport.width(), (quint32)theViewport.height()));
}

void QDemonLayerRenderData::resetForFrame()
{
    QDemonLayerRenderPreparationData::resetForFrame();
    m_boundingRectColor.setEmpty();
}

void QDemonLayerRenderData::prepareAndRender(const QMatrix4x4 &inViewProjection)
{
    TRenderableObjectList theTransparentObjects(transparentObjects);
    TRenderableObjectList theOpaqueObjects(opaqueObjects);
    theTransparentObjects.clear();
    theOpaqueObjects.clear();
    modelContexts.clear();
    QDemonLayerRenderPreparationResultFlags theFlags;
    prepareRenderablesForRender(inViewProjection, QDemonEmpty(), 1.0, theFlags);
    renderDepthPass(false);
    render();
}

struct QDemonLayerRenderToTextureRunnable : public QDemonRenderTask
{
    QDemonLayerRenderData &m_data;
    QDemonLayerRenderToTextureRunnable(QDemonLayerRenderData &d) : m_data(d) {}

    void run() override { m_data.renderToTexture(); }
};

static inline QDemonOffscreenRendererDepthValues getOffscreenRendererDepthValue(QDemonRenderTextureFormat inBufferFormat)
{
    switch (inBufferFormat.format) {
    case QDemonRenderTextureFormat::Depth32:
        return QDemonOffscreenRendererDepthValues::Depth32;
    case QDemonRenderTextureFormat::Depth24:
        return QDemonOffscreenRendererDepthValues::Depth24;
    case QDemonRenderTextureFormat::Depth24Stencil8:
        return QDemonOffscreenRendererDepthValues::Depth24;
    default:
        Q_ASSERT(false); // fallthrough intentional
    case QDemonRenderTextureFormat::Depth16:
        return QDemonOffscreenRendererDepthValues::Depth16;
    }
}

QDemonOffscreenRendererEnvironment QDemonLayerRenderData::createOffscreenRenderEnvironment()
{
    QDemonOffscreenRendererDepthValues theOffscreenDepth(getOffscreenRendererDepthValue(getDepthBufferFormat()));
    QRect theViewport = renderer->getDemonContext()->getRenderList()->getViewport();
    return QDemonOffscreenRendererEnvironment(theViewport.width(),
                                              theViewport.height(),
                                              QDemonRenderTextureFormat::RGBA8,
                                              theOffscreenDepth,
                                              false,
                                              AAModeValues::NoAA);
}

QDemonRef<QDemonRenderTask> QDemonLayerRenderData::createRenderToTextureRunnable()
{
    return QDemonRef<QDemonRenderTask>(new QDemonLayerRenderToTextureRunnable(*this));
}

QT_END_NAMESPACE
