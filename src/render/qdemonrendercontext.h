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
#ifndef QDEMON_RENDER_CONTEXT_H
#define QDEMON_RENDER_CONTEXT_H

#include <QtDemon/QDemonDataRef>

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonglimplobjects.h>
#include <QtDemonRender/qdemonrenderbackendgles2.h>
#include <QtDemonRender/qdemonrenderbackendgl3.h>
#include <QtDemonRender/qdemonrenderbackendgl4.h>
#include <QtDemonRender/qdemonrenderbackendnull.h>
#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtDemonRender/qdemonrenderindexbuffer.h>
#include <QtDemonRender/qdemonrenderconstantbuffer.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrenderdepthstencilstate.h>
#include <QtDemonRender/qdemonrenderrasterizerstate.h>
#include <QtDemonRender/qdemonrenderdrawable.h>
#include <QtDemonRender/qdemonrenderinputassembler.h>
#include <QtDemonRender/qdemonrenderattriblayout.h>
#include <QtDemonRender/qdemonrenderimagetexture.h>
#include <QtDemonRender/qdemonrenderocclusionquery.h>
#include <QtDemonRender/qdemonrendertimerquery.h>
#include <QtDemonRender/qdemonrendersync.h>
#include <QtDemonRender/qdemonrendertexture2darray.h>
#include <QtDemonRender/qdemonrendertexturecube.h>
#include <QtDemonRender/qdemonrenderstoragebuffer.h>
#include <QtDemonRender/qdemonrenderatomiccounterbuffer.h>
#include <QtDemonRender/qdemonrenderdrawindirectbuffer.h>
#include <QtDemonRender/qdemonrenderpathrender.h>
#include <QtDemonRender/qdemonrenderpathspecification.h>
#include <QtDemonRender/qdemonrenderpathfontspecification.h>
#include <QtDemonRender/qdemonrenderpathfonttext.h>

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QSharedPointer>

#include <QtGui/QSurfaceFormat>

QT_BEGIN_NAMESPACE

// When SW fallback is defined we can support (some) object/layer advanced blend modes. If defined,
// the HW implementation is still preperred if available through extensions. SW fallback can't be
// used in custom shaders.
#define ADVANCED_BLEND_SW_FALLBACK

enum class QDemonRenderShaderProgramBinaryType
{
    Unknown = 0,
    NVBinary = 1,
};

// context dirty flags
enum class QDemonRenderContextDirtyValues
{
    InputAssembler = 1 << 0,
};

Q_DECLARE_FLAGS(QDemonRenderContextDirtyFlags, QDemonRenderContextDirtyValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderContextDirtyFlags)

typedef QHash<QByteArray, QDemonRenderConstantBuffer *> TContextConstantBufferMap;
typedef QHash<QByteArray, QDemonRenderStorageBuffer *> TContextStorageBufferMap;
typedef QHash<QByteArray, QDemonRenderAtomicCounterBuffer *> TContextAtomicCounterBufferMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendBufferObject, QDemonRenderDrawIndirectBuffer *> TContextDrawIndirectBufferMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject, QDemonRenderDepthStencilState *> TContextDepthStencilStateMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject, QDemonRenderRasterizerState *> TContextRasterizerStateMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendTextureObject, QDemonRenderTexture2DArray *> TContextTex2DArrayToImpMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendTextureObject, QDemonRenderTextureCube *> TContextTexCubeToImpMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendTextureObject, QDemonRenderImage2D *> TContextImage2DToImpMap;
typedef QHash<QString, QDemonRenderPathFontSpecification *> TContextPathFontSpecificationMap;

class QDemonRenderProgramPipeline;

// Now for scoped property access.
template<typename TDataType>
struct QDemonRenderContextScopedProperty : public QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>
{
    typedef typename QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>::TGetter TGetter;
    typedef typename QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>::TSetter TSetter;
    QDemonRenderContextScopedProperty(QDemonRenderContext &ctx, TGetter getter, TSetter setter)
        : QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>(ctx, getter, setter)
    {
    }
    QDemonRenderContextScopedProperty(QDemonRenderContext &ctx, TGetter getter, TSetter setter, const TDataType &inNewValue)
        : QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>(ctx, getter, setter, inNewValue)
    {
    }
};

