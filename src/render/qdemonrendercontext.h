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

struct QDemonRenderShaderProgramBinaryType
{
    enum Enum {
        Unknown = 0,
        NVBinary = 1,
    };
};

// context dirty flags
struct QDemonRenderContextDirtyValues
{
    enum Enum {
        InputAssembler = 1 << 0,
    };
};

typedef QDemonFlags<QDemonRenderContextDirtyValues::Enum, quint32> QDemonRenderContextDirtyFlags;
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

class Q_DEMONRENDER_EXPORT QDemonRenderContext : public QDemonRenderDrawable
{
public:
    QAtomicInt ref;
    virtual QDemonRenderContextType getRenderContextType() const = 0;
    virtual bool areMultisampleTexturesSupported() const = 0;
    virtual bool getConstantBufferSupport() const = 0;
    virtual void getMaxTextureSize(qint32 &oWidth, qint32 &oHeight) = 0;
    virtual const char *getShadingLanguageVersion() = 0;
    // Get the bit depth of the currently bound depth buffer.
    virtual qint32 getDepthBits() const = 0;
    virtual qint32 getStencilBits() const = 0;
    virtual bool getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Enum inCap) const = 0;
    virtual bool areDXTImagesSupported() const = 0;
    virtual bool isDepthStencilSupported() const = 0;
    virtual bool isFpRenderTargetSupported() const = 0;
    virtual bool isTessellationSupported() const = 0;
    virtual bool isGeometryStageSupported() const = 0;
    virtual bool isComputeSupported() const = 0;
    virtual bool isSampleQuerySupported() const = 0;
    virtual bool isTimerQuerySupported() const = 0;
    virtual bool isCommandSyncSupported() const = 0;
    virtual bool isTextureArraySupported() const = 0;
    virtual bool isStorageBufferSupported() const = 0;
    virtual bool isAtomicCounterBufferSupported() const = 0;
    virtual bool isShaderImageLoadStoreSupported() const = 0;
    virtual bool isProgramPipelineSupported() const = 0;
    virtual bool isPathRenderingSupported() const = 0;
    virtual bool isBlendCoherencySupported() const = 0;
    virtual bool isAdvancedBlendHwSupported() const = 0;
    virtual bool isAdvancedBlendHwSupportedKHR() const = 0;
    virtual bool isStandardDerivativesSupported() const = 0;
    virtual bool isTextureLodSupported() const = 0;

    virtual void setDefaultRenderTarget(quint64 targetID) = 0;
    virtual void setDefaultDepthBufferBitCount(qint32 depthBits) = 0;

    virtual QDemonRef<QDemonRenderDepthStencilState> createDepthStencilState(bool enableDepth,
                                                                             bool depthMask,
                                                                             QDemonRenderBoolOp::Enum depthFunc,
                                                                             bool enableStencil,
                                                                             QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                                                             QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                                                             QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                                                             QDemonRenderStencilOperationArgument &depthStencilOpBack) = 0;

    virtual QDemonRef<QDemonRenderRasterizerState> createRasterizerState(float depthBias,
                                                                         float depthScale,
                                                                         QDemonRenderFaces::Enum cullFace) = 0;

    virtual QDemonRef<QDemonRenderVertexBuffer> createVertexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                                   size_t size,
                                                                   quint32 stride = 0,
                                                                   QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) = 0;
    virtual QDemonRef<QDemonRenderVertexBuffer> getVertexBuffer(const void *implementationHandle) = 0;

    virtual QDemonRef<QDemonRenderIndexBuffer> createIndexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                                 QDemonRenderComponentTypes::Enum componentType,
                                                                 size_t size,
                                                                 QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) = 0;
    virtual QDemonRef<QDemonRenderIndexBuffer> getIndexBuffer(const void *implementationHandle) = 0;

    virtual QDemonRef<QDemonRenderConstantBuffer> createConstantBuffer(const char *bufferName,
                                                                       QDemonRenderBufferUsageType::Enum usageType,
                                                                       size_t size,
                                                                       QDemonConstDataRef<quint8> bufferData) = 0;
    virtual QDemonRef<QDemonRenderConstantBuffer> getConstantBuffer(const QByteArray &bufferName) = 0;

    virtual QDemonRef<QDemonRenderStorageBuffer> createStorageBuffer(const char *bufferName,
                                                                     QDemonRenderBufferUsageType::Enum usageType,
                                                                     size_t size,
                                                                     QDemonConstDataRef<quint8> bufferData,
                                                                     QDemonRenderDataBuffer *pBuffer) = 0;
    virtual QDemonRef<QDemonRenderStorageBuffer> getStorageBuffer(const QByteArray &bufferName) = 0;

    virtual QDemonRef<QDemonRenderAtomicCounterBuffer> createAtomicCounterBuffer(const char *bufferName,
                                                                                 QDemonRenderBufferUsageType::Enum usageType,
                                                                                 size_t size,
                                                                                 QDemonConstDataRef<quint8> bufferData) = 0;
    virtual QDemonRef<QDemonRenderAtomicCounterBuffer> getAtomicCounterBuffer(const QByteArray &bufferName) = 0;
    virtual QDemonRef<QDemonRenderAtomicCounterBuffer> getAtomicCounterBufferByParam(const QByteArray &paramName) = 0;

    virtual QDemonRef<QDemonRenderDrawIndirectBuffer> createDrawIndirectBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                                               size_t size,
                                                                               QDemonConstDataRef<quint8> bufferData) = 0;
    virtual QDemonRef<QDemonRenderDrawIndirectBuffer> getDrawIndirectBuffer(
            QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle) = 0;

    virtual void setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) = 0;

    virtual QDemonRef<QDemonRenderOcclusionQuery> createOcclusionQuery() = 0;
    virtual QDemonRef<QDemonRenderTimerQuery> createTimerQuery() = 0;
    virtual QDemonRef<QDemonRenderSync> createSync() = 0;

    virtual QDemonRef<QDemonRenderTexture2D> createTexture2D() = 0;
    virtual QDemonRef<QDemonRenderTexture2D> getTexture2D(const void *implementationHandle) = 0;
    virtual QDemonRef<QDemonRenderBackend> getBackend() = 0;

    virtual QDemonRef<QDemonRenderTexture2DArray> createTexture2DArray() = 0;

    virtual QDemonRef<QDemonRenderTextureCube> createTextureCube() = 0;

    virtual QDemonRef<QDemonRenderImage2D> createImage2D(QDemonRef<QDemonRenderTexture2D> inTexture,
                                                         QDemonRenderImageAccessType::Enum inAccess) = 0;

    virtual QDemonRef<QDemonRenderRenderBuffer> createRenderBuffer(QDemonRenderRenderBufferFormats::Enum bufferFormat,
                                                                   quint32 width,
                                                                   quint32 height) = 0;
    virtual QDemonRef<QDemonRenderRenderBuffer> getRenderBuffer(const void *implementationHandle) = 0;
    // Create a new frame buffer and set the current render target to that frame buffer.
    virtual QDemonRef<QDemonRenderFrameBuffer> createFrameBuffer() = 0;
    virtual QDemonRef<QDemonRenderFrameBuffer> getFrameBuffer(const void *implementationHandle) = 0;

    virtual QDemonRef<QDemonRenderAttribLayout> createAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) = 0;

    virtual QDemonRef<QDemonRenderInputAssembler> createInputAssembler(QDemonRef<QDemonRenderAttribLayout> attribLayout,
                                                                       QDemonConstDataRef<QDemonRef<QDemonRenderVertexBuffer>> buffers,
                                                                       const QDemonRef<QDemonRenderIndexBuffer> indexBuffer,
                                                                       QDemonConstDataRef<quint32> strides,
                                                                       QDemonConstDataRef<quint32> offsets,
                                                                       QDemonRenderDrawMode::Enum primType = QDemonRenderDrawMode::Triangles,
                                                                       quint32 patchVertexCount = 1) = 0;
    virtual void setInputAssembler(QDemonRef<QDemonRenderInputAssembler> inputAssembler) = 0;

    virtual QDemonRenderVertFragCompilationResult compileSource(
            const char *shaderName,
            QDemonConstDataRef<qint8> vertShader,
            QDemonConstDataRef<qint8> fragShader,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType::Enum type = QDemonRenderShaderProgramBinaryType::Unknown,
            bool binaryProgram = false) = 0;

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

    virtual QDemonRenderVertFragCompilationResult compileBinary(
            const char *shaderName,
            QDemonRenderShaderProgramBinaryType::Enum type,
            QDemonDataRef<qint8> vertShader,
            QDemonDataRef<qint8> fragShader,
            QDemonDataRef<qint8> tessControlShaderSource = QDemonDataRef<qint8>(),
            QDemonDataRef<qint8> tessEvaluationShaderSource = QDemonDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>()) = 0;

    virtual QDemonRenderVertFragCompilationResult compileComputeSource(const char *shaderName,
                                                                       QDemonConstDataRef<qint8> computeShaderSource) = 0;

    virtual QDemonRef<QDemonRenderShaderProgram> getShaderProgram(const void *implementationHandle) = 0;

    virtual QDemonRef<QDemonRenderProgramPipeline> createProgramPipeline() = 0;

    virtual QDemonRef<QDemonRenderPathSpecification> createPathSpecification() = 0;
    virtual QDemonRef<QDemonRenderPathRender> createPathRender(size_t range = 1) = 0;
    virtual void setPathProjectionMatrix(const QMatrix4x4 inPathProjection) = 0;
    virtual void setPathModelViewMatrix(const QMatrix4x4 inPathModelview) = 0;
    virtual void setPathStencilDepthOffset(float inSlope, float inBias) = 0;
    virtual void setPathCoverDepthFunc(QDemonRenderBoolOp::Enum inFunc) = 0;

    virtual QDemonRef<QDemonRenderPathFontSpecification> createPathFontSpecification(const QString &fontName) = 0;
    virtual QDemonRef<QDemonRenderPathFontItem> createPathFontItem() = 0;

    // Specific setters for the guaranteed-to-exist context properties to set them as fast as
    // possible.
    // Note that this bypasses the property manage so push/pop will have no effect (unless you
    // set these already
    // once after a push using the property manager.
    virtual void setClearColor(QVector4D inClearColor) = 0;
    virtual QVector4D getClearColor() const = 0;

    virtual void setBlendFunction(QDemonRenderBlendFunctionArgument inFunctions) = 0;
    virtual QDemonRenderBlendFunctionArgument getBlendFunction() const = 0;

    virtual void setBlendEquation(QDemonRenderBlendEquationArgument inEquations) = 0;
    virtual QDemonRenderBlendEquationArgument getBlendEquation() const = 0;

    virtual void setCullingEnabled(bool inEnabled) = 0;
    virtual bool isCullingEnabled() const = 0;

    virtual void setDepthFunction(QDemonRenderBoolOp::Enum inFunction) = 0;
    virtual QDemonRenderBoolOp::Enum getDepthFunction() const = 0;

    virtual void setBlendingEnabled(bool inEnabled) = 0;
    virtual bool isBlendingEnabled() const = 0;

    virtual void setDepthWriteEnabled(bool inEnabled) = 0;
    virtual bool isDepthWriteEnabled() const = 0;

    virtual void setDepthTestEnabled(bool inEnabled) = 0;
    virtual bool isDepthTestEnabled() const = 0;

    virtual void setDepthStencilState(QDemonRef<QDemonRenderDepthStencilState> inDepthStencilState) = 0;
    virtual void setStencilTestEnabled(bool inEnabled) = 0;
    virtual bool isStencilTestEnabled() const = 0;

    virtual void setRasterizerState(QDemonRef<QDemonRenderRasterizerState> inRasterizerState) = 0;

    virtual void setScissorTestEnabled(bool inEnabled) = 0;
    virtual bool isScissorTestEnabled() const = 0;

    virtual void setScissorRect(QRect inRect) = 0;
    virtual QRect getScissorRect() const = 0;

    virtual void setViewport(QRect inViewport) = 0;
    virtual QRect getViewport() const = 0;

    virtual void setColorWritesEnabled(bool inEnabled) = 0;
    virtual bool isColorWritesEnabled() const = 0;

    virtual void setMultisampleEnabled(bool inEnabled) = 0;
    virtual bool isMultisampleEnabled() const = 0;

    // Used during layer rendering because we can't set the *actual* viewport to what it should
    // be due to hardware problems.
    // Set during begin render.
    static QMatrix4x4 applyVirtualViewportToProjectionMatrix(const QMatrix4x4 &inProjection,
                                                             const QRectF &inViewport,
                                                             const QRectF &inVirtualViewport);

    virtual void setRenderTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer) = 0;
    virtual void setReadTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer) = 0;
    virtual QDemonRef<QDemonRenderFrameBuffer> getRenderTarget() const = 0;

    virtual void setActiveShader(QDemonRef<QDemonRenderShaderProgram> inShader) = 0;
    virtual QDemonRef<QDemonRenderShaderProgram> getActiveShader() const = 0;

    virtual void setActiveProgramPipeline(QDemonRef<QDemonRenderProgramPipeline> inProgramPipeline) = 0;
    virtual QDemonRef<QDemonRenderProgramPipeline> getActiveProgramPipeline() const = 0;

    virtual void dispatchCompute(QDemonRef<QDemonRenderShaderProgram> inShader, quint32 numGroupsX, quint32 numGroupsY, quint32 numGroupsZ) = 0;

    // Push the entire set of properties.
    virtual void pushPropertySet() = 0;
    // Pop the entire set of properties, potentially forcing the values
    // to opengl.  Will set the hardware state to whatever at the time of push.
    virtual void popPropertySet(bool inForceSetProperties) = 0;

    // Ensure the hardware state matches the state we expect.
    virtual void resetBlendState() = 0;

    // Draw buffers are what the system will render to.. Applies to the current render context.
    //-1 means none, else the integer is assumed to be an index past the draw buffer index.
    // This applies only to the currently bound framebuffer.
    virtual void setDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet) = 0;
    virtual void setReadBuffer(QDemonReadFaces::Enum inReadFace) = 0;

    virtual void readPixels(QRect inRect, QDemonRenderReadPixelFormats::Enum inFormat, QDemonDataRef<quint8> inWriteBuffer) = 0;

    // Return the property manager for this render context
    // virtual NVRenderPropertyManager& GetPropertyManager() = 0;
    // Clear the current render target
    virtual void clear(QDemonRenderClearFlags flags) = 0;
    // Clear this framebuffer without changing the active frame buffer
    virtual void clear(QDemonRef<QDemonRenderFrameBuffer> framebuffer, QDemonRenderClearFlags flags) = 0;
    // copy framebuffer content between read target and render target
    virtual void blitFramebuffer(qint32 srcX0,
                                 qint32 srcY0,
                                 qint32 srcX1,
                                 qint32 srcY1,
                                 qint32 dstX0,
                                 qint32 dstY0,
                                 qint32 dstX1,
                                 qint32 dstY1,
                                 QDemonRenderClearFlags flags,
                                 QDemonRenderTextureMagnifyingOp::Enum filter) = 0;

    // Render, applying these immediate property values just before render.  The hardware
    // properties are tracked for push/pop
    // but the shader properties are just set on the active shader *after* the hardware
    // properties have been set.  The shader properties are not tracked for push/pop.
    // This call is meant to come directly before each draw call to setup the last bit of state
    // before the draw operation.  Note that there isn't another way to set the immedate
    // property values for a given shader because it isn't clear which shader is active
    // when the properties are getting set.
    void draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset) override = 0;

    /**
     * @brief Draw the current active vertex buffer using an indirect buffer
     *		  This means the setup of the draw call is stored in a buffer bound to
     *		  QDemonRenderBufferBindValues::Draw_Indirect
     *
     * @param[in] drawMode	Draw mode (Triangles, ....)
     * @param[in] indirect	Offset into a indirect drawing setup buffer
     *
     * @return no return.
     */
    virtual void drawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset) = 0;

    virtual QSurfaceFormat format() const = 0;

    virtual void resetStates() = 0;

    static QDemonRef<QDemonRenderContext> createGl(const QSurfaceFormat &format);

    static QDemonRef<QDemonRenderContext> createNull();
};

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

