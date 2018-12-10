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
#include <Qt3DSFoundation.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <qdemontextrenderer.h>
#include <qdemonrendereffectsystem.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <qdemonoffscreenrenderkey.h>
#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
#include <qdemonrenderplugingraphobject.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <Qt3DSPerfTimer.h>
#include <AutoDeallocatorAllocator.h>
#include <qdemonrendermaterialhelpers.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <qdemonrendercustommaterialsystem.h>
#include <qdemonrendertexttexturecache.h>
#include <qdemonrendertexttextureatlas.h>
#include <qdemonrenderrenderlist.h>
#include <qdemonrendererutil.h>

#ifdef WIN32
#pragma warning(disable : 4355)
#endif

#define QDEMON_CACHED_POST_EFFECT
const float QDEMON_DEGREES_TO_RADIANS = 0.0174532925199f;

QT_BEGIN_NAMESPACE
using eastl::reverse;
using eastl::stable_sort;
using QDemonRenderContextScopedProperty;
using QVector2D;

SLayerRenderData::SLayerRenderData(SLayer &inLayer, Qt3DSRendererImpl &inRenderer)
    : SLayerRenderPreparationData(inLayer, inRenderer)
    , m_LayerTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_TemporalAATexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerDepthTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerPrepassDepthTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerWidgetTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerSsaoTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerMultisampleTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerMultisamplePrepassDepthTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerMultisampleWidgetTexture(inRenderer.GetQt3DSContext().GetResourceManager())
    , m_LayerCachedTexture(nullptr)
    , m_AdvancedBlendDrawTexture(nullptr)
    , m_AdvancedBlendBlendTexture(nullptr)
    , m_AdvancedModeDrawFB(nullptr)
    , m_AdvancedModeBlendFB(nullptr)
    , m_ProgressiveAAPassIndex(0)
    , m_TemporalAAPassIndex(0)
    , m_NonDirtyTemporalAAPassIndex(0)
    , m_TextScale(1.0f)
    , m_DepthBufferFormat(QDemonRenderTextureFormats::Unknown)
{
}

SLayerRenderData::~SLayerRenderData()
{
    IResourceManager &theResourceManager(m_Renderer.GetQt3DSContext().GetResourceManager());
    if (m_LayerCachedTexture && m_LayerCachedTexture != m_LayerTexture)
        theResourceManager.Release(*m_LayerCachedTexture);
    if (m_AdvancedModeDrawFB) {
        //m_AdvancedModeDrawFB->release();
        m_AdvancedModeDrawFB = nullptr;
    }
    if (m_AdvancedModeBlendFB) {
        //m_AdvancedModeBlendFB->release();
        m_AdvancedModeBlendFB = nullptr;
    }
    if (m_AdvancedBlendBlendTexture)
        m_AdvancedBlendBlendTexture = nullptr;
    if (m_AdvancedBlendDrawTexture)
        m_AdvancedBlendDrawTexture = nullptr;
}
void SLayerRenderData::PrepareForRender(const QSize &inViewportDimensions)
{
    SLayerRenderPreparationData::PrepareForRender(inViewportDimensions);
    SLayerRenderPreparationResult &thePrepResult(*m_LayerPrepResult);
    IResourceManager &theResourceManager(m_Renderer.GetQt3DSContext().GetResourceManager());
    // at that time all values shoud be updated
    m_Renderer.UpdateCbAoShadow(&m_Layer, m_Camera, m_LayerDepthTexture);

    // Generate all necessary lighting keys

    if (thePrepResult.m_Flags.WasLayerDataDirty()) {
        m_ProgressiveAAPassIndex = 0;
    }

    // Get rid of the layer texture if we aren't rendering to texture this frame.
    if (m_LayerTexture && !thePrepResult.m_Flags.ShouldRenderToTexture()) {
        if (m_LayerCachedTexture && m_LayerCachedTexture != m_LayerTexture) {
            theResourceManager.Release(*m_LayerCachedTexture);
            m_LayerCachedTexture = nullptr;
        }

        m_LayerTexture.ReleaseTexture();
        m_LayerDepthTexture.ReleaseTexture();
        m_LayerWidgetTexture.ReleaseTexture();
        m_LayerSsaoTexture.ReleaseTexture();
        m_LayerMultisampleTexture.ReleaseTexture();
        m_LayerMultisamplePrepassDepthTexture.ReleaseTexture();
        m_LayerMultisampleWidgetTexture.ReleaseTexture();
    }

    if (NeedsWidgetTexture() == false)
        m_LayerWidgetTexture.ReleaseTexture();

    if (m_LayerDepthTexture && !thePrepResult.m_Flags.RequiresDepthTexture())
        m_LayerDepthTexture.ReleaseTexture();

    if (m_LayerSsaoTexture && !thePrepResult.m_Flags.RequiresSsaoPass())
        m_LayerSsaoTexture.ReleaseTexture();

    m_Renderer.LayerNeedsFrameClear(*this);

    // Clean up the texture cache if layer dimensions changed
    if (inViewportDimensions.width() != m_previousDimensions.width()
            || inViewportDimensions.height() != m_previousDimensions.height()) {
        m_LayerTexture.ReleaseTexture();
        m_LayerDepthTexture.ReleaseTexture();
        m_LayerSsaoTexture.ReleaseTexture();
        m_LayerWidgetTexture.ReleaseTexture();
        m_LayerPrepassDepthTexture.ReleaseTexture();
        m_TemporalAATexture.ReleaseTexture();
        m_LayerMultisampleTexture.ReleaseTexture();
        m_LayerMultisamplePrepassDepthTexture.ReleaseTexture();
        m_LayerMultisampleWidgetTexture.ReleaseTexture();

        m_previousDimensions.setWidth(inViewportDimensions.width());
        m_previousDimensions.setHeight(inViewportDimensions.height());

        theResourceManager.DestroyFreeSizedResources();

        // Effect system uses different resource manager, so clean that up too
        m_Renderer.GetQt3DSContext().GetEffectSystem().GetResourceManager()
                .DestroyFreeSizedResources();
    }
}

QDemonRenderTextureFormats::Enum SLayerRenderData::GetDepthBufferFormat()
{
    if (m_DepthBufferFormat == QDemonRenderTextureFormats::Unknown) {
        quint32 theExistingDepthBits = m_Renderer.GetContext().GetDepthBits();
        quint32 theExistingStencilBits = m_Renderer.GetContext().GetStencilBits();
        switch (theExistingDepthBits) {
        case 32:
            m_DepthBufferFormat = QDemonRenderTextureFormats::Depth32;
            break;
        case 24:
            //  check if we have stencil bits
            if (theExistingStencilBits > 0)
                m_DepthBufferFormat =
                        QDemonRenderTextureFormats::Depth24Stencil8; // currently no stencil usage
            // should be Depth24Stencil8 in
            // this case
            else
                m_DepthBufferFormat = QDemonRenderTextureFormats::Depth24;
            break;
        case 16:
            m_DepthBufferFormat = QDemonRenderTextureFormats::Depth16;
            break;
        default:
            Q_ASSERT(false);
            m_DepthBufferFormat = QDemonRenderTextureFormats::Depth16;
            break;
        }
    }
    return m_DepthBufferFormat;
}

QDemonRenderFrameBufferAttachments::Enum
SLayerRenderData::GetFramebufferDepthAttachmentFormat(QDemonRenderTextureFormats::Enum depthFormat)
{
    QDemonRenderFrameBufferAttachments::Enum fmt = QDemonRenderFrameBufferAttachments::Depth;

    switch (depthFormat) {
    case QDemonRenderTextureFormats::Depth16:
    case QDemonRenderTextureFormats::Depth24:
    case QDemonRenderTextureFormats::Depth32:
        fmt = QDemonRenderFrameBufferAttachments::Depth;
        break;
    case QDemonRenderTextureFormats::Depth24Stencil8:
        fmt = QDemonRenderFrameBufferAttachments::DepthStencil;
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    return fmt;
}

void SLayerRenderData::RenderAoPass()
{
    m_Renderer.BeginLayerDepthPassRender(*this);

    QDemonRenderContext &theContext(m_Renderer.GetContext());
    SDefaultAoPassShader *shader = m_Renderer.GetDefaultAoPassShader(GetShaderFeatureSet());
    if (shader == nullptr)
        return;

    // Set initial state
    theContext.SetBlendingEnabled(false);
    theContext.SetDepthWriteEnabled(false);
    theContext.SetDepthTestEnabled(false);
    theContext.SetActiveShader(&(shader->m_Shader));

    // Setup constants
    shader->m_CameraDirection.Set(m_CameraDirection);
    shader->m_ViewMatrix.Set(m_Camera->m_GlobalTransform);

    shader->m_DepthTexture.Set(m_LayerDepthTexture);
    shader->m_DepthSamplerSize.Set(QVector2D(m_LayerDepthTexture->GetTextureDetails().m_Width,
                                             m_LayerDepthTexture->GetTextureDetails().m_Height));

    // Important uniforms for AO calculations
    QVector2D theCameraProps = QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar);
    shader->m_CameraProperties.Set(theCameraProps);
    shader->m_AoShadowParams.Set();

    // Draw a fullscreen quad
    m_Renderer.RenderQuad();

    m_Renderer.EndLayerDepthPassRender();
}

void SLayerRenderData::RenderFakeDepthMapPass(QDemonRenderTexture2D *theDepthTex,
                                              QDemonRenderTextureCube *theDepthCube)
{
    m_Renderer.BeginLayerDepthPassRender(*this);

    QDemonRenderContext &theContext(m_Renderer.GetContext());
    SDefaultAoPassShader *shader = theDepthTex
            ? m_Renderer.GetFakeDepthShader(GetShaderFeatureSet())
            : m_Renderer.GetFakeCubeDepthShader(GetShaderFeatureSet());
    if (shader == nullptr)
        return;

    // Set initial state
    theContext.SetBlendingEnabled(false);
    theContext.SetDepthWriteEnabled(false);
    theContext.SetDepthTestEnabled(false);
    theContext.SetActiveShader(&(shader->m_Shader));

    // Setup constants
    shader->m_CameraDirection.Set(m_CameraDirection);
    shader->m_ViewMatrix.Set(m_Camera->m_GlobalTransform);

    shader->m_DepthTexture.Set(theDepthTex);
    shader->m_CubeTexture.Set(theDepthCube);
    shader->m_DepthSamplerSize.Set(QVector2D(theDepthTex->GetTextureDetails().m_Width,
                                             theDepthTex->GetTextureDetails().m_Height));

    // Important uniforms for AO calculations
    QVector2D theCameraProps = QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar);
    shader->m_CameraProperties.Set(theCameraProps);
    shader->m_AoShadowParams.Set();

    // Draw a fullscreen quad
    m_Renderer.RenderQuad();
}