// TODO: Get rid of this, or at least make it more explicit, i.e, don't assume any patterns.
#define ITERATE_HARDWARE_CONTEXT_PROPERTIES                                                                            \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(RenderTarget, frameBuffer)                                                        \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ActiveShader, activeShader)                                                       \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ActiveProgramPipeline, activeProgramPipeline)                                     \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(InputAssembler, inputAssembler)                                                   \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(BlendFunction, blendFunction)                                                     \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(CullingEnabled, cullingEnabled)                                                   \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(DepthFunction, depthFunction)                                                     \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(BlendingEnabled, blendingEnabled)                                                 \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(DepthWriteEnabled, depthWriteEnabled)                                             \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(DepthTestEnabled, depthTestEnabled)                                               \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(StencilTestEnabled, stencilTestEnabled)                                           \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ScissorTestEnabled, scissorTestEnabled)                                           \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ScissorRect, scissorRect)                                                         \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(Viewport, viewport)                                                               \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ClearColor, clearColor)

class Q_DEMONRENDER_EXPORT QDemonRenderContext : public QDemonRenderDrawable
{
    Q_DISABLE_COPY(QDemonRenderContext)
public:
    QAtomicInt ref;
    // these variables represent the current hardware state of the render context.
    QDemonGLHardPropertyContext m_hardwarePropertyContext;

private:
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to our render backend
    QDemonRenderContextDirtyFlags m_dirtyFlags; ///< context dirty flags

    QDemonRenderBackend::QDemonRenderBackendRenderTargetObject m_defaultOffscreenRenderTarget; ///< this is a special target set from outside if we
    /// never render to a window directly (GL only)
    qint32 m_dephBits; ///< this is the depth bits count of the default window render target
    qint32 m_stencilBits; ///< this is the stencil bits count of the default window render target

protected:
    QHash<const void *, QDemonRenderVertexBuffer *> m_vertToImpMap;
    QHash<const void *, QDemonRenderIndexBuffer *> m_indexToImpMap;
    TContextConstantBufferMap m_constantToImpMap;
    TContextStorageBufferMap m_storageToImpMap;
    TContextAtomicCounterBufferMap m_atomicCounterToImpMap;
    TContextDrawIndirectBufferMap m_drawIndirectToImpMap;
    TContextDepthStencilStateMap m_depthStencilStateToImpMap;
    TContextRasterizerStateMap m_rasterizerStateToImpMap;
    TContextPathFontSpecificationMap m_pathFontSpecToImpMap;

    QHash<const void *, QDemonRenderTexture2D *> m_tex2DToImpMap;
    TContextTex2DArrayToImpMap m_tex2DArrayToImpMap;
    TContextTexCubeToImpMap m_texCubeToImpMap;
    TContextImage2DToImpMap m_image2DtoImpMap;
    QHash<const void *, QDemonRenderShaderProgram *> m_shaderToImpMap;
    QHash<const void *, QDemonRenderRenderBuffer *> m_renderBufferToImpMap;
    QHash<const void *, QDemonRenderFrameBuffer *> m_frameBufferToImpMap;
    qint32 m_maxTextureUnits;
    qint32 m_nextTextureUnit;
    qint32 m_maxConstantBufferUnits;
    qint32 m_nextConstantBufferUnit;

    QVector<QDemonGLHardPropertyContext> m_propertyStack;

    void doSetClearColor(QVector4D inClearColor)
    {
        m_hardwarePropertyContext.m_clearColor = inClearColor;
        m_backend->setClearColor(&inClearColor);
    }

    void doSetBlendFunction(QDemonRenderBlendFunctionArgument inFunctions)
    {
        qint32_4 values;
        m_hardwarePropertyContext.m_blendFunction = inFunctions;

        m_backend->setBlendFunc(inFunctions);
    }