/**
 * A Render Context implementation class
 *
 */
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

// forward declarations

class QDemonRenderContextImpl : public QDemonRenderContext
{
    Q_DISABLE_COPY(QDemonRenderContextImpl)
public:
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

    void doSetDepthFunction(QDemonRenderBoolOp::Enum inFunction)
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
            m_backend->setRenderTarget(inBuffer->getFrameBuffertHandle());
        else
            m_backend->setRenderTarget(m_defaultOffscreenRenderTarget);

        m_hardwarePropertyContext.m_frameBuffer = inBuffer;
    }

    void doSetReadTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer)
    {
        if (inBuffer)
            m_backend->setReadTarget(inBuffer->getFrameBuffertHandle());
        else
            m_backend->setReadTarget(QDemonRenderBackend::QDemonRenderBackendRenderTargetObject(nullptr));
    }

    bool bindShaderToInputAssembler(const QDemonRef<QDemonRenderInputAssembler> &inputAssembler,
                                    const QDemonRef<QDemonRenderShaderProgram> &shader);
    bool applyPreDrawProperties();
    void onPostDraw();

public:
    QDemonRenderContextImpl(const QDemonRef<QDemonRenderBackend> &inBackend);
    virtual ~QDemonRenderContextImpl();

    QDemonRef<QDemonRenderBackend> getBackend() override { return m_backend; }

    void getMaxTextureSize(qint32 &oWidth, qint32 &oHeight) override;

    const char *getShadingLanguageVersion() override { return m_backend->getShadingLanguageVersion(); }

    QDemonRenderContextType getRenderContextType() const override { return m_backend->getRenderContextType(); }

    qint32 getDepthBits() const override
    {
        // only query this if a framebuffer is bound
        if (m_hardwarePropertyContext.m_frameBuffer)
            return m_backend->getDepthBits();
        else
            return m_dephBits;
    }

    qint32 getStencilBits() const override
    {
        // only query this if a framebuffer is bound
        if (m_hardwarePropertyContext.m_frameBuffer)
            return m_backend->getStencilBits();
        else
            return m_stencilBits;
    }

    bool getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Enum inCap) const override
    {
        return m_backend->getRenderBackendCap(inCap);
    }

    bool areMultisampleTexturesSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::MsTexture);
    }

    bool getConstantBufferSupport() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ConstantBuffer);
    }

    bool areDXTImagesSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::DxtImages);
    }

    bool isDepthStencilSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::DepthStencilTexture);
    }

    bool isFpRenderTargetSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::FpRenderTarget);
    }

    bool isTessellationSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Tessellation);
    }

    bool isGeometryStageSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Geometry);
    }

    bool isComputeSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Compute);
    }

    bool isSampleQuerySupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::SampleQuery);
    }

    bool isTimerQuerySupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TimerQuery);
    }

    bool isCommandSyncSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::CommandSync);
    }
    bool isTextureArraySupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TextureArray);
    }
    bool isStorageBufferSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::StorageBuffer);
    }
    bool isAtomicCounterBufferSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AtomicCounterBuffer);
    }
    bool isShaderImageLoadStoreSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ShaderImageLoadStore);
    }
    bool isProgramPipelineSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ProgramPipeline);
    }
    bool isPathRenderingSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::PathRendering);
    }
    // Are blend modes really supported in HW?
    bool isAdvancedBlendHwSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AdvancedBlend);
    }
    bool isAdvancedBlendHwSupportedKHR() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AdvancedBlendKHR);
    }
    bool isBlendCoherencySupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::BlendCoherency);
    }
    bool isStandardDerivativesSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::StandardDerivatives);
    }
    bool isTextureLodSupported() const override
    {
        return getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TextureLod);
    }

    void setDefaultRenderTarget(quint64 targetID) override
    {
        m_defaultOffscreenRenderTarget = reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendRenderTargetObject>(targetID);
    }

    void setDefaultDepthBufferBitCount(qint32 depthBits) override { m_dephBits = depthBits; }

    virtual QDemonRef<QDemonRenderDepthStencilState> createDepthStencilState(bool enableDepth,
                                                                             bool depthMask,
                                                                             QDemonRenderBoolOp::Enum depthFunc,
                                                                             bool enableStencil,
                                                                             QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                                                             QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                                                             QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                                                             QDemonRenderStencilOperationArgument &depthStencilOpBack) override;
    void setDepthStencilState(QDemonRef<QDemonRenderDepthStencilState> inDepthStencilState) override;
    virtual void stateDestroyed(QDemonRenderDepthStencilState *state);

    QDemonRef<QDemonRenderRasterizerState> createRasterizerState(float depthBias, float depthScale, QDemonRenderFaces::Enum cullFace) override;
    void setRasterizerState(QDemonRef<QDemonRenderRasterizerState> inRasterizerState) override;
    virtual void stateDestroyed(QDemonRenderRasterizerState *state);

    QDemonRef<QDemonRenderVertexBuffer> createVertexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                           size_t size,
                                                           quint32 stride,
                                                           QDemonConstDataRef<quint8> bufferData) override;
    QDemonRef<QDemonRenderVertexBuffer> getVertexBuffer(const void *implementationHandle) override;
    virtual void bufferDestroyed(QDemonRenderVertexBuffer *buffer);

    virtual QDemonRef<QDemonRenderIndexBuffer> createIndexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                                 QDemonRenderComponentTypes::Enum componentType,
                                                                 size_t size,
                                                                 QDemonConstDataRef<quint8> bufferData) override;
    QDemonRef<QDemonRenderIndexBuffer> getIndexBuffer(const void *implementationHandle) override;
    virtual void bufferDestroyed(QDemonRenderIndexBuffer *buffer);

    virtual QDemonRef<QDemonRenderConstantBuffer> createConstantBuffer(const char *bufferName,
                                                                       QDemonRenderBufferUsageType::Enum usageType,
                                                                       size_t size,
                                                                       QDemonConstDataRef<quint8> bufferData) override;
    QDemonRef<QDemonRenderConstantBuffer> getConstantBuffer(const QByteArray &bufferName) override;
    virtual void bufferDestroyed(QDemonRenderConstantBuffer *buffer);

    virtual qint32 getNextConstantBufferUnit();

    virtual QDemonRef<QDemonRenderStorageBuffer> createStorageBuffer(const char *bufferName,
                                                                     QDemonRenderBufferUsageType::Enum usageType,
                                                                     size_t size,
                                                                     QDemonConstDataRef<quint8> bufferData,
                                                                     QDemonRenderDataBuffer *pBuffer) override;
    QDemonRef<QDemonRenderStorageBuffer> getStorageBuffer(const QByteArray &bufferName) override;
    virtual void bufferDestroyed(QDemonRenderStorageBuffer *buffer);

    virtual QDemonRef<QDemonRenderAtomicCounterBuffer> createAtomicCounterBuffer(const char *bufferName,
                                                                                 QDemonRenderBufferUsageType::Enum usageType,
                                                                                 size_t size,
                                                                                 QDemonConstDataRef<quint8> bufferData) override;
    QDemonRef<QDemonRenderAtomicCounterBuffer> getAtomicCounterBuffer(const QByteArray &bufferName) override;
    virtual QDemonRef<QDemonRenderAtomicCounterBuffer> getAtomicCounterBufferByParam(const QByteArray &paramName) override;
    virtual void bufferDestroyed(QDemonRenderAtomicCounterBuffer *buffer);

    virtual QDemonRef<QDemonRenderDrawIndirectBuffer> createDrawIndirectBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                                               size_t size,
                                                                               QDemonConstDataRef<quint8> bufferData) override;
    virtual QDemonRef<QDemonRenderDrawIndirectBuffer> getDrawIndirectBuffer(QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle) override;
    virtual void bufferDestroyed(QDemonRenderDrawIndirectBuffer *buffer);

    void setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) override;

    QDemonRef<QDemonRenderOcclusionQuery> createOcclusionQuery() override;
    QDemonRef<QDemonRenderTimerQuery> createTimerQuery() override;
    QDemonRef<QDemonRenderSync> createSync() override;

    QDemonRef<QDemonRenderTexture2D> createTexture2D() override;
    QDemonRef<QDemonRenderTexture2D> getTexture2D(const void *implementationHandle) override;
    virtual void textureDestroyed(QDemonRenderTexture2D *buffer);

    QDemonRef<QDemonRenderTexture2DArray> createTexture2DArray() override;
    virtual void textureDestroyed(QDemonRenderTexture2DArray *buffer);

    QDemonRef<QDemonRenderTextureCube> createTextureCube() override;
    virtual void textureDestroyed(QDemonRenderTextureCube *buffer);

    virtual qint32 getNextTextureUnit();

    QDemonRef<QDemonRenderImage2D> createImage2D(QDemonRef<QDemonRenderTexture2D> inTexture,
                                                 QDemonRenderImageAccessType::Enum inAccess) override;
    virtual void imageDestroyed(QDemonRenderImage2D *buffer);

    virtual QDemonRef<QDemonRenderRenderBuffer> createRenderBuffer(QDemonRenderRenderBufferFormats::Enum bufferFormat,
                                                                   quint32 width,
                                                                   quint32 height) override;
    QDemonRef<QDemonRenderRenderBuffer> getRenderBuffer(const void *implementationHandle) override;
    virtual void renderBufferDestroyed(QDemonRenderRenderBuffer *buffer);

    QDemonRef<QDemonRenderFrameBuffer> createFrameBuffer() override;
    QDemonRef<QDemonRenderFrameBuffer> getFrameBuffer(const void *implementationHandle) override;
    virtual void frameBufferDestroyed(QDemonRenderFrameBuffer *fb);

    virtual QDemonRef<QDemonRenderAttribLayout> createAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) override;
    QDemonRef<QDemonRenderInputAssembler> createInputAssembler(QDemonRef<QDemonRenderAttribLayout> attribLayout,
                                                               QDemonConstDataRef<QDemonRef<QDemonRenderVertexBuffer>> buffers,
                                                               const QDemonRef<QDemonRenderIndexBuffer> indexBuffer,
                                                               QDemonConstDataRef<quint32> strides,
                                                               QDemonConstDataRef<quint32> offsets,
                                                               QDemonRenderDrawMode::Enum primType,
                                                               quint32 patchVertexCount) override;
    void setInputAssembler(QDemonRef<QDemonRenderInputAssembler> inputAssembler) override;

    QDemonRenderVertFragCompilationResult compileSource(
            const char *shaderName,
            QDemonConstDataRef<qint8> vertShader,
            QDemonConstDataRef<qint8> fragShader,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType::Enum type = QDemonRenderShaderProgramBinaryType::Unknown,
            bool binaryProgram = false) override;

    virtual QDemonRenderVertFragCompilationResult compileBinary(
            const char *shaderName,
            QDemonRenderShaderProgramBinaryType::Enum type,
            QDemonDataRef<qint8> vertShader,
            QDemonDataRef<qint8> fragShader,
            QDemonDataRef<qint8> tessControlShaderSource = QDemonDataRef<qint8>(),
            QDemonDataRef<qint8> tessEvaluationShaderSource = QDemonDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>()) override;

    virtual QDemonRenderVertFragCompilationResult compileComputeSource(const char *shaderName,
                                                                       QDemonConstDataRef<qint8> computeShaderSource) override;

    QDemonRef<QDemonRenderShaderProgram> getShaderProgram(const void *implementationHandle) override;
    virtual void shaderDestroyed(QDemonRenderShaderProgram *shader);

    QDemonRef<QDemonRenderProgramPipeline> createProgramPipeline() override;
    QDemonRef<QDemonRenderPathSpecification> createPathSpecification() override;
    QDemonRef<QDemonRenderPathRender> createPathRender(size_t range = 1) override;
    void setPathProjectionMatrix(const QMatrix4x4 inPathProjection) override;
    void setPathModelViewMatrix(const QMatrix4x4 inPathModelview) override;
    void setPathStencilDepthOffset(float inSlope, float inBias) override;
    void setPathCoverDepthFunc(QDemonRenderBoolOp::Enum inFunc) override;

    virtual QDemonRef<QDemonRenderPathFontSpecification> createPathFontSpecification(const QString &fontName) override;
    virtual void releasePathFontSpecification(QDemonRenderPathFontSpecification *inPathSpec);
    QDemonRef<QDemonRenderPathFontItem> createPathFontItem() override;

    void setClearColor(QVector4D inClearColor) override;
    QVector4D getClearColor() const override { return m_hardwarePropertyContext.m_clearColor; }

    void setBlendFunction(QDemonRenderBlendFunctionArgument inFunctions) override;
    QDemonRenderBlendFunctionArgument getBlendFunction() const override
    {
        return m_hardwarePropertyContext.m_blendFunction;
    }

    void setBlendEquation(QDemonRenderBlendEquationArgument inEquations) override;
    QDemonRenderBlendEquationArgument getBlendEquation() const override
    {
        return m_hardwarePropertyContext.m_blendEquation;
    }

    void setCullingEnabled(bool inEnabled) override;
    bool isCullingEnabled() const override { return m_hardwarePropertyContext.m_cullingEnabled; }

    void setDepthFunction(QDemonRenderBoolOp::Enum inFunction) override;
    QDemonRenderBoolOp::Enum getDepthFunction() const override { return m_hardwarePropertyContext.m_depthFunction; }

    void setBlendingEnabled(bool inEnabled) override;
    bool isBlendingEnabled() const override { return m_hardwarePropertyContext.m_blendingEnabled; }

    void setDepthWriteEnabled(bool inEnabled) override;
    bool isDepthWriteEnabled() const override { return m_hardwarePropertyContext.m_depthWriteEnabled; }
    void setDepthTestEnabled(bool inEnabled) override;
    bool isDepthTestEnabled() const override { return m_hardwarePropertyContext.m_depthTestEnabled; }

    void setStencilTestEnabled(bool inEnabled) override;
    bool isStencilTestEnabled() const override { return m_hardwarePropertyContext.m_stencilTestEnabled; }

    void setScissorTestEnabled(bool inEnabled) override;
    bool isScissorTestEnabled() const override { return m_hardwarePropertyContext.m_scissorTestEnabled; }
    void setScissorRect(QRect inRect) override;
    QRect getScissorRect() const override { return m_hardwarePropertyContext.m_scissorRect; }

    void setViewport(QRect inViewport) override;
    QRect getViewport() const override { return m_hardwarePropertyContext.m_viewport; }

    void setColorWritesEnabled(bool inEnabled) override;
    bool isColorWritesEnabled() const override { return m_hardwarePropertyContext.m_colorWritesEnabled; }

    void setMultisampleEnabled(bool inEnabled) override;
    bool isMultisampleEnabled() const override { return m_hardwarePropertyContext.m_multisampleEnabled; }

    void setActiveShader(QDemonRef<QDemonRenderShaderProgram> inShader) override;
    QDemonRef<QDemonRenderShaderProgram> getActiveShader() const override;

    void setActiveProgramPipeline(QDemonRef<QDemonRenderProgramPipeline> inProgramPipeline) override;
    QDemonRef<QDemonRenderProgramPipeline> getActiveProgramPipeline() const override;

    void dispatchCompute(QDemonRef<QDemonRenderShaderProgram> inShader, quint32 numGroupsX, quint32 numGroupsY, quint32 numGroupsZ) override;

    void setDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet) override;
    void setReadBuffer(QDemonReadFaces::Enum inReadFace) override;

    void readPixels(QRect inRect, QDemonRenderReadPixelFormats::Enum inFormat, QDemonDataRef<quint8> inWriteBuffer) override;

    void setRenderTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer) override;
    void setReadTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer) override;
    QDemonRef<QDemonRenderFrameBuffer> getRenderTarget() const override
    {
        return m_hardwarePropertyContext.m_frameBuffer;
    }

    void resetBlendState() override;

    // Push the entire set of properties.
    void pushPropertySet() override;

    // Pop the entire set of properties, potentially forcing the values
    // to opengl.
    void popPropertySet(bool inForceSetProperties) override;

    // clear current bound render target
    void clear(QDemonRenderClearFlags flags) override;
    // clear passed in rendertarget
    void clear(QDemonRef<QDemonRenderFrameBuffer> fb, QDemonRenderClearFlags flags) override;

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
                         QDemonRenderTextureMagnifyingOp::Enum filter) override;

    void draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset) override;
    void drawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset) override;

    QSurfaceFormat format() const override { return m_backend->format(); }
    virtual void resetStates()
    {
        pushPropertySet();
        popPropertySet(true);
    }
};

QT_END_NAMESPACE

#endif