namespace {

void computeFrustumBounds(const SCamera &inCamera, const QDemonRenderRectF &inViewPort,
                          QVector3D &ctrBound, QVector3D camVerts[8])
{
    QVector3D camEdges[4];

    const float *dataPtr(inCamera.m_GlobalTransform.front());
    QVector3D camX(dataPtr[0], dataPtr[1], dataPtr[2]);
    QVector3D camY(dataPtr[4], dataPtr[5], dataPtr[6]);
    QVector3D camZ(dataPtr[8], dataPtr[9], dataPtr[10]);

    float tanFOV = tanf(inCamera.verticalFov(inViewPort) * 0.5f);
    float asTanFOV = tanFOV * inViewPort.m_Width / inViewPort.m_Height;
    camEdges[0] = -asTanFOV * camX + tanFOV * camY + camZ;
    camEdges[1] = asTanFOV * camX + tanFOV * camY + camZ;
    camEdges[2] = asTanFOV * camX - tanFOV * camY + camZ;
    camEdges[3] = -asTanFOV * camX - tanFOV * camY + camZ;

    for (int i = 0; i < 4; ++i) {
        camEdges[i].x = -camEdges[i].x;
        camEdges[i].y = -camEdges[i].y;
    }

    camVerts[0] = inCamera.m_Position + camEdges[0] * inCamera.m_ClipNear;
    camVerts[1] = inCamera.m_Position + camEdges[0] * inCamera.m_ClipFar;
    camVerts[2] = inCamera.m_Position + camEdges[1] * inCamera.m_ClipNear;
    camVerts[3] = inCamera.m_Position + camEdges[1] * inCamera.m_ClipFar;
    camVerts[4] = inCamera.m_Position + camEdges[2] * inCamera.m_ClipNear;
    camVerts[5] = inCamera.m_Position + camEdges[2] * inCamera.m_ClipFar;
    camVerts[6] = inCamera.m_Position + camEdges[3] * inCamera.m_ClipNear;
    camVerts[7] = inCamera.m_Position + camEdges[3] * inCamera.m_ClipFar;

    ctrBound = camVerts[0];
    for (int i = 1; i < 8; ++i) {
        ctrBound += camVerts[i];
    }
    ctrBound *= 0.125f;
}

void SetupCameraForShadowMap(const QVector2D &inCameraVec, QDemonRenderContext & /*inContext*/,
                             const QDemonRenderRectF &inViewport, const SCamera &inCamera,
                             const SLight *inLight, SCamera &theCamera)
{
    // setup light matrix
    quint32 mapRes = 1 << inLight->m_ShadowMapRes;
    QDemonRenderRectF theViewport(0.0f, 0.0f, (float)mapRes, (float)mapRes);
    theCamera.m_ClipNear = 1.0f;
    theCamera.m_ClipFar = inLight->m_ShadowMapFar;
    // Setup camera projection
    QVector3D inLightPos = inLight->GetGlobalPos();
    QVector3D inLightDir = inLight->GetDirection();

    if (inLight->m_Flags.IsLeftHanded())
        inLightPos.z = -inLightPos.z;

    inLightPos -= inLightDir * inCamera.m_ClipNear;
    theCamera.m_FOV = inLight->m_ShadowMapFov * QDEMON_DEGREES_TO_RADIANS;

    if (inLight->m_LightType == RenderLightTypes::Directional) {
        QVector3D frustBounds[8], boundCtr;
        computeFrustumBounds(inCamera, inViewport, boundCtr, frustBounds);

        QVector3D forward = inLightDir;
        forward.normalize();
        QVector3D right = forward.cross(QVector3D(0, 1, 0));
        right.normalize();
        QVector3D up = right.cross(forward);
        up.normalize();

        // Calculate bounding box of the scene camera frustum
        float minDistanceZ = std::numeric_limits<float>::max();
        float maxDistanceZ = -std::numeric_limits<float>::max();
        float minDistanceY = std::numeric_limits<float>::max();
        float maxDistanceY = -std::numeric_limits<float>::max();
        float minDistanceX = std::numeric_limits<float>::max();
        float maxDistanceX = -std::numeric_limits<float>::max();
        for (int i = 0; i < 8; ++i) {
            float distanceZ = frustBounds[i].dot(forward);
            if (distanceZ < minDistanceZ)
                minDistanceZ = distanceZ;
            if (distanceZ > maxDistanceZ)
                maxDistanceZ = distanceZ;
            float distanceY = frustBounds[i].dot(up);
            if (distanceY < minDistanceY)
                minDistanceY = distanceY;
            if (distanceY > maxDistanceY)
                maxDistanceY = distanceY;
            float distanceX = frustBounds[i].dot(right);
            if (distanceX < minDistanceX)
                minDistanceX = distanceX;
            if (distanceX > maxDistanceX)
                maxDistanceX = distanceX;
        }

        // Apply bounding box parameters to shadow map camera projection matrix
        // so that the whole scene is fit inside the shadow map
        inLightPos = boundCtr;
        theViewport.m_Height = abs(maxDistanceY - minDistanceY);
        theViewport.m_Width = abs(maxDistanceX - minDistanceX);
        theCamera.m_ClipNear = -abs(maxDistanceZ - minDistanceZ);
        theCamera.m_ClipFar = abs(maxDistanceZ - minDistanceZ);
    }

    theCamera.m_Flags.SetLeftHanded(false);

    theCamera.m_Flags.ClearOrSet(inLight->m_LightType == RenderLightTypes::Directional,
                                 NodeFlagValues::Orthographic);
    theCamera.m_Parent = nullptr;
    theCamera.m_Pivot = inLight->m_Pivot;

    if (inLight->m_LightType != RenderLightTypes::Point) {
        theCamera.LookAt(inLightPos, QVector3D(0, 1.0, 0), inLightPos + inLightDir);
    } else {
        theCamera.LookAt(inLightPos, QVector3D(0, 1.0, 0), QVector3D(0, 0, 0));
    }

    theCamera.CalculateGlobalVariables(theViewport,
                                       QVector2D(theViewport.m_Width, theViewport.m_Height));
}
}

void SetupCubeShadowCameras(const SLight *inLight, SCamera inCameras[6])
{
    // setup light matrix
    quint32 mapRes = 1 << inLight->m_ShadowMapRes;
    QDemonRenderRectF theViewport(0.0f, 0.0f, (float)mapRes, (float)mapRes);
    QVector3D rotOfs[6];

    Q_ASSERT(inLight != nullptr);
    Q_ASSERT(inLight->m_LightType != RenderLightTypes::Directional);

    QVector3D inLightPos = inLight->GetGlobalPos();
    if (inLight->m_Flags.IsLeftHanded())
        inLightPos.z = -inLightPos.z;

    rotOfs[0] = QVector3D(0.f, -NVHalfPi, NVPi);
    rotOfs[1] = QVector3D(0.f, NVHalfPi, NVPi);
    rotOfs[2] = QVector3D(NVHalfPi, 0.f, 0.f);
    rotOfs[3] = QVector3D(-NVHalfPi, 0.f, 0.f);
    rotOfs[4] = QVector3D(0.f, NVPi, -NVPi);
    rotOfs[5] = QVector3D(0.f, 0.f, NVPi);

    for (int i = 0; i < 6; ++i) {
        inCameras[i].m_Flags.SetLeftHanded(false);

        inCameras[i].m_Flags.ClearOrSet(false, NodeFlagValues::Orthographic);
        inCameras[i].m_Parent = nullptr;
        inCameras[i].m_Pivot = inLight->m_Pivot;
        inCameras[i].m_ClipNear = 1.0f;
        inCameras[i].m_ClipFar = NVMax<float>(2.0f, inLight->m_ShadowMapFar);
        inCameras[i].m_FOV = inLight->m_ShadowMapFov * QDEMON_DEGREES_TO_RADIANS;

        inCameras[i].m_Position = inLightPos;
        inCameras[i].m_Rotation = rotOfs[i];
        inCameras[i].CalculateGlobalVariables(
                    theViewport, QVector2D(theViewport.m_Width, theViewport.m_Height));
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

inline void RenderRenderableShadowMapPass(SLayerRenderData &inData, SRenderableObject &inObject,
                                          const QVector2D &inCameraProps, TShaderFeatureSet,
                                          quint32 lightIndex, const SCamera &inCamera)
{
    SShadowMapEntry *pEntry = inData.m_ShadowMapManager->GetShadowMapEntry(lightIndex);

    if (inObject.m_RenderableFlags.IsDefaultMaterialMeshSubset())
        static_cast<SSubsetRenderableBase &>(inObject).RenderShadowMapPass(
                inCameraProps, inData.m_Lights[lightIndex], inCamera, pEntry);
    else if (inObject.m_RenderableFlags.IsCustomMaterialMeshSubset()) {
        static_cast<SSubsetRenderableBase &>(inObject).RenderShadowMapPass(
                    inCameraProps, inData.m_Lights[lightIndex], inCamera, pEntry);
    } else if (inObject.m_RenderableFlags.IsPath()) {
        static_cast<SPathRenderable &>(inObject).RenderShadowMapPass(
                    inCameraProps, inData.m_Lights[lightIndex], inCamera, pEntry);
    }
}

void SLayerRenderData::RenderShadowCubeBlurPass(CResourceFrameBuffer *theFB,
                                                QDemonRenderTextureCube *target0,
                                                QDemonRenderTextureCube *target1, float filterSz,
                                                float clipFar)
{
    QDemonRenderContext &theContext(m_Renderer.GetContext());

    SShadowmapPreblurShader *shaderX = m_Renderer.GetCubeShadowBlurXShader();
    SShadowmapPreblurShader *shaderY = m_Renderer.GetCubeShadowBlurYShader();

    if (shaderX == nullptr)
        return;
    if (shaderY == nullptr)
        return;
    // if ( theShader == nullptr ) return;

    // Enable drawing to 6 color attachment buffers for cubemap passes
    qint32 buffers[6] = { 0, 1, 2, 3, 4, 5 };
    QDemonConstDataRef<qint32> bufferList(buffers, 6);
    theContext.SetDrawBuffers(bufferList);

    // Attach framebuffer targets
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color0, *target1,
                         QDemonRenderTextureCubeFaces::CubePosX);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color1, *target1,
                         QDemonRenderTextureCubeFaces::CubeNegX);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color2, *target1,
                         QDemonRenderTextureCubeFaces::CubePosY);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color3, *target1,
                         QDemonRenderTextureCubeFaces::CubeNegY);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color4, *target1,
                         QDemonRenderTextureCubeFaces::CubePosZ);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color5, *target1,
                         QDemonRenderTextureCubeFaces::CubeNegZ);

    // Set initial state
    theContext.SetBlendingEnabled(false);
    theContext.SetDepthWriteEnabled(false);
    theContext.SetDepthTestEnabled(false);
    // theContext.SetColorWritesEnabled(true);
    theContext.SetActiveShader(&(shaderX->m_Shader));

    shaderX->m_CameraProperties.Set(QVector2D(filterSz, clipFar));
    shaderX->m_DepthCube.Set(target0);

    // Draw a fullscreen quad
    m_Renderer.RenderQuad();

    theContext.SetActiveShader(&(shaderY->m_Shader));

    // Lather, Rinse, and Repeat for the Y-blur pass
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color0, *target0,
                         QDemonRenderTextureCubeFaces::CubePosX);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color1, *target0,
                         QDemonRenderTextureCubeFaces::CubeNegX);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color2, *target0,
                         QDemonRenderTextureCubeFaces::CubePosY);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color3, *target0,
                         QDemonRenderTextureCubeFaces::CubeNegY);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color4, *target0,
                         QDemonRenderTextureCubeFaces::CubePosZ);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color5, *target0,
                         QDemonRenderTextureCubeFaces::CubeNegZ);

    shaderY->m_CameraProperties.Set(QVector2D(filterSz, clipFar));
    shaderY->m_DepthCube.Set(target1);

    // Draw a fullscreen quad
    m_Renderer.RenderQuad();

    theContext.SetDepthWriteEnabled(true);
    theContext.SetDepthTestEnabled(true);
    // theContext.SetColorWritesEnabled(false);

    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFaces::CubePosX);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color1, QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFaces::CubeNegX);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color2, QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFaces::CubePosY);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color3, QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFaces::CubeNegY);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color4, QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFaces::CubePosZ);
    (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color5, QDemonRenderTextureOrRenderBuffer(),
                         QDemonRenderTextureCubeFaces::CubeNegZ);

    theContext.SetDrawBuffers(toConstDataRef((qint32)0));
}