    void doSetBlendEquation(QDemonRenderBlendEquationArgument inEquations)
    {
        qint32_4 values;
        m_hardwarePropertyContext.m_blendEquation = inEquations;

        m_backend->setBlendEquation(inEquations);
    }

    void doSetCullingEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_cullingEnabled = inEnabled;
        m_backend->setRenderState(inEnabled, QDemonRenderState::CullFace);
    }

    void doSetDepthFunction(QDemonRenderBoolOp inFunction)
    {
        m_hardwarePropertyContext.m_depthFunction = inFunction;
        m_backend->setDepthFunc(inFunction);
    }

    void doSetBlendingEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_blendingEnabled = inEnabled;
        m_backend->setRenderState(inEnabled, QDemonRenderState::Blend);
    }

    void doSetColorWritesEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_colorWritesEnabled = inEnabled;
        m_backend->setColorWrites(inEnabled, inEnabled, inEnabled, inEnabled);
    }

    void doSetMultisampleEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_multisampleEnabled = inEnabled;
        m_backend->setMultisample(inEnabled);
    }

    void doSetDepthWriteEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_depthWriteEnabled = inEnabled;
        m_backend->setDepthWrite(inEnabled);
    }

    void doSetDepthTestEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_depthTestEnabled = inEnabled;
        m_backend->setRenderState(inEnabled, QDemonRenderState::DepthTest);
    }

    void doSetStencilTestEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_stencilTestEnabled = inEnabled;
        m_backend->setRenderState(inEnabled, QDemonRenderState::StencilTest);
    }

    void doSetScissorTestEnabled(bool inEnabled)
    {
        m_hardwarePropertyContext.m_scissorTestEnabled = inEnabled;
        m_backend->setRenderState(inEnabled, QDemonRenderState::ScissorTest);
    }

    void doSetScissorRect(QRect inRect)
    {
        m_hardwarePropertyContext.m_scissorRect = inRect;
        m_backend->setScissorRect(inRect);
    }

    void doSetViewport(QRect inViewport)
    {
        m_hardwarePropertyContext.m_viewport = inViewport;
        m_backend->setViewportRect(inViewport);
    }

    // Circular dependencies between shader constants and shader programs preclude
    // implementation in header
    void doSetActiveShader(const QDemonRef<QDemonRenderShaderProgram> &inShader);
    void doSetActiveProgramPipeline(const QDemonRef<QDemonRenderProgramPipeline> &inProgramPipeline);

    void doSetInputAssembler(QDemonRef<QDemonRenderInputAssembler> inAssembler)
    {
        m_hardwarePropertyContext.m_inputAssembler = inAssembler;
        m_dirtyFlags |= QDemonRenderContextDirtyValues::InputAssembler;
    }

    void doSetRenderTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer)
    {
        if (inBuffer)
            m_backend->setRenderTarget(inBuffer->handle());
        else
            m_backend->setRenderTarget(m_defaultOffscreenRenderTarget);

        m_hardwarePropertyContext.m_frameBuffer = inBuffer;
    }

    void doSetReadTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer)
    {
        if (inBuffer)
            m_backend->setReadTarget(inBuffer->handle());
        else
            m_backend->setReadTarget(QDemonRenderBackend::QDemonRenderBackendRenderTargetObject(nullptr));
    }

    bool bindShaderToInputAssembler(const QDemonRef<QDemonRenderInputAssembler> &inputAssembler,
                                    const QDemonRef<QDemonRenderShaderProgram> &shader);
    bool applyPreDrawProperties();
    void onPostDraw();