void SLayerRenderData::RenderShadowMapBlurPass(CResourceFrameBuffer *theFB,
                                               QDemonRenderTexture2D *target0,
                                               QDemonRenderTexture2D *target1, float filterSz,
                                               float clipFar)
{
    QDemonRenderContext &theContext(m_Renderer.GetContext());

    SShadowmapPreblurShader *shaderX = m_Renderer.GetOrthoShadowBlurXShader();
    SShadowmapPreblurShader *shaderY = m_Renderer.GetOrthoShadowBlurYShader();

    if (shaderX == nullptr)
        return;
    if (shaderY == nullptr)
        return;

    // Attach framebuffer target
    (*theFB)->Attach(QDemonRenderFrameBufferAttachments::Color0, *target1);
    //(*theFB)->Attach( QDemonRenderFrameBufferAttachments::DepthStencil, *target1 );

    // Set initial state
    theContext.SetBlendingEnabled(false);
    theContext.SetDepthWriteEnabled(false);
    theContext.SetDepthTestEnabled(false);
    theContext.SetColorWritesEnabled(true);
    theContext.SetActiveShader(&(shaderX->m_Shader));

    shaderX->m_CameraProperties.Set(QVector2D(filterSz, clipFar));
    shaderX->m_DepthMap.Set(target0);

    // Draw a fullscreen quad
    m_Renderer.RenderQuad();

    (*theFB)->Attach(QDemonRenderFrameBufferAttachments::Color0, *target0);
    //(*theFB)->Attach( QDemonRenderFrameBufferAttachments::DepthStencil, *target0 );
    theContext.SetActiveShader(&(shaderY->m_Shader));

    shaderY->m_CameraProperties.Set(QVector2D(filterSz, clipFar));
    shaderY->m_DepthMap.Set(target1);

    // Draw a fullscreen quad
    m_Renderer.RenderQuad();

    theContext.SetDepthWriteEnabled(true);
    theContext.SetDepthTestEnabled(true);
    theContext.SetColorWritesEnabled(false);

    //(*theFB)->Attach( QDemonRenderFrameBufferAttachments::DepthStencil,
    //QDemonRenderTextureOrRenderBuffer() );
    (*theFB)->Attach(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer());
}

void SLayerRenderData::RenderShadowMapPass(CResourceFrameBuffer *theFB)
{
    SStackPerfTimer ___timer(m_Renderer.GetQt3DSContext().GetPerfTimer(),
                             "SLayerRenderData::RenderShadowMapPass");

    if (m_Camera == nullptr || !GetShadowMapManager())
        return;

    // Check if we have anything to render
    if (m_OpaqueObjects.size() == 0 || m_Lights.size() == 0)
        return;

    m_Renderer.BeginLayerDepthPassRender(*this);

    QDemonRenderContext &theRenderContext(m_Renderer.GetContext());

    // we may change the viewport
    QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                theRenderContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);

    // disable color writes
    // theRenderContext.SetColorWritesEnabled( false );
    theRenderContext.SetColorWritesEnabled(true);
    theRenderContext.SetDepthWriteEnabled(true);
    theRenderContext.SetCullingEnabled(false);
    theRenderContext.SetClearColor(QVector4D(1.0f));

    // we render the shadow map with a slight offset to prevent shadow acne and cull the front
    // faces
    QDemonScopedRefCounted<QDemonRenderRasterizerState> rsdefaultstate =
            theRenderContext.CreateRasterizerState(0.0, 0.0, QDemonRenderFaces::Back);
    QDemonScopedRefCounted<QDemonRenderRasterizerState> rsstate =
            theRenderContext.CreateRasterizerState(1.5, 2.0, QDemonRenderFaces::Front);
    theRenderContext.SetRasterizerState(rsstate);

    QDemonRenderClearFlags clearFlags(QDemonRenderClearValues::Depth
                                      | QDemonRenderClearValues::Stencil
                                      | QDemonRenderClearValues::Color);

    for (quint32 i = 0; i < m_Lights.size(); i++) {
        // don't render shadows when not casting
        if (m_Lights[i]->m_CastShadow == false)
            continue;
        SShadowMapEntry *pEntry = m_ShadowMapManager->GetShadowMapEntry(i);
        if (pEntry && pEntry->m_DepthMap && pEntry->m_DepthCopy && pEntry->m_DepthRender) {
            SCamera theCamera;

            QVector2D theCameraProps = QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar);
            SetupCameraForShadowMap(theCameraProps, m_Renderer.GetContext(),
                                    __viewport.m_InitialValue, *m_Camera,
                                    m_Lights[i], theCamera);
            // we need this matrix for the final rendering
            theCamera.CalculateViewProjectionMatrix(pEntry->m_LightVP);
            pEntry->m_LightView = theCamera.m_GlobalTransform.getInverse();

            STextureDetails theDetails(pEntry->m_DepthMap->GetTextureDetails());
            theRenderContext.SetViewport(
                        QDemonRenderRect(0, 0, (quint32)theDetails.m_Width, (quint32)theDetails.m_Height));

            (*theFB)->Attach(QDemonRenderFrameBufferAttachments::Color0, *pEntry->m_DepthMap);
            (*theFB)->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                             *pEntry->m_DepthRender);
            theRenderContext.Clear(clearFlags);

            RunRenderPass(RenderRenderableShadowMapPass, false, true, true, i, theCamera);
            RenderShadowMapBlurPass(theFB, pEntry->m_DepthMap, pEntry->m_DepthCopy,
                                    m_Lights[i]->m_ShadowFilter, m_Lights[i]->m_ShadowMapFar);
        } else if (pEntry && pEntry->m_DepthCube && pEntry->m_CubeCopy
                   && pEntry->m_DepthRender) {
            SCamera theCameras[6];

            SetupCubeShadowCameras(m_Lights[i], theCameras);

            // pEntry->m_LightView = m_Lights[i]->m_LightType == RenderLightTypes::Point ?
            // QMatrix4x4::createIdentity()
            //	: m_Lights[i]->m_GlobalTransform;
            pEntry->m_LightView = QMatrix4x4::createIdentity();

            STextureDetails theDetails(pEntry->m_DepthCube->GetTextureDetails());
            theRenderContext.SetViewport(
                        QDemonRenderRect(0, 0, (quint32)theDetails.m_Width, (quint32)theDetails.m_Height));

            // int passes = m_Lights[i]->m_LightType == RenderLightTypes::Point ? 6 : 5;
            int passes = 6;
            for (int k = 0; k < passes; ++k) {
                // theCameras[k].CalculateViewProjectionMatrix( pEntry->m_LightCubeVP[k] );
                pEntry->m_LightCubeView[k] = theCameras[k].m_GlobalTransform.getInverse();
                theCameras[k].CalculateViewProjectionMatrix(pEntry->m_LightVP);

                // Geometry shader multiplication really doesn't work unless you have a
                // 6-layered 3D depth texture...
                // Otherwise, you have no way to depth test while rendering...
                // which more or less completely defeats the purpose of having a cubemap render
                // target.
                QDemonRenderTextureCubeFaces::Enum curFace =
                        (QDemonRenderTextureCubeFaces::Enum)(k + 1);
                //(*theFB)->AttachFace( QDemonRenderFrameBufferAttachments::DepthStencil,
                //*pEntry->m_DepthCube, curFace );
                (*theFB)->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 *pEntry->m_DepthRender);
                (*theFB)->AttachFace(QDemonRenderFrameBufferAttachments::Color0,
                                     *pEntry->m_DepthCube, curFace);
                (*theFB)->IsComplete();
                theRenderContext.Clear(clearFlags);

                RunRenderPass(RenderRenderableShadowMapPass, false, true, true, i,
                              theCameras[k]);
            }

            RenderShadowCubeBlurPass(theFB, pEntry->m_DepthCube, pEntry->m_CubeCopy,
                                     m_Lights[i]->m_ShadowFilter, m_Lights[i]->m_ShadowMapFar);
        }
    }

    (*theFB)->Attach(QDemonRenderFrameBufferAttachments::Depth, QDemonRenderTextureOrRenderBuffer());
    (*theFB)->Attach(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer());

    // enable color writes
    theRenderContext.SetColorWritesEnabled(true);
    theRenderContext.SetCullingEnabled(true);
    theRenderContext.SetClearColor(QVector4D(0.0f));
    // reset rasterizer state
    theRenderContext.SetRasterizerState(rsdefaultstate);

    m_Renderer.EndLayerDepthPassRender();
}

inline void RenderRenderableDepthPass(SLayerRenderData &inData, SRenderableObject &inObject,
                                      const QVector2D &inCameraProps, TShaderFeatureSet, quint32,
                                      const SCamera &inCamera)
{
    if (inObject.m_RenderableFlags.IsDefaultMaterialMeshSubset())
        static_cast<SSubsetRenderable &>(inObject).RenderDepthPass(inCameraProps);
    else if (inObject.m_RenderableFlags.IsText())
        static_cast<STextRenderable &>(inObject).RenderDepthPass(inCameraProps);
    else if (inObject.m_RenderableFlags.IsCustomMaterialMeshSubset()) {
        static_cast<SCustomMaterialRenderable &>(inObject).RenderDepthPass(
                    inCameraProps, inData.m_Layer, inData.m_Lights, inCamera, nullptr);
    } else if (inObject.m_RenderableFlags.IsPath()) {
        static_cast<SPathRenderable &>(inObject).RenderDepthPass(
                    inCameraProps, inData.m_Layer, inData.m_Lights, inCamera, nullptr);
    } else {
        Q_ASSERT(false);
    }
}

void SLayerRenderData::RenderDepthPass(bool inEnableTransparentDepthWrite)
{
    SStackPerfTimer ___timer(m_Renderer.GetQt3DSContext().GetPerfTimer(),
                             "SLayerRenderData::RenderDepthPass");
    if (m_Camera == nullptr)
        return;

    // Avoid running this method if possible.
    if ((inEnableTransparentDepthWrite == false
         && (m_OpaqueObjects.size() == 0
             || m_Layer.m_Flags.IsLayerEnableDepthPrepass() == false))
            || m_Layer.m_Flags.IsLayerEnableDepthTest() == false)
        return;

    m_Renderer.BeginLayerDepthPassRender(*this);

    QDemonRenderContext &theRenderContext(m_Renderer.GetContext());

    // disable color writes
    theRenderContext.SetColorWritesEnabled(false);
    theRenderContext.SetDepthWriteEnabled(true);

    QDemonRenderClearFlags clearFlags(QDemonRenderClearValues::Stencil
                                      | QDemonRenderClearValues::Depth);
    theRenderContext.Clear(clearFlags);

    RunRenderPass(RenderRenderableDepthPass, false, true, inEnableTransparentDepthWrite, 0,
                  *m_Camera);

    // enable color writes
    theRenderContext.SetColorWritesEnabled(true);

    m_Renderer.EndLayerDepthPassRender();
}

inline void RenderRenderable(SLayerRenderData &inData, SRenderableObject &inObject,
                             const QVector2D &inCameraProps, TShaderFeatureSet inFeatureSet, quint32,
                             const SCamera &inCamera)
{
    if (inObject.m_RenderableFlags.IsDefaultMaterialMeshSubset())
        static_cast<SSubsetRenderable &>(inObject).Render(inCameraProps, inFeatureSet);
    else if (inObject.m_RenderableFlags.IsText())
        static_cast<STextRenderable &>(inObject).Render(inCameraProps);
    else if (inObject.m_RenderableFlags.IsCustomMaterialMeshSubset()) {
        // PKC : Need a better place to do this.
        SCustomMaterialRenderable &theObject =
                static_cast<SCustomMaterialRenderable &>(inObject);
        if (!inData.m_Layer.m_LightProbe && theObject.m_Material.m_IblProbe)
            inData.SetShaderFeature("QDEMON_ENABLE_LIGHT_PROBE",
                                    theObject.m_Material.m_IblProbe->m_TextureData.m_Texture
                                    != nullptr);
        else if (inData.m_Layer.m_LightProbe)
            inData.SetShaderFeature("QDEMON_ENABLE_LIGHT_PROBE",
                                    inData.m_Layer.m_LightProbe->m_TextureData.m_Texture
                                    != nullptr);

        static_cast<SCustomMaterialRenderable &>(inObject).Render(
                    inCameraProps, inData, inData.m_Layer, inData.m_Lights, inCamera,
                    inData.m_LayerDepthTexture, inData.m_LayerSsaoTexture, inFeatureSet);
    } else if (inObject.m_RenderableFlags.IsPath()) {
        static_cast<SPathRenderable &>(inObject).Render(
                    inCameraProps, inData.m_Layer, inData.m_Lights, inCamera,
                    inData.m_LayerDepthTexture, inData.m_LayerSsaoTexture, inFeatureSet);
    } else {
        Q_ASSERT(false);
    }
}

void SLayerRenderData::RunRenderPass(TRenderRenderableFunction inRenderFn,
                                     bool inEnableBlending, bool inEnableDepthWrite,
                                     bool inEnableTransparentDepthWrite, quint32 indexLight,
                                     const SCamera &inCamera, CResourceFrameBuffer *theFB)
{
    QDemonRenderContext &theRenderContext(m_Renderer.GetContext());
    theRenderContext.SetDepthFunction(QDemonRenderBoolOp::LessThanOrEqual);
    theRenderContext.SetBlendingEnabled(false);
    QVector2D theCameraProps = QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar);
    QDemonDataRef<SRenderableObject *> theOpaqueObjects = GetOpaqueRenderableObjects();
    bool usingDepthBuffer =
            m_Layer.m_Flags.IsLayerEnableDepthTest() && theOpaqueObjects.size() > 0;

    if (usingDepthBuffer) {
        theRenderContext.SetDepthTestEnabled(true);
        theRenderContext.SetDepthWriteEnabled(inEnableDepthWrite);
    } else {
        theRenderContext.SetDepthWriteEnabled(false);
        theRenderContext.SetDepthTestEnabled(false);
    }

    for (quint32 idx = 0, end = theOpaqueObjects.size(); idx < end; ++idx) {
        SRenderableObject &theObject(*theOpaqueObjects[idx]);
        SScopedLightsListScope lightsScope(m_Lights, m_LightDirections, m_SourceLightDirections,
                                           theObject.m_ScopedLights);
        SetShaderFeature(m_CGLightingFeatureName, m_Lights.empty() == false);
        inRenderFn(*this, theObject, theCameraProps, GetShaderFeatureSet(), indexLight,
                   inCamera);
    }

    // transparent objects
    if (inEnableBlending || m_Layer.m_Flags.IsLayerEnableDepthTest() == false) {
        theRenderContext.SetBlendingEnabled(true && inEnableBlending);
        theRenderContext.SetDepthWriteEnabled(inEnableTransparentDepthWrite);

        QDemonDataRef<SRenderableObject *> theTransparentObjects = GetTransparentRenderableObjects();
        // Assume all objects have transparency if the layer's depth test enabled flag is true.
        if (m_Layer.m_Flags.IsLayerEnableDepthTest() == true) {
            for (quint32 idx = 0, end = theTransparentObjects.size(); idx < end; ++idx) {
                SRenderableObject &theObject(*theTransparentObjects[idx]);
                if (!(theObject.m_RenderableFlags.IsCompletelyTransparent())) {
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    // SW fallback for advanced blend modes.
                    // Renders transparent objects to a separate FBO and blends them in shader
                    // with the opaque items and background.
                    DefaultMaterialBlendMode::Enum blendMode
                            = DefaultMaterialBlendMode::Enum::Normal;
                    if (theObject.m_RenderableFlags.IsDefaultMaterialMeshSubset())
                        blendMode = static_cast<SSubsetRenderable &>(theObject).getBlendingMode();
                    bool useBlendFallback = (blendMode == DefaultMaterialBlendMode::Overlay ||
                                             blendMode == DefaultMaterialBlendMode::ColorBurn ||
                                             blendMode == DefaultMaterialBlendMode::ColorDodge) &&
                            !theRenderContext.IsAdvancedBlendHwSupported() &&
                            !theRenderContext.IsAdvancedBlendHwSupportedKHR() &&
                            m_LayerPrepassDepthTexture;
                    if (useBlendFallback)
                        SetupDrawFB(true);
#endif
                    SScopedLightsListScope lightsScope(m_Lights, m_LightDirections,
                                                       m_SourceLightDirections,
                                                       theObject.m_ScopedLights);
                    SetShaderFeature(m_CGLightingFeatureName, m_Lights.empty() == false);

                    inRenderFn(*this, theObject, theCameraProps, GetShaderFeatureSet(),
                               indexLight, inCamera);
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    // SW fallback for advanced blend modes.
                    // Continue blending after transparent objects have been rendered to a FBO
                    if (useBlendFallback) {
                        BlendAdvancedToFB(blendMode, true, theFB);
                        // restore blending status
                        theRenderContext.SetBlendingEnabled(inEnableBlending);
                        // restore depth test status
                        theRenderContext.SetDepthTestEnabled(usingDepthBuffer);
                        theRenderContext.SetDepthWriteEnabled(inEnableTransparentDepthWrite);
                    }
#endif
                }
            }
        }
        // If the layer doesn't have depth enabled then we have to render via an alternate route
        // where the transparent objects vector could have both opaque and transparent objects.
        else {
            for (quint32 idx = 0, end = theTransparentObjects.size(); idx < end; ++idx) {
                SRenderableObject &theObject(*theTransparentObjects[idx]);
                if (!(theObject.m_RenderableFlags.IsCompletelyTransparent())) {
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    DefaultMaterialBlendMode::Enum blendMode
                            = DefaultMaterialBlendMode::Enum::Normal;
                    if (theObject.m_RenderableFlags.IsDefaultMaterialMeshSubset())
                        blendMode = static_cast<SSubsetRenderable &>(theObject).getBlendingMode();
                    bool useBlendFallback = (blendMode == DefaultMaterialBlendMode::Overlay ||
                                             blendMode == DefaultMaterialBlendMode::ColorBurn ||
                                             blendMode == DefaultMaterialBlendMode::ColorDodge) &&
                            !theRenderContext.IsAdvancedBlendHwSupported() &&
                            !theRenderContext.IsAdvancedBlendHwSupportedKHR();

                    if (theObject.m_RenderableFlags.HasTransparency()) {
                        theRenderContext.SetBlendingEnabled(true && inEnableBlending);
                        // If we have SW fallback for blend mode, render to a FBO and blend back.
                        // Slow as this must be done per-object (transparent and opaque items
                        // are mixed, not batched)
                        if (useBlendFallback)
                            SetupDrawFB(false);
                    }
#endif
                    SScopedLightsListScope lightsScope(m_Lights, m_LightDirections,
                                                       m_SourceLightDirections,
                                                       theObject.m_ScopedLights);
                    SetShaderFeature(m_CGLightingFeatureName, m_Lights.empty() == false);
                    inRenderFn(*this, theObject, theCameraProps, GetShaderFeatureSet(),
                               indexLight, inCamera);
#ifdef ADVANCED_BLEND_SW_FALLBACK
                    if (useBlendFallback) {
                        BlendAdvancedToFB(blendMode, false, theFB);
                        // restore blending status
                        theRenderContext.SetBlendingEnabled(inEnableBlending);

                    }
#endif
                }
            }
        }
    }
}

void SLayerRenderData::Render(CResourceFrameBuffer *theFB)
{
    SStackPerfTimer ___timer(m_Renderer.GetQt3DSContext().GetPerfTimer(),
                             "SLayerRenderData::Render");
    if (m_Camera == nullptr)
        return;

    m_Renderer.BeginLayerRender(*this);
    RunRenderPass(RenderRenderable, true, !m_Layer.m_Flags.IsLayerEnableDepthPrepass(), false,
                  0, *m_Camera, theFB);
    m_Renderer.EndLayerRender();
}

void SLayerRenderData::CreateGpuProfiler()
{
    if (m_Renderer.GetContext().IsTimerQuerySupported()) {
        m_LayerProfilerGpu = IRenderProfiler::CreateGpuProfiler(
                    m_Renderer.GetContext().GetFoundation(), m_Renderer.GetQt3DSContext(),
                    m_Renderer.GetContext());
    }
}

void SLayerRenderData::StartProfiling(QString &nameID, bool sync)
{
    if (m_LayerProfilerGpu.mPtr) {
        m_LayerProfilerGpu->StartTimer(nameID, false, sync);
    }
}

void SLayerRenderData::EndProfiling(QString &nameID)
{
    if (m_LayerProfilerGpu.mPtr) {
        m_LayerProfilerGpu->EndTimer(nameID);
    }
}

void SLayerRenderData::StartProfiling(const char *nameID, bool sync)
{
    if (m_LayerProfilerGpu.mPtr) {
        QString theStr(QString::fromLocal8Bit(nameID));
        m_LayerProfilerGpu->StartTimer(theStr, false, sync);
    }
}

void SLayerRenderData::EndProfiling(const char *nameID)
{
    if (m_LayerProfilerGpu.mPtr) {
        QString theStr(QString::fromLocal8Bit(nameID));
        m_LayerProfilerGpu->EndTimer(theStr);
    }
}

void SLayerRenderData::AddVertexCount(quint32 count)
{
    if (m_LayerProfilerGpu.mPtr) {
        m_LayerProfilerGpu->AddVertexCount(count);
    }
}

// Assumes the viewport is setup appropriately to render the widget.
void SLayerRenderData::RenderRenderWidgets()
{
    if (m_Camera) {
        QDemonRenderContext &theContext(m_Renderer.GetContext());
        for (quint32 idx = 0, end = m_IRenderWidgets.size(); idx < end; ++idx) {
            IRenderWidget &theWidget = *m_IRenderWidgets[idx];
            theWidget.Render(m_Renderer, theContext);
        }
    }
}

#ifdef ADVANCED_BLEND_SW_FALLBACK
void SLayerRenderData::BlendAdvancedEquationSwFallback(QDemonRenderTexture2D *drawTexture,
                                                       QDemonRenderTexture2D *layerTexture,
                                                       AdvancedBlendModes::Enum blendMode)
{
    QDemonRenderContext &theContext(m_Renderer.GetContext());
    SAdvancedModeBlendShader *shader = m_Renderer.GetAdvancedBlendModeShader(blendMode);
    if (shader == nullptr)
        return;

    theContext.SetActiveShader(&(shader->m_Shader));

    shader->m_baseLayer.Set(layerTexture);
    shader->m_blendLayer.Set(drawTexture);
    // Draw a fullscreen quad
    m_Renderer.RenderQuad();
}

void SLayerRenderData::SetupDrawFB(bool depthEnabled)
{
    QDemonRenderContext &theRenderContext(m_Renderer.GetContext());
    // create drawing FBO and texture, if not existing
    if (!m_AdvancedModeDrawFB)
        m_AdvancedModeDrawFB = theRenderContext.CreateFrameBuffer();
    if (!m_AdvancedBlendDrawTexture) {
        m_AdvancedBlendDrawTexture = theRenderContext.CreateTexture2D();
        QDemonRenderRect theViewport = m_Renderer.GetQt3DSContext().GetRenderList().GetViewport();
        m_AdvancedBlendDrawTexture->SetTextureData(QDemonDataRef<quint8>(), 0,
                                                   theViewport.m_Width,
                                                   theViewport.m_Height,
                                                   QDemonRenderTextureFormats::RGBA8);
        m_AdvancedModeDrawFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                     *m_AdvancedBlendDrawTexture);
        // Use existing depth prepass information when rendering transparent objects to a FBO
        if (depthEnabled)
            m_AdvancedModeDrawFB->Attach(QDemonRenderFrameBufferAttachments::Depth,
                                         *m_LayerPrepassDepthTexture);
    }
    theRenderContext.SetRenderTarget(m_AdvancedModeDrawFB);
    // make sure that depth testing is on in order to render just the
    // depth-passed objects (=transparent objects) and leave background intact
    if (depthEnabled)
        theRenderContext.SetDepthTestEnabled(true);
    theRenderContext.SetBlendingEnabled(false);
    // clear color commonly is the layer background, make sure that it is all-zero here
    QVector4D originalClrColor = theRenderContext.GetClearColor();
    theRenderContext.SetClearColor(QVector4D(0.0));
    theRenderContext.Clear(QDemonRenderClearValues::Color);
    theRenderContext.SetClearColor(originalClrColor);

}
void SLayerRenderData::BlendAdvancedToFB(DefaultMaterialBlendMode::Enum blendMode,
                                         bool depthEnabled, CResourceFrameBuffer *theFB)
{
    QDemonRenderContext &theRenderContext(m_Renderer.GetContext());
    QDemonRenderRect theViewport = m_Renderer.GetQt3DSContext().GetRenderList().GetViewport();
    AdvancedBlendModes::Enum advancedMode;

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
    if (!m_AdvancedModeBlendFB)
        m_AdvancedModeBlendFB = theRenderContext.CreateFrameBuffer();
    if (!m_AdvancedBlendBlendTexture) {
        m_AdvancedBlendBlendTexture = theRenderContext.CreateTexture2D();
        m_AdvancedBlendBlendTexture->SetTextureData(QDemonDataRef<quint8>(), 0,
                                                    theViewport.m_Width,
                                                    theViewport.m_Height,
                                                    QDemonRenderTextureFormats::RGBA8);
        m_AdvancedModeBlendFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                      *m_AdvancedBlendBlendTexture);
    }
    theRenderContext.SetRenderTarget(m_AdvancedModeBlendFB);

    // Blend transparent objects with SW fallback shaders.
    // Disable depth testing as transparent objects have already been
    // depth-checked; here we want to run shader for all layer pixels
    if (depthEnabled)
    {
        theRenderContext.SetDepthTestEnabled(false);
        theRenderContext.SetDepthWriteEnabled(false);
    }
    BlendAdvancedEquationSwFallback(m_AdvancedBlendDrawTexture, m_LayerTexture, advancedMode);
    theRenderContext.SetRenderTarget(*theFB);
    // setup read target
    theRenderContext.SetReadTarget(m_AdvancedModeBlendFB);
    theRenderContext.SetReadBuffer(QDemonReadFaces::Color0);
    theRenderContext.BlitFramebuffer(0, 0, theViewport.m_Width, theViewport.m_Height,
                                     0, 0, theViewport.m_Width, theViewport.m_Height,
                                     QDemonRenderClearValues::Color,
                                     QDemonRenderTextureMagnifyingOp::Nearest);
}
#endif