public:
    QDemonRenderContext(const QDemonRef<QDemonRenderBackend> &inBackend);
    ~QDemonRenderContext() override;

    QDemonRef<QDemonRenderBackend> getBackend() { return m_backend; }

    void getMaxTextureSize(qint32 &oWidth, qint32 &oHeight);

    const char *getShadingLanguageVersion() { return m_backend->getShadingLanguageVersion(); }

    QDemonRenderContextType getRenderContextType() const { return m_backend->getRenderContextType(); }

    qint32 getDepthBits() const
    {
        // only query this if a framebuffer is bound
        if (m_hardwarePropertyContext.m_frameBuffer)
            return m_backend->getDepthBits();
        else
            return m_dephBits;
    }

    qint32 getStencilBits() const
    {
        // only query this if a framebuffer is bound
        if (m_hardwarePropertyContext.m_frameBuffer)
            return m_backend->getStencilBits();
        else
            return m_stencilBits;
    }

    bool getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps inCap) const
    {
        return m_backend->getRenderBackendCap(inCap);
    }

    bool areMultisampleTexturesSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::MsTexture);
    }

    bool getConstantBufferSupport() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ConstantBuffer);
    }

    bool areDXTImagesSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::DxtImages);
    }

    bool isDepthStencilSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::DepthStencilTexture);
    }

    bool isFpRenderTargetSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::FpRenderTarget);
    }

    bool isTessellationSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Tessellation);
    }

    bool isGeometryStageSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Geometry);
    }

    bool isComputeSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Compute);
    }

    bool isSampleQuerySupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::SampleQuery);
    }

    bool isTimerQuerySupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TimerQuery);
    }

    bool isCommandSyncSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::CommandSync);
    }
    bool isTextureArraySupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TextureArray);
    }
    bool isStorageBufferSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::StorageBuffer);
    }
    bool isAtomicCounterBufferSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AtomicCounterBuffer);
    }
    bool isShaderImageLoadStoreSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ShaderImageLoadStore);
    }
    bool isProgramPipelineSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ProgramPipeline);
    }
    bool isPathRenderingSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::PathRendering);
    }
    // Are blend modes really supported in HW?
    bool isAdvancedBlendHwSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AdvancedBlend);
    }
    bool isAdvancedBlendHwSupportedKHR() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AdvancedBlendKHR);
    }
    bool isBlendCoherencySupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::BlendCoherency);
    }
    bool isStandardDerivativesSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::StandardDerivatives);
    }
    bool isTextureLodSupported() const
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TextureLod);
    }

    void setDefaultRenderTarget(quint64 targetID)
    {
        m_defaultOffscreenRenderTarget = reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendRenderTargetObject>(targetID);
    }

    void setDefaultDepthBufferBitCount(qint32 depthBits) { m_dephBits = depthBits; }

    void setDepthStencilState(QDemonRef<QDemonRenderDepthStencilState> inDepthStencilState);

    QDemonRef<QDemonRenderRasterizerState> createRasterizerState(float depthBias, float depthScale, QDemonRenderFace cullFace);
    void setRasterizerState(QDemonRef<QDemonRenderRasterizerState> inRasterizerState);
    void stateDestroyed(QDemonRenderRasterizerState *state);

    QDemonRef<QDemonRenderVertexBuffer> createVertexBuffer(QDemonRenderBufferUsageType usageType,
                                                           size_t size,
                                                           quint32 stride,
                                                           QDemonConstDataRef<quint8> bufferData);
    QDemonRef<QDemonRenderVertexBuffer> getVertexBuffer(const void *implementationHandle);
    void bufferDestroyed(QDemonRenderVertexBuffer *buffer);

    QDemonRef<QDemonRenderIndexBuffer> createIndexBuffer(QDemonRenderBufferUsageType usageType,
                                                                 QDemonRenderComponentType componentType,
                                                                 size_t size,
                                                                 QDemonConstDataRef<quint8> bufferData);
    QDemonRef<QDemonRenderIndexBuffer> getIndexBuffer(const void *implementationHandle);
    void bufferDestroyed(QDemonRenderIndexBuffer *buffer);

    QDemonRef<QDemonRenderConstantBuffer> createConstantBuffer(const char *bufferName,
                                                                       QDemonRenderBufferUsageType usageType,
                                                                       size_t size,
                                                                       QDemonConstDataRef<quint8> bufferData);
    QDemonRef<QDemonRenderConstantBuffer> getConstantBuffer(const QByteArray &bufferName);
    void bufferDestroyed(QDemonRenderConstantBuffer *buffer);

    qint32 getNextConstantBufferUnit();

    QDemonRef<QDemonRenderStorageBuffer> createStorageBuffer(const char *bufferName,
                                                                     QDemonRenderBufferUsageType usageType,
                                                                     size_t size,
                                                                     QDemonConstDataRef<quint8> bufferData,
                                                                     QDemonRenderDataBuffer *pBuffer);
    QDemonRef<QDemonRenderStorageBuffer> getStorageBuffer(const QByteArray &bufferName);
    void bufferDestroyed(QDemonRenderStorageBuffer *buffer);

    QDemonRef<QDemonRenderAtomicCounterBuffer> createAtomicCounterBuffer(const char *bufferName,
                                                                                 QDemonRenderBufferUsageType usageType,
                                                                                 size_t size,
                                                                                 QDemonConstDataRef<quint8> bufferData);
    QDemonRef<QDemonRenderAtomicCounterBuffer> getAtomicCounterBuffer(const QByteArray &bufferName);
    QDemonRef<QDemonRenderAtomicCounterBuffer> getAtomicCounterBufferByParam(const QByteArray &paramName);
    void bufferDestroyed(QDemonRenderAtomicCounterBuffer *buffer);

    QDemonRef<QDemonRenderDrawIndirectBuffer> createDrawIndirectBuffer(QDemonRenderBufferUsageType usageType,
                                                                               size_t size,
                                                                               QDemonConstDataRef<quint8> bufferData);
    QDemonRef<QDemonRenderDrawIndirectBuffer> getDrawIndirectBuffer(QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle);
    void bufferDestroyed(QDemonRenderDrawIndirectBuffer *buffer);

    void setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers);

    QDemonRef<QDemonRenderOcclusionQuery> createOcclusionQuery();
    QDemonRef<QDemonRenderTimerQuery> createTimerQuery();
    QDemonRef<QDemonRenderSync> createSync();

    QDemonRef<QDemonRenderTexture2D> createTexture2D();
    QDemonRef<QDemonRenderTexture2D> getTexture2D(const void *implementationHandle);
    void textureDestroyed(QDemonRenderTexture2D *buffer);

    QDemonRef<QDemonRenderTexture2DArray> createTexture2DArray();
    void textureDestroyed(QDemonRenderTexture2DArray *buffer);

    QDemonRef<QDemonRenderTextureCube> createTextureCube();
    void textureDestroyed(QDemonRenderTextureCube *buffer);

    qint32 getNextTextureUnit();

    QDemonRef<QDemonRenderImage2D> createImage2D(QDemonRef<QDemonRenderTexture2D> inTexture,
                                                 QDemonRenderImageAccessType inAccess);
    void imageDestroyed(QDemonRenderImage2D *buffer);

    QDemonRef<QDemonRenderRenderBuffer> createRenderBuffer(QDemonRenderRenderBufferFormat bufferFormat,
                                                                   quint32 width,
                                                                   quint32 height);
    QDemonRef<QDemonRenderRenderBuffer> getRenderBuffer(const void *implementationHandle);
    void renderBufferDestroyed(QDemonRenderRenderBuffer *buffer);

    QDemonRef<QDemonRenderFrameBuffer> createFrameBuffer();
    QDemonRef<QDemonRenderFrameBuffer> getFrameBuffer(const void *implementationHandle);
    void frameBufferDestroyed(QDemonRenderFrameBuffer *fb);

    QDemonRef<QDemonRenderAttribLayout> createAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs);
    QDemonRef<QDemonRenderInputAssembler> createInputAssembler(QDemonRef<QDemonRenderAttribLayout> attribLayout,
                                                               QDemonConstDataRef<QDemonRef<QDemonRenderVertexBuffer>> buffers,
                                                               const QDemonRef<QDemonRenderIndexBuffer> indexBuffer,
                                                               QDemonConstDataRef<quint32> strides,
                                                               QDemonConstDataRef<quint32> offsets,
                                                               QDemonRenderDrawMode primType = QDemonRenderDrawMode::Triangles,
                                                               quint32 patchVertexCount = 1);
    void setInputAssembler(QDemonRef<QDemonRenderInputAssembler> inputAssembler);

    QDemonRenderVertFragCompilationResult compileSource(
            const char *shaderName,
            QDemonConstDataRef<qint8> vertShader,
            QDemonConstDataRef<qint8> fragShader,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType type = QDemonRenderShaderProgramBinaryType::Unknown,
            bool binaryProgram = false);

    // You must figure out inVertLen and inFragLen yourself, this object doesn't do that.
    QDemonRenderVertFragCompilationResult compileSource(const char *shaderName,
                                                        const char *vertShader,
                                                        quint32 inVertLen,
                                                        const char *fragShader,
                                                        quint32 inFragLen,
                                                        const char * = nullptr,
                                                        quint32 inTCLen = 0,
                                                        const char *tessEvaluationShaderSource = nullptr,
                                                        quint32 inTELen = 0,
                                                        const char *geometryShaderSource = nullptr,
                                                        quint32 inGSLen = 0,
                                                        bool separableProgram = false);

    QDemonRenderVertFragCompilationResult compileBinary(
            const char *shaderName,
            QDemonRenderShaderProgramBinaryType type,
            QDemonDataRef<qint8> vertShader,
            QDemonDataRef<qint8> fragShader,
            QDemonDataRef<qint8> tessControlShaderSource = QDemonDataRef<qint8>(),
            QDemonDataRef<qint8> tessEvaluationShaderSource = QDemonDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>());

    QDemonRenderVertFragCompilationResult compileComputeSource(const char *shaderName,
                                                                       QDemonConstDataRef<qint8> computeShaderSource);

    QDemonRef<QDemonRenderShaderProgram> getShaderProgram(const void *implementationHandle);
    void shaderDestroyed(QDemonRenderShaderProgram *shader);

    QDemonRef<QDemonRenderProgramPipeline> createProgramPipeline();
    QDemonRef<QDemonRenderPathSpecification> createPathSpecification();
    QDemonRef<QDemonRenderPathRender> createPathRender(size_t range = 1);
    void setPathProjectionMatrix(const QMatrix4x4 inPathProjection);
    void setPathModelViewMatrix(const QMatrix4x4 inPathModelview);
    void setPathStencilDepthOffset(float inSlope, float inBias);
    void setPathCoverDepthFunc(QDemonRenderBoolOp inFunc);

    QDemonRef<QDemonRenderPathFontSpecification> createPathFontSpecification(const QString &fontName);
    void releasePathFontSpecification(QDemonRenderPathFontSpecification *inPathSpec);
    QDemonRef<QDemonRenderPathFontItem> createPathFontItem();

    void setClearColor(QVector4D inClearColor);
    QVector4D getClearColor() const { return m_hardwarePropertyContext.m_clearColor; }

    void setBlendFunction(QDemonRenderBlendFunctionArgument inFunctions);
    QDemonRenderBlendFunctionArgument getBlendFunction() const
    {
        return m_hardwarePropertyContext.m_blendFunction;
    }

    void setBlendEquation(QDemonRenderBlendEquationArgument inEquations);
    QDemonRenderBlendEquationArgument getBlendEquation() const
    {
        return m_hardwarePropertyContext.m_blendEquation;
    }

    void setCullingEnabled(bool inEnabled);
    bool isCullingEnabled() const { return m_hardwarePropertyContext.m_cullingEnabled; }

    void setDepthFunction(QDemonRenderBoolOp inFunction);
    QDemonRenderBoolOp getDepthFunction() const { return m_hardwarePropertyContext.m_depthFunction; }

    void setBlendingEnabled(bool inEnabled);
    bool isBlendingEnabled() const { return m_hardwarePropertyContext.m_blendingEnabled; }

    void setDepthWriteEnabled(bool inEnabled);
    bool isDepthWriteEnabled() const { return m_hardwarePropertyContext.m_depthWriteEnabled; }
    void setDepthTestEnabled(bool inEnabled);
    bool isDepthTestEnabled() const { return m_hardwarePropertyContext.m_depthTestEnabled; }

    void setStencilTestEnabled(bool inEnabled);
    bool isStencilTestEnabled() const { return m_hardwarePropertyContext.m_stencilTestEnabled; }

    void setScissorTestEnabled(bool inEnabled);
    bool isScissorTestEnabled() const { return m_hardwarePropertyContext.m_scissorTestEnabled; }
    void setScissorRect(QRect inRect);
    QRect getScissorRect() const { return m_hardwarePropertyContext.m_scissorRect; }

    void setViewport(QRect inViewport);
    QRect getViewport() const { return m_hardwarePropertyContext.m_viewport; }

    void setColorWritesEnabled(bool inEnabled);
    bool isColorWritesEnabled() const { return m_hardwarePropertyContext.m_colorWritesEnabled; }

    void setMultisampleEnabled(bool inEnabled);
    bool isMultisampleEnabled() const { return m_hardwarePropertyContext.m_multisampleEnabled; }

    void setActiveShader(QDemonRef<QDemonRenderShaderProgram> inShader);
    QDemonRef<QDemonRenderShaderProgram> getActiveShader() const;

    void setActiveProgramPipeline(QDemonRef<QDemonRenderProgramPipeline> inProgramPipeline);
    QDemonRef<QDemonRenderProgramPipeline> getActiveProgramPipeline() const;

    void dispatchCompute(QDemonRef<QDemonRenderShaderProgram> inShader, quint32 numGroupsX, quint32 numGroupsY, quint32 numGroupsZ);

    void setDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet);
    void setReadBuffer(QDemonReadFace inReadFace);

    void readPixels(QRect inRect, QDemonRenderReadPixelFormat inFormat, QDemonDataRef<quint8> inWriteBuffer);

    void setRenderTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer);
    void setReadTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer);
    QDemonRef<QDemonRenderFrameBuffer> getRenderTarget() const
    {
        return m_hardwarePropertyContext.m_frameBuffer;
    }

    void resetBlendState();

    // Push the entire set of properties.
    void pushPropertySet();

    // Pop the entire set of properties, potentially forcing the values
    // to opengl.
    void popPropertySet(bool inForceSetProperties);

    // clear current bound render target
    void clear(QDemonRenderClearFlags flags);
    // clear passed in rendertarget
    void clear(QDemonRef<QDemonRenderFrameBuffer> fb, QDemonRenderClearFlags flags);

    // copy framebuffer content between read target and render target
    void blitFramebuffer(qint32 srcX0,
                         qint32 srcY0,
                         qint32 srcX1,
                         qint32 srcY1,
                         qint32 dstX0,
                         qint32 dstY0,
                         qint32 dstX1,
                         qint32 dstY1,
                         QDemonRenderClearFlags flags,
                         QDemonRenderTextureMagnifyingOp filter);

    void draw(QDemonRenderDrawMode drawMode, quint32 count, quint32 offset) override;
    void drawIndirect(QDemonRenderDrawMode drawMode, quint32 offset);

    QSurfaceFormat format() const { return m_backend->format(); }
    void resetStates()
    {
        pushPropertySet();
        popPropertySet(true);
    }

    // Used during layer rendering because we can't set the *actual* viewport to what it should
    // be due to hardware problems.
    // Set during begin render.
    static QMatrix4x4 applyVirtualViewportToProjectionMatrix(const QMatrix4x4 &inProjection,
                                                             const QRectF &inViewport,
                                                             const QRectF &inVirtualViewport);

    static QDemonRef<QDemonRenderContext> createGl(const QSurfaceFormat &format);

    static QDemonRef<QDemonRenderContext> createNull();
};

QT_END_NAMESPACE

#endif