void SLayerRenderData::RenderToViewport()
{
    if (m_LayerPrepResult->IsLayerVisible()) {
        if (GetOffscreenRenderer()) {
            if (m_Layer.m_Background == LayerBackground::Color) {
                m_LastFrameOffscreenRenderer->RenderWithClear(
                            CreateOffscreenRenderEnvironment(), m_Renderer.GetContext(),
                            m_Renderer.GetQt3DSContext().GetPresentationScaleFactor(),
                            SScene::AlwaysClear, m_Layer.m_ClearColor, &m_Layer);
            } else {
                m_LastFrameOffscreenRenderer->Render(
                            CreateOffscreenRenderEnvironment(), m_Renderer.GetContext(),
                            m_Renderer.GetQt3DSContext().GetPresentationScaleFactor(),
                            SScene::ClearIsOptional, &m_Layer);
            }
        } else {
            RenderDepthPass(false);
            Render();
            RenderRenderWidgets();
        }
    }
}
// These are meant to be pixel offsets, so you need to divide them by the width/height
// of the layer respectively.
const QVector2D s_VertexOffsets[SLayerRenderPreparationData::MAX_AA_LEVELS] = {
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
const QVector2D s_BlendFactors[SLayerRenderPreparationData::MAX_AA_LEVELS] = {
    QVector2D(0.500000f, 0.500000f), // 1x
    QVector2D(0.333333f, 0.666667f), // 2x
    QVector2D(0.250000f, 0.750000f), // 3x
    QVector2D(0.200000f, 0.800000f), // 4x
    QVector2D(0.166667f, 0.833333f), // 5x
    QVector2D(0.142857f, 0.857143f), // 6x
    QVector2D(0.125000f, 0.875000f), // 7x
    QVector2D(0.111111f, 0.888889f), // 8x
};

const QVector2D s_TemporalVertexOffsets[SLayerRenderPreparationData::MAX_TEMPORAL_AA_LEVELS] = {
    QVector2D(.3f, .3f), QVector2D(-.3f, -.3f)
};

static inline void OffsetProjectionMatrix(QMatrix4x4 &inProjectionMatrix, QVector2D inVertexOffsets)
{
    inProjectionMatrix.column3.x =
            inProjectionMatrix.column3.x + inProjectionMatrix.column3.w * inVertexOffsets.x;
    inProjectionMatrix.column3.y =
            inProjectionMatrix.column3.y + inProjectionMatrix.column3.w * inVertexOffsets.y;
}

QString depthPassStr;

// Render this layer's data to a texture.  Required if we have any effects,
// prog AA, or if forced.
void SLayerRenderData::RenderToTexture()
{
    Q_ASSERT(m_LayerPrepResult->m_Flags.ShouldRenderToTexture());
    SLayerRenderPreparationResult &thePrepResult(*m_LayerPrepResult);
    QDemonRenderContext &theRenderContext(m_Renderer.GetContext());
    QSize theLayerTextureDimensions = thePrepResult.GetTextureDimensions();
    QSize theLayerOriginalTextureDimensions = theLayerTextureDimensions;
    QDemonRenderTextureFormats::Enum DepthTextureFormat = QDemonRenderTextureFormats::Depth24Stencil8;
    QDemonRenderTextureFormats::Enum ColorTextureFormat = QDemonRenderTextureFormats::RGBA8;
    if (thePrepResult.m_LastEffect
            && theRenderContext.GetRenderContextType() != QDemonRenderContextValues::GLES2) {
        if (m_Layer.m_Background != LayerBackground::Transparent)
            ColorTextureFormat = QDemonRenderTextureFormats::R11G11B10;
        else
            ColorTextureFormat = QDemonRenderTextureFormats::RGBA16F;
    }
    QDemonRenderTextureFormats::Enum ColorSSAOTextureFormat = QDemonRenderTextureFormats::RGBA8;

    bool needsRender = false;
    quint32 sampleCount = 1;
    // check multsample mode and MSAA texture support
    if (m_Layer.m_MultisampleAAMode != AAModeValues::NoAA
            && theRenderContext.AreMultisampleTexturesSupported())
        sampleCount = (quint32)m_Layer.m_MultisampleAAMode;

    bool isMultisamplePass = false;
    if (theRenderContext.GetRenderContextType() != QDemonRenderContextValues::GLES2)
        isMultisamplePass =
                (sampleCount > 1) || (m_Layer.m_MultisampleAAMode == AAModeValues::SSAA);

    QDemonRenderTextureTargetType::Enum thFboAttachTarget =
            QDemonRenderTextureTargetType::Texture2D;

    // If the user has disabled all layer caching this has the side effect of disabling the
    // progressive AA algorithm.
    if (thePrepResult.m_Flags.WasLayerDataDirty()
            || thePrepResult.m_Flags.WasDirty()
            || m_Renderer.IsLayerCachingEnabled() == false
            || thePrepResult.m_Flags.ShouldRenderToTexture()) {
        m_ProgressiveAAPassIndex = 0;
        m_NonDirtyTemporalAAPassIndex = 0;
        needsRender = true;
    }

    CResourceTexture2D *renderColorTexture = &m_LayerTexture;
    CResourceTexture2D *renderPrepassDepthTexture = &m_LayerPrepassDepthTexture;
    CResourceTexture2D *renderWidgetTexture = &m_LayerWidgetTexture;
    QDemonRenderContextScopedProperty<bool> __multisampleEnabled(
                theRenderContext, &QDemonRenderContext::IsMultisampleEnabled,
                &QDemonRenderContext::SetMultisampleEnabled);
    theRenderContext.SetMultisampleEnabled(false);
    if (isMultisamplePass) {
        renderColorTexture = &m_LayerMultisampleTexture;
        renderPrepassDepthTexture = &m_LayerMultisamplePrepassDepthTexture;
        renderWidgetTexture = &m_LayerMultisampleWidgetTexture;
        // for SSAA we don't use MS textures
        if (m_Layer.m_MultisampleAAMode != AAModeValues::SSAA)
            thFboAttachTarget = QDemonRenderTextureTargetType::Texture2D_MS;
    }
    quint32 maxTemporalPassIndex = m_Layer.m_TemporalAAEnabled ? 2 : 0;

    // If all the dimensions match then we do not have to re-render the layer.
    if (m_LayerTexture.TextureMatches(theLayerTextureDimensions.width(),
                                      theLayerTextureDimensions.height(), ColorTextureFormat)
            && (!thePrepResult.m_Flags.RequiresDepthTexture()
                || m_LayerDepthTexture.TextureMatches(theLayerTextureDimensions.width(),
                                                      theLayerTextureDimensions.height(),
                                                      DepthTextureFormat))
            && m_ProgressiveAAPassIndex >= thePrepResult.m_MaxAAPassIndex
            && m_NonDirtyTemporalAAPassIndex >= maxTemporalPassIndex && needsRender == false) {
        return;
    }

    // adjust render size for SSAA
    if (m_Layer.m_MultisampleAAMode == AAModeValues::SSAA) {
        quint32 ow, oh;
        CRendererUtil::GetSSAARenderSize(theLayerOriginalTextureDimensions.width(),
                                         theLayerOriginalTextureDimensions.height(),
                                         ow, oh);
        theLayerTextureDimensions = QSize(ow, oh);
    }

    // If our pass index == thePreResult.m_MaxAAPassIndex then
    // we shouldn't get into here.

    IResourceManager &theResourceManager = m_Renderer.GetQt3DSContext().GetResourceManager();
    bool hadLayerTexture = true;

    if (renderColorTexture->EnsureTexture(theLayerTextureDimensions.width(),
                                          theLayerTextureDimensions.height(),
                                          ColorTextureFormat, sampleCount)) {
        m_ProgressiveAAPassIndex = 0;
        m_NonDirtyTemporalAAPassIndex = 0;
        hadLayerTexture = false;
    }

    if (thePrepResult.m_Flags.RequiresDepthTexture()) {
        // The depth texture doesn't need to be multisample, the prepass depth does.
        if (m_LayerDepthTexture.EnsureTexture(theLayerTextureDimensions.width(),
                                              theLayerTextureDimensions.height(),
                                              DepthTextureFormat)) {
            // Depth textures are generally not bilinear filtered.
            m_LayerDepthTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Nearest);
            m_LayerDepthTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Nearest);
            m_ProgressiveAAPassIndex = 0;
            m_NonDirtyTemporalAAPassIndex = 0;
        }
    }

    if (thePrepResult.m_Flags.RequiresSsaoPass()) {
        if (m_LayerSsaoTexture.EnsureTexture(theLayerTextureDimensions.width(),
                                             theLayerTextureDimensions.height(),
                                             ColorSSAOTextureFormat)) {
            m_LayerSsaoTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            m_LayerSsaoTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            m_ProgressiveAAPassIndex = 0;
            m_NonDirtyTemporalAAPassIndex = 0;
        }
    }

    Q_ASSERT(!thePrepResult.m_Flags.RequiresDepthTexture() || m_LayerDepthTexture);
    Q_ASSERT(!thePrepResult.m_Flags.RequiresSsaoPass() || m_LayerSsaoTexture);

    CResourceTexture2D theLastLayerTexture(theResourceManager);
    SLayerProgAABlendShader *theBlendShader = nullptr;
    quint32 aaFactorIndex = 0;
    bool isProgressiveAABlendPass =
            m_ProgressiveAAPassIndex && m_ProgressiveAAPassIndex < thePrepResult.m_MaxAAPassIndex;
    bool isTemporalAABlendPass = m_Layer.m_TemporalAAEnabled && m_ProgressiveAAPassIndex == 0;

    if (isProgressiveAABlendPass || isTemporalAABlendPass) {
        theBlendShader = m_Renderer.GetLayerProgAABlendShader();
        if (theBlendShader) {
            m_LayerTexture.EnsureTexture(theLayerOriginalTextureDimensions.width(),
                                         theLayerOriginalTextureDimensions.height(),
                                         ColorTextureFormat);
            QVector2D theVertexOffsets;
            if (isProgressiveAABlendPass) {
                theLastLayerTexture.StealTexture(m_LayerTexture);
                aaFactorIndex = (m_ProgressiveAAPassIndex - 1);
                theVertexOffsets = s_VertexOffsets[aaFactorIndex];
            } else {
                if (m_TemporalAATexture.GetTexture())
                    theLastLayerTexture.StealTexture(m_TemporalAATexture);
                else {
                    if (hadLayerTexture) {
                        theLastLayerTexture.StealTexture(m_LayerTexture);
                    }
                }
                theVertexOffsets = s_TemporalVertexOffsets[m_TemporalAAPassIndex];
                ++m_TemporalAAPassIndex;
                ++m_NonDirtyTemporalAAPassIndex;
                m_TemporalAAPassIndex = m_TemporalAAPassIndex % MAX_TEMPORAL_AA_LEVELS;
            }
            if (theLastLayerTexture.GetTexture()) {
                theVertexOffsets.x =
                        theVertexOffsets.x / (theLayerOriginalTextureDimensions.width() / 2.0f);
                theVertexOffsets.y =
                        theVertexOffsets.y / (theLayerOriginalTextureDimensions.height() / 2.0f);
                // Run through all models and update MVP.
                // run through all texts and update MVP.
                // run through all path and update MVP.

                // TODO - optimize this exact matrix operation.
                for (quint32 idx = 0, end = m_ModelContexts.size(); idx < end; ++idx) {
                    QMatrix4x4 &originalProjection(m_ModelContexts[idx]->m_ModelViewProjection);
                    OffsetProjectionMatrix(originalProjection, theVertexOffsets);
                }
                for (quint32 idx = 0, end = m_OpaqueObjects.size(); idx < end; ++idx) {
                    if (m_OpaqueObjects[idx]->m_RenderableFlags.IsPath()) {
                        SPathRenderable &theRenderable =
                                static_cast<SPathRenderable &>(*m_OpaqueObjects[idx]);
                        OffsetProjectionMatrix(theRenderable.m_ModelViewProjection,
                                               theVertexOffsets);
                    }
                }
                for (quint32 idx = 0, end = m_TransparentObjects.size(); idx < end; ++idx) {
                    if (m_TransparentObjects[idx]->m_RenderableFlags.IsText()) {
                        STextRenderable &theRenderable =
                                static_cast<STextRenderable &>(*m_TransparentObjects[idx]);
                        OffsetProjectionMatrix(theRenderable.m_ModelViewProjection,
                                               theVertexOffsets);
                    } else if (m_TransparentObjects[idx]->m_RenderableFlags.IsPath()) {
                        SPathRenderable &theRenderable =
                                static_cast<SPathRenderable &>(*m_TransparentObjects[idx]);
                        OffsetProjectionMatrix(theRenderable.m_ModelViewProjection,
                                               theVertexOffsets);
                    }
                }
            }
        }
    }
    if (theLastLayerTexture.GetTexture() == nullptr) {
        isProgressiveAABlendPass = false;
        isTemporalAABlendPass = false;
    }
    // Sometimes we will have stolen the render texture.
    renderColorTexture->EnsureTexture(theLayerTextureDimensions.width(),
                                      theLayerTextureDimensions.height(), ColorTextureFormat,
                                      sampleCount);

    if (!isTemporalAABlendPass)
        m_TemporalAATexture.ReleaseTexture();

    // Allocating a frame buffer can cause it to be bound, so we need to save state before this
    // happens.
    QDemonRenderContextScopedProperty<QDemonRenderFrameBuffer *> __framebuf(
                theRenderContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
    // Match the bit depth of the current render target to avoid popping when we switch from aa
    // to non aa layers
    // We have to all this here in because once we change the FB by allocating an FB we are
    // screwed.
    QDemonRenderTextureFormats::Enum theDepthFormat(GetDepthBufferFormat());
    QDemonRenderFrameBufferAttachments::Enum theDepthAttachmentFormat(
                GetFramebufferDepthAttachmentFormat(theDepthFormat));

    // Definitely disable the scissor rect if it is running right now.
    QDemonRenderContextScopedProperty<bool> __scissorEnabled(
                theRenderContext, &QDemonRenderContext::IsScissorTestEnabled,
                &QDemonRenderContext::SetScissorTestEnabled, false);
    CResourceFrameBuffer theFB(theResourceManager);
    // Allocates the frame buffer which has the side effect of setting the current render target
    // to that frame buffer.
    theFB.EnsureFrameBuffer();

    bool hasDepthObjects = m_OpaqueObjects.size() > 0;
    bool requiresDepthStencilBuffer =
            hasDepthObjects || thePrepResult.m_Flags.RequiresStencilBuffer();
    QDemonRenderRect theNewViewport(0, 0, theLayerTextureDimensions.width(),
                                    theLayerTextureDimensions.height());
    {
        theRenderContext.SetRenderTarget(theFB);
        QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                    theRenderContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport,
                    theNewViewport);
        QVector4D clearColor(0.0);
        if (m_Layer.m_Background == LayerBackground::Color)
            clearColor = QVector4D(m_Layer.m_ClearColor, 1.0);

        QDemonRenderContextScopedProperty<QVector4D> __clearColor(
                    theRenderContext, &QDemonRenderContext::GetClearColor, &QDemonRenderContext::SetClearColor,
                    clearColor);
        if (requiresDepthStencilBuffer) {
            if (renderPrepassDepthTexture->EnsureTexture(theLayerTextureDimensions.width(),
                                                         theLayerTextureDimensions.height(),
                                                         theDepthFormat, sampleCount)) {
                (*renderPrepassDepthTexture)->SetMinFilter(QDemonRenderTextureMinifyingOp::Nearest);
                (*renderPrepassDepthTexture)
                        ->SetMagFilter(QDemonRenderTextureMagnifyingOp::Nearest);
            }
        }

        if (thePrepResult.m_Flags.RequiresDepthTexture() && m_ProgressiveAAPassIndex == 0) {
            // Setup FBO with single depth buffer target.
            // Note this does not use multisample.
            QDemonRenderFrameBufferAttachments::Enum theAttachment =
                    GetFramebufferDepthAttachmentFormat(DepthTextureFormat);
            theFB->Attach(theAttachment, *m_LayerDepthTexture);

            // In this case transparent objects also may write their depth.
            RenderDepthPass(true);
            theFB->Attach(theAttachment, QDemonRenderTextureOrRenderBuffer());
        }

        if (thePrepResult.m_Flags.RequiresSsaoPass() && m_ProgressiveAAPassIndex == 0
                && m_Camera != nullptr) {
            StartProfiling("AO pass", false);
            // Setup FBO with single color buffer target
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, *m_LayerSsaoTexture);
            theRenderContext.Clear(QDemonRenderClearValues::Color);
            RenderAoPass();
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                          QDemonRenderTextureOrRenderBuffer());
            EndProfiling("AO pass");
        }

        if (thePrepResult.m_Flags.RequiresShadowMapPass() && m_ProgressiveAAPassIndex == 0) {
            // shadow map path
            RenderShadowMapPass(&theFB);
        }

        if (sampleCount > 1) {
            theRenderContext.SetMultisampleEnabled(true);
        }

        QDemonRenderClearFlags clearFlags = QDemonRenderClearValues::Color;

        // render depth prepass
        if ((*renderPrepassDepthTexture)) {
            theFB->Attach(theDepthAttachmentFormat, **renderPrepassDepthTexture,
                          thFboAttachTarget);

            if (m_Layer.m_Flags.IsLayerEnableDepthPrepass()) {
                StartProfiling("Depth pass", false);
                RenderDepthPass(false);
                EndProfiling("Depth pass");
            } else {
                clearFlags |= (QDemonRenderClearValues::Depth);
                clearFlags |= (QDemonRenderClearValues::Stencil);
                // enable depth write for the clear below
                theRenderContext.SetDepthWriteEnabled(true);
            }
        }

        theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, **renderColorTexture,
                      thFboAttachTarget);
        if (m_Layer.m_Background != LayerBackground::Unspecified)
            theRenderContext.Clear(clearFlags);

        // We don't clear the depth buffer because the layer render code we are about to call
        // will do this.
        StartProfiling("Render pass", false);
        Render(&theFB);
        // Debug measure to view the depth map to ensure we're rendering it correctly.
        //if (m_Layer.m_TemporalAAEnabled) {
        //    RenderFakeDepthMapPass(m_ShadowMapManager->GetShadowMapEntry(0)->m_DepthMap,
        //                           m_ShadowMapManager->GetShadowMapEntry(0)->m_DepthCube);
        //}
        EndProfiling("Render pass");

        // Now before going further, we downsample and resolve the multisample information.
        // This allows all algorithms running after
        // this point to run unchanged.
        if (isMultisamplePass) {
            if (m_Layer.m_MultisampleAAMode != AAModeValues::SSAA) {
                // Resolve the FBO to the layer texture
                CRendererUtil::ResolveMutisampleFBOColorOnly(
                            theResourceManager, m_LayerTexture, theRenderContext,
                            theLayerTextureDimensions.width(), theLayerTextureDimensions.height(),
                            ColorTextureFormat, *theFB);

                theRenderContext.SetMultisampleEnabled(false);
            } else {
                // Resolve the FBO to the layer texture
                CRendererUtil::ResolveSSAAFBOColorOnly(
                            theResourceManager, m_LayerTexture,
                            theLayerOriginalTextureDimensions.width(),
                            theLayerOriginalTextureDimensions.height(), theRenderContext,
                            theLayerTextureDimensions.width(), theLayerTextureDimensions.height(),
                            ColorTextureFormat, *theFB);
            }
        }

        // CN - when I tried to get anti-aliased widgets I lost all transparency on the widget
        // layer which made it overwrite the object you were
        // manipulating.  When I tried to use parallel nsight on it the entire studio
        // application crashed on startup.
        if (NeedsWidgetTexture()) {
            m_LayerWidgetTexture.EnsureTexture(theLayerTextureDimensions.width(),
                                               theLayerTextureDimensions.height(),
                                               QDemonRenderTextureFormats::RGBA8);
            theRenderContext.SetRenderTarget(theFB);
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, *m_LayerWidgetTexture);
            theFB->Attach(GetFramebufferDepthAttachmentFormat(DepthTextureFormat),
                          *m_LayerDepthTexture);
            theRenderContext.SetClearColor(QVector4D(0.0f));
            theRenderContext.Clear(QDemonRenderClearValues::Color);
            // We should already have the viewport and everything setup for this.
            RenderRenderWidgets();
        }

        if (theLastLayerTexture.GetTexture() != nullptr
                && (isProgressiveAABlendPass || isTemporalAABlendPass)) {
            theRenderContext.SetViewport(
                        QDemonRenderRect(0, 0, theLayerOriginalTextureDimensions.width(),
                                         theLayerOriginalTextureDimensions.height()));
            CResourceTexture2D targetTexture(
                        theResourceManager, theLayerOriginalTextureDimensions.width(),
                        theLayerOriginalTextureDimensions.height(), ColorTextureFormat);
            theFB->Attach(theDepthAttachmentFormat,
                          QDemonRenderTextureOrRenderBuffer());
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, *targetTexture);
            QVector2D theBlendFactors;
            if (isProgressiveAABlendPass)
                theBlendFactors = s_BlendFactors[aaFactorIndex];
            else
                theBlendFactors = QVector2D(.5f, .5f);

            theRenderContext.SetDepthTestEnabled(false);
            theRenderContext.SetBlendingEnabled(false);
            theRenderContext.SetCullingEnabled(false);
            theRenderContext.SetActiveShader(theBlendShader->m_Shader);
            theBlendShader->m_AccumSampler.Set(theLastLayerTexture);
            theBlendShader->m_LastFrame.Set(m_LayerTexture);
            theBlendShader->m_BlendFactors.Set(theBlendFactors);
            m_Renderer.RenderQuad();
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                          QDemonRenderTextureOrRenderBuffer());
            if (isTemporalAABlendPass)
                m_TemporalAATexture.StealTexture(m_LayerTexture);
            m_LayerTexture.StealTexture(targetTexture);
        }

        m_LayerTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        m_LayerTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);

        // Don't remember why needs widget texture is false here.
        // Should have commented why progAA plus widgets is a fail.
        if (m_ProgressiveAAPassIndex < thePrepResult.m_MaxAAPassIndex
                && NeedsWidgetTexture() == false)
            ++m_ProgressiveAAPassIndex;

        // now we render all post effects
#ifdef QDEMON_CACHED_POST_EFFECT
        ApplyLayerPostEffects();
#endif

        if (m_LayerPrepassDepthTexture) {
            // Detach any depth buffers.
            theFB->Attach(theDepthAttachmentFormat, QDemonRenderTextureOrRenderBuffer(),
                          thFboAttachTarget);
        }

        theFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                      QDemonRenderTextureOrRenderBuffer(), thFboAttachTarget);
        // Let natural scoping rules destroy the other stuff.
    }
}

void SLayerRenderData::ApplyLayerPostEffects()
{
    if (m_Layer.m_FirstEffect == nullptr) {
        if (m_LayerCachedTexture) {
            IResourceManager &theResourceManager(m_Renderer.GetQt3DSContext().GetResourceManager());
            theResourceManager.Release(*m_LayerCachedTexture);
            m_LayerCachedTexture = nullptr;
        }
        return;
    }

    IEffectSystem &theEffectSystem(m_Renderer.GetQt3DSContext().GetEffectSystem());
    IResourceManager &theResourceManager(m_Renderer.GetQt3DSContext().GetResourceManager());
    // we use the non MSAA buffer for the effect
    QDemonRenderTexture2D *theLayerColorTexture = m_LayerTexture;
    QDemonRenderTexture2D *theLayerDepthTexture = m_LayerDepthTexture;

    QDemonRenderTexture2D *theCurrentTexture = theLayerColorTexture;
    for (SEffect *theEffect = m_Layer.m_FirstEffect; theEffect;
         theEffect = theEffect->m_NextEffect) {
        if (theEffect->m_Flags.IsActive() && m_Camera) {
            StartProfiling(theEffect->m_ClassName, false);

            QDemonRenderTexture2D *theRenderedEffect = theEffectSystem.RenderEffect(
                        SEffectRenderArgument(*theEffect, *theCurrentTexture,
                                              QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar),
                                              theLayerDepthTexture, m_LayerPrepassDepthTexture));

            EndProfiling(theEffect->m_ClassName);

            // If the texture came from rendering a chain of effects, then we don't need it
            // after this.
            if (theCurrentTexture != theLayerColorTexture)
                theResourceManager.Release(*theCurrentTexture);

            theCurrentTexture = theRenderedEffect;

            if (!theRenderedEffect) {
                QString errorMsg = QObject::tr("Failed to compile \"%1\" effect.\nConsider"
                                               " removing it from the presentation.")
                        .arg(theEffect->m_ClassName.c_str());
                QDEMON_ALWAYS_ASSERT_MESSAGE(errorMsg.toUtf8());
                break;
            }
        }
    }

    if (m_LayerCachedTexture && m_LayerCachedTexture != m_LayerTexture) {
        theResourceManager.Release(*m_LayerCachedTexture);
        m_LayerCachedTexture = nullptr;
    }

    if (theCurrentTexture != m_LayerTexture)
        m_LayerCachedTexture = theCurrentTexture;
}

inline bool AnyCompletelyNonTransparentObjects(TRenderableObjectList &inObjects)
{
    for (quint32 idx = 0, end = inObjects.size(); idx < end; ++idx) {
        if (inObjects[idx]->m_RenderableFlags.IsCompletelyTransparent() == false)
            return true;
    }
    return false;
}

void SLayerRenderData::RunnableRenderToViewport(QDemonRenderFrameBuffer *theFB)
{
    // If we have an effect, an opaque object, or any transparent objects that aren't completely
    // transparent
    // or an offscreen renderer or a layer widget texture
    // Then we can't possible affect the resulting render target.
    bool needsToRender = m_Layer.m_FirstEffect != nullptr || m_OpaqueObjects.empty() == false
            || AnyCompletelyNonTransparentObjects(m_TransparentObjects) || GetOffscreenRenderer()
            || m_LayerWidgetTexture || m_BoundingRectColor.hasValue()
            || m_Layer.m_Background == LayerBackground::Color;

    if (needsToRender == false)
        return;

    QDemonRenderContext &theContext(m_Renderer.GetContext());
    theContext.resetStates();

    QDemonRenderContextScopedProperty<QDemonRenderFrameBuffer *> __fbo(
                theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
    QDemonRenderRect theCurrentViewport = theContext.GetViewport();
    QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);
    QDemonRenderContextScopedProperty<bool> theScissorEnabled(
                theContext, &QDemonRenderContext::IsScissorTestEnabled,
                &QDemonRenderContext::SetScissorTestEnabled);
    QDemonRenderContextScopedProperty<QDemonRenderRect> theScissorRect(
                theContext, &QDemonRenderContext::GetScissorRect, &QDemonRenderContext::SetScissorRect);
    SLayerRenderPreparationResult &thePrepResult(*m_LayerPrepResult);
    QDemonRenderRectF theScreenRect(thePrepResult.GetLayerToPresentationViewport());

    bool blendingEnabled = m_Layer.m_Background == LayerBackground::Transparent;
    if (!thePrepResult.m_Flags.ShouldRenderToTexture()) {
        theContext.SetViewport(
                    m_LayerPrepResult->GetLayerToPresentationViewport().ToIntegerRect());
        theContext.SetScissorTestEnabled(true);
        theContext.SetScissorRect(
                    m_LayerPrepResult->GetLayerToPresentationScissorRect().ToIntegerRect());
        if (m_Layer.m_Background == LayerBackground::Color) {
            QDemonRenderContextScopedProperty<QVector4D> __clearColor(
                        theContext, &QDemonRenderContext::GetClearColor, &QDemonRenderContext::SetClearColor,
                        QVector4D(m_Layer.m_ClearColor, 0.0f));
            theContext.Clear(QDemonRenderClearValues::Color);
        }
        RenderToViewport();
    } else {
        // First, render the layer along with whatever progressive AA is appropriate.
        // The render graph should have taken care of the render to texture step.
#ifdef QDEMON_CACHED_POST_EFFECT
        QDemonRenderTexture2D *theLayerColorTexture =
                (m_LayerCachedTexture) ? m_LayerCachedTexture : m_LayerTexture;
#else
        // Then render all but the last effect
        IEffectSystem &theEffectSystem(m_Renderer.GetQt3DSContext().GetEffectSystem());
        IResourceManager &theResourceManager(m_Renderer.GetQt3DSContext().GetResourceManager());
        // we use the non MSAA buffer for the effect
        QDemonRenderTexture2D *theLayerColorTexture = m_LayerTexture;
        QDemonRenderTexture2D *theLayerDepthTexture = m_LayerDepthTexture;

        QDemonRenderTexture2D *theCurrentTexture = theLayerColorTexture;
        for (SEffect *theEffect = m_Layer.m_FirstEffect;
             theEffect && theEffect != thePrepResult.m_LastEffect;
             theEffect = theEffect->m_NextEffect) {
            if (theEffect->m_Flags.IsActive() && m_Camera) {
                StartProfiling(theEffect->m_ClassName, false);

                QDemonRenderTexture2D *theRenderedEffect = theEffectSystem.RenderEffect(
                            SEffectRenderArgument(*theEffect, *theCurrentTexture,
                                                  QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar),
                                                  theLayerDepthTexture, m_LayerPrepassDepthTexture));

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
        QMatrix4x4 theFinalMVP(QMatrix4x4::createIdentity());
        SCamera theTempCamera;
        QDemonRenderRect theLayerViewport(
                    thePrepResult.GetLayerToPresentationViewport().ToIntegerRect());
        QDemonRenderRect theLayerClip(
                    thePrepResult.GetLayerToPresentationScissorRect().ToIntegerRect());

        {
            QMatrix3x3 ignored;
            QMatrix4x4 theViewProjection;
            // We could cache these variables
            theTempCamera.m_Flags.SetOrthographic(true);
            theTempCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
            // Move the camera back far enough that we can see everything
            float theCameraSetback(10);
            // Attempt to ensure the layer can never be clipped.
            theTempCamera.m_Position.z = -theCameraSetback;
            theTempCamera.m_ClipFar = 2.0f * theCameraSetback;
            // Render the layer texture to the entire viewport.
            SCameraGlobalCalculationResult theResult = theTempCamera.CalculateGlobalVariables(
                        theLayerViewport,
                        QVector2D((float)theLayerViewport.m_Width, (float)theLayerViewport.m_Height));
            theTempCamera.CalculateViewProjectionMatrix(theViewProjection);
            SNode theTempNode;
            theFinalMVP = theViewProjection;
            QDemonRenderBlendFunctionArgument blendFunc;
            QDemonRenderBlendEquationArgument blendEqu;

            switch (m_Layer.m_BlendType) {
            case LayerBlendTypes::Screen:
                blendFunc = QDemonRenderBlendFunctionArgument(
                            QDemonRenderSrcBlendFunc::SrcAlpha, QDemonRenderDstBlendFunc::One,
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            case LayerBlendTypes::Multiply:
                blendFunc = QDemonRenderBlendFunctionArgument(
                            QDemonRenderSrcBlendFunc::DstColor, QDemonRenderDstBlendFunc::Zero,
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            case LayerBlendTypes::Add:
                blendFunc = QDemonRenderBlendFunctionArgument(
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One,
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            case LayerBlendTypes::Subtract:
                blendFunc = QDemonRenderBlendFunctionArgument(
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One,
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
                blendEqu = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::ReverseSubtract,
                            QDemonRenderBlendEquation::ReverseSubtract);
                break;
            case LayerBlendTypes::Overlay:
                // SW fallback doesn't use blend equation
                // note blend func is not used here anymore
                if (theContext.IsAdvancedBlendHwSupported() ||
                        theContext.IsAdvancedBlendHwSupportedKHR())
                    blendEqu = QDemonRenderBlendEquationArgument(
                                QDemonRenderBlendEquation::Overlay, QDemonRenderBlendEquation::Overlay);
                break;
            case LayerBlendTypes::ColorBurn:
                // SW fallback doesn't use blend equation
                // note blend func is not used here anymore
                if (theContext.IsAdvancedBlendHwSupported() ||
                        theContext.IsAdvancedBlendHwSupportedKHR())
                    blendEqu = QDemonRenderBlendEquationArgument(
                                QDemonRenderBlendEquation::ColorBurn, QDemonRenderBlendEquation::ColorBurn);
                break;
            case LayerBlendTypes::ColorDodge:
                // SW fallback doesn't use blend equation
                // note blend func is not used here anymore
                if (theContext.IsAdvancedBlendHwSupported() ||
                        theContext.IsAdvancedBlendHwSupportedKHR())
                    blendEqu = QDemonRenderBlendEquationArgument(
                                QDemonRenderBlendEquation::ColorDodge, QDemonRenderBlendEquation::ColorDodge);
                break;
            default:
                blendFunc = QDemonRenderBlendFunctionArgument(
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha);
                blendEqu = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
                break;
            }
            theContext.SetBlendFunction(blendFunc);
            theContext.SetBlendEquation(blendEqu);
            theContext.SetBlendingEnabled(blendingEnabled);
            theContext.SetDepthTestEnabled(false);
        }

        {
            theContext.SetScissorTestEnabled(true);
            theContext.SetViewport(theLayerViewport);
            theContext.SetScissorRect(theLayerClip);

            // Remember the camera we used so we can get a valid pick ray
            m_SceneCamera = theTempCamera;
            theContext.SetDepthTestEnabled(false);
#ifndef QDEMON_CACHED_POST_EFFECT
            if (thePrepResult.m_LastEffect && m_Camera) {
                StartProfiling(thePrepResult.m_LastEffect->m_ClassName, false);
                // inUseLayerMPV is true then we are rendering directly to the scene and thus we
                // should enable blending
                // for the final render pass.  Else we should leave it.
                theEffectSystem.RenderEffect(
                            SEffectRenderArgument(*thePrepResult.m_LastEffect, *theCurrentTexture,
                                                  QVector2D(m_Camera->m_ClipNear, m_Camera->m_ClipFar),
                                                  theLayerDepthTexture, m_LayerPrepassDepthTexture),
                            theFinalMVP, blendingEnabled);
                EndProfiling(thePrepResult.m_LastEffect->m_ClassName);
                // If the texture came from rendering a chain of effects, then we don't need it
                // after this.
                if (theCurrentTexture != theLayerColorTexture)
                    theResourceManager.Release(*theCurrentTexture);
            } else
#endif
            {
                theContext.SetCullingEnabled(false);
                theContext.SetBlendingEnabled(blendingEnabled);
                theContext.SetDepthTestEnabled(false);
#ifdef ADVANCED_BLEND_SW_FALLBACK
                QDemonScopedRefCounted<QDemonRenderTexture2D> screenTexture =
                        m_Renderer.GetLayerBlendTexture();
                QDemonScopedRefCounted<QDemonRenderFrameBuffer> blendFB = m_Renderer.GetBlendFB();

                // Layer blending for advanced blending modes if SW fallback is needed
                // rendering to FBO and blending with separate shader
                if (screenTexture) {
                    // Blending is enabled only if layer background has been chosen transparent
                    // Layers with advanced blending modes
                    if (blendingEnabled && (m_Layer.m_BlendType == LayerBlendTypes::Overlay ||
                                            m_Layer.m_BlendType == LayerBlendTypes::ColorBurn ||
                                            m_Layer.m_BlendType == LayerBlendTypes::ColorDodge)) {
                        theContext.SetScissorTestEnabled(false);
                        theContext.SetBlendingEnabled(false);

                        // Get part matching to layer from screen texture and
                        // use that for blending
                        QDemonRenderTexture2D *blendBlitTexture;
                        blendBlitTexture = theContext.CreateTexture2D();
                        blendBlitTexture->SetTextureData(QDemonDataRef<quint8>(), 0,
                                                         theLayerViewport.m_Width,
                                                         theLayerViewport.m_Height,
                                                         QDemonRenderTextureFormats::RGBA8);
                        QDemonRenderFrameBuffer *blitFB;
                        blitFB = theContext.CreateFrameBuffer();
                        blitFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                       QDemonRenderTextureOrRenderBuffer(*blendBlitTexture));
                        blendFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                        QDemonRenderTextureOrRenderBuffer(*screenTexture));
                        theContext.SetRenderTarget(blitFB);
                        theContext.SetReadTarget(blendFB);
                        theContext.SetReadBuffer(QDemonReadFaces::Color0);
                        theContext.BlitFramebuffer(theLayerViewport.m_X, theLayerViewport.m_Y,
                                                   theLayerViewport.m_Width +
                                                   theLayerViewport.m_X,
                                                   theLayerViewport.m_Height +
                                                   theLayerViewport.m_Y,
                                                   0, 0,
                                                   theLayerViewport.m_Width,
                                                   theLayerViewport.m_Height,
                                                   QDemonRenderClearValues::Color,
                                                   QDemonRenderTextureMagnifyingOp::Nearest);

                        QDemonRenderTexture2D *blendResultTexture;
                        blendResultTexture = theContext.CreateTexture2D();
                        blendResultTexture->SetTextureData(QDemonDataRef<quint8>(), 0,
                                                           theLayerViewport.m_Width,
                                                           theLayerViewport.m_Height,
                                                           QDemonRenderTextureFormats::RGBA8);
                        QDemonRenderFrameBuffer *resultFB;
                        resultFB = theContext.CreateFrameBuffer();
                        resultFB->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                         QDemonRenderTextureOrRenderBuffer(*blendResultTexture));
                        theContext.SetRenderTarget(resultFB);

                        AdvancedBlendModes::Enum advancedMode;
                        switch (m_Layer.m_BlendType) {
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

                        theContext.SetViewport(QDemonRenderRect(0, 0, theLayerViewport.m_Width,
                                                                theLayerViewport.m_Height));
                        BlendAdvancedEquationSwFallback(theLayerColorTexture, blendBlitTexture,
                                                        advancedMode);
                        //blitFB->release();
                        // save blending result to screen texture for use with other layers
                        theContext.SetViewport(theLayerViewport);
                        theContext.SetRenderTarget(blendFB);
                        m_Renderer.RenderQuad(QVector2D((float)theLayerViewport.m_Width,
                                                        (float)theLayerViewport.m_Height),
                                              theFinalMVP, *blendResultTexture);
                        // render the blended result
                        theContext.SetRenderTarget(theFB);
                        theContext.SetScissorTestEnabled(true);
                        m_Renderer.RenderQuad(QVector2D((float)theLayerViewport.m_Width,
                                                        (float)theLayerViewport.m_Height),
                                              theFinalMVP, *blendResultTexture);
                        //resultFB->release();
                    } else {
                        // Layers with normal blending modes
                        // save result for future use
                        theContext.SetViewport(theLayerViewport);
                        theContext.SetScissorTestEnabled(false);
                        theContext.SetBlendingEnabled(true);
                        theContext.SetRenderTarget(blendFB);
                        m_Renderer.RenderQuad(QVector2D((float)theLayerViewport.m_Width,
                                                        (float)theLayerViewport.m_Height),
                                              theFinalMVP, *theLayerColorTexture);
                        theContext.SetRenderTarget(theFB);
                        theContext.SetScissorTestEnabled(true);
                        theContext.SetViewport(theCurrentViewport);
                        m_Renderer.RenderQuad(QVector2D((float)theLayerViewport.m_Width,
                                                        (float)theLayerViewport.m_Height),
                                              theFinalMVP, *theLayerColorTexture);
                    }
                } else {
                    // No advanced blending SW fallback needed
                    m_Renderer.RenderQuad(QVector2D((float)theLayerViewport.m_Width,
                                                    (float)theLayerViewport.m_Height),
                                          theFinalMVP, *theLayerColorTexture);
                }
#else
                m_Renderer.RenderQuad(QVector2D((float)theLayerViewport.m_Width,
                                                (float)theLayerViewport.m_Height),
                                      theFinalMVP, *theLayerColorTexture);
#endif
            }
            if (m_LayerWidgetTexture) {
                theContext.SetBlendingEnabled(false);
                m_Renderer.SetupWidgetLayer();
                SLayerRenderPreparationResult &thePrepResult(*m_LayerPrepResult);
                QDemonRenderRectF thePresRect(thePrepResult.GetPresentationViewport());
                QDemonRenderRectF theLayerRect(thePrepResult.GetLayerToPresentationViewport());

                // Ensure we remove any offsetting in the layer rect that was caused simply by
                // the
                // presentation rect offsetting but then use a new rect.
                QDemonRenderRectF theWidgetLayerRect(theLayerRect.m_X - thePresRect.m_X,
                                                     theLayerRect.m_Y - thePresRect.m_Y,
                                                     theLayerRect.m_Width, theLayerRect.m_Height);
                theContext.SetScissorTestEnabled(false);
                theContext.SetViewport(theWidgetLayerRect.ToIntegerRect());
                m_Renderer.RenderQuad(
                            QVector2D((float)theLayerViewport.m_Width, (float)theLayerViewport.m_Height),
                            theFinalMVP, *m_LayerWidgetTexture);
            }
        }
    } // End offscreen render code.

    if (m_BoundingRectColor.hasValue()) {
        QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                    theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);
        QDemonRenderContextScopedProperty<bool> theScissorEnabled(
                    theContext, &QDemonRenderContext::IsScissorTestEnabled,
                    &QDemonRenderContext::SetScissorTestEnabled);
        QDemonRenderContextScopedProperty<QDemonRenderRect> theScissorRect(
                    theContext, &QDemonRenderContext::GetScissorRect, &QDemonRenderContext::SetScissorRect);
        m_Renderer.SetupWidgetLayer();
        // Setup a simple viewport to render to the entire presentation viewport.
        theContext.SetViewport(
                    QDemonRenderRect(0, 0, (quint32)thePrepResult.GetPresentationViewport().m_Width,
                                     (quint32)thePrepResult.GetPresentationViewport().m_Height));

        QDemonRenderRectF thePresRect(thePrepResult.GetPresentationViewport());

        // Remove any offsetting from the presentation rect since the widget layer is a
        // stand-alone fbo.
        QDemonRenderRectF theWidgetScreenRect(theScreenRect.m_X - thePresRect.m_X,
                                              theScreenRect.m_Y - thePresRect.m_Y,
                                              theScreenRect.m_Width, theScreenRect.m_Height);
        theContext.SetScissorTestEnabled(false);
        m_Renderer.DrawScreenRect(theWidgetScreenRect, *m_BoundingRectColor);
    }
    theContext.SetBlendFunction(QDemonRenderBlendFunctionArgument(
                                    QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                                    QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha));
    theContext.SetBlendEquation(QDemonRenderBlendEquationArgument(
                                    QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add));
}

void SLayerRenderData::AddLayerRenderStep()
{
    SStackPerfTimer __perfTimer(m_Renderer.GetQt3DSContext().GetPerfTimer(),
                                "SLayerRenderData::AddLayerRenderStep");
    Q_ASSERT(m_Camera);
    if (!m_Camera)
        return;

    IRenderList &theGraph(m_Renderer.GetQt3DSContext().GetRenderList());

    QDemonRenderRect theCurrentViewport = theGraph.GetViewport();
    if (!m_LayerPrepResult.hasValue())
        PrepareForRender(
                    QSize(theCurrentViewport.m_Width, theCurrentViewport.m_Height));
}

void SLayerRenderData::PrepareForRender()
{
    // When we render to the scene itself (as opposed to an offscreen buffer somewhere)
    // then we use the MVP of the layer somewhat.
    QDemonRenderRect theViewport = m_Renderer.GetQt3DSContext().GetRenderList().GetViewport();
    PrepareForRender(
                QSize((quint32)theViewport.m_Width, (quint32)theViewport.m_Height));
}

void SLayerRenderData::ResetForFrame()
{
    SLayerRenderPreparationData::ResetForFrame();
    m_BoundingRectColor.setEmpty();
}

void SLayerRenderData::PrepareAndRender(const QMatrix4x4 &inViewProjection)
{
    TRenderableObjectList theTransparentObjects(m_TransparentObjects);
    TRenderableObjectList theOpaqueObjects(m_OpaqueObjects);
    theTransparentObjects.clear();
    theOpaqueObjects.clear();
    m_ModelContexts.clear();
    SLayerRenderPreparationResultFlags theFlags;
    PrepareRenderablesForRender(inViewProjection, Empty(), 1.0, theFlags);
    RenderDepthPass(false);
    Render();
}

struct SLayerRenderToTextureRunnable : public IRenderTask
{
    SLayerRenderData &m_Data;
    SLayerRenderToTextureRunnable(SLayerRenderData &d)
        : m_Data(d)
    {
    }

    void Run() override { m_Data.RenderToTexture(); }
};

static inline OffscreenRendererDepthValues::Enum
GetOffscreenRendererDepthValue(QDemonRenderTextureFormats::Enum inBufferFormat)
{
    switch (inBufferFormat) {
    case QDemonRenderTextureFormats::Depth32:
        return OffscreenRendererDepthValues::Depth32;
    case QDemonRenderTextureFormats::Depth24:
        return OffscreenRendererDepthValues::Depth24;
    case QDemonRenderTextureFormats::Depth24Stencil8:
        return OffscreenRendererDepthValues::Depth24;
    default:
        Q_ASSERT(false); // fallthrough intentional
    case QDemonRenderTextureFormats::Depth16:
        return OffscreenRendererDepthValues::Depth16;
    }
}

SOffscreenRendererEnvironment SLayerRenderData::CreateOffscreenRenderEnvironment()
{
    OffscreenRendererDepthValues::Enum theOffscreenDepth(
                GetOffscreenRendererDepthValue(GetDepthBufferFormat()));
    QDemonRenderRect theViewport = m_Renderer.GetQt3DSContext().GetRenderList().GetViewport();
    return SOffscreenRendererEnvironment(theViewport.m_Width, theViewport.m_Height,
                                         QDemonRenderTextureFormats::RGBA8, theOffscreenDepth,
                                         false, AAModeValues::NoAA);
}

IRenderTask &SLayerRenderData::CreateRenderToTextureRunnable()
{
    return *new SLayerRenderToTextureRunnable(*this);
}

QT_END_NAMESPACE
