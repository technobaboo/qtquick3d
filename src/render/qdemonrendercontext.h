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
#pragma once
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
#include <QtDemonRender/qdemonrenderprogrampipeline.h>
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
typedef QHash<QString, QDemonRenderConstantBuffer *> TContextConstantBufferMap;
typedef QHash<QString, QDemonRenderStorageBuffer *> TContextStorageBufferMap;
typedef QHash<QString, QDemonRenderAtomicCounterBuffer *> TContextAtomicCounterBufferMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendBufferObject, QDemonRenderDrawIndirectBuffer *> TContextDrawIndirectBufferMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject, QDemonRenderDepthStencilState *> TContextDepthStencilStateMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject, QDemonRenderRasterizerState *> TContextRasterizerStateMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendTextureObject, QDemonRenderTexture2DArray *> TContextTex2DArrayToImpMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendTextureObject, QDemonRenderTextureCube *> TContextTexCubeToImpMap;
typedef QHash<QDemonRenderBackend::QDemonRenderBackendTextureObject, QDemonRenderImage2D *> TContextImage2DToImpMap;
typedef QHash<QString, QDemonRenderPathFontSpecification *> TContextPathFontSpecificationMap;

class Q_DEMONRENDER_EXPORT QDemonRenderContext : public QDemonRenderDrawable
{
public:
    virtual QDemonRenderContextType GetRenderContextType() const = 0;
    virtual bool AreMultisampleTexturesSupported() const = 0;
    virtual bool GetConstantBufferSupport() const = 0;
    virtual void getMaxTextureSize(quint32 &oWidth, quint32 &oHeight) = 0;
    virtual const char *GetShadingLanguageVersion() = 0;
    // Get the bit depth of the currently bound depth buffer.
    virtual quint32 GetDepthBits() const = 0;
    virtual quint32 GetStencilBits() const = 0;
    virtual bool
    GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Enum inCap) const = 0;
    virtual bool AreDXTImagesSupported() const = 0;
    virtual bool IsDepthStencilSupported() const = 0;
    virtual bool IsFpRenderTargetSupported() const = 0;
    virtual bool IsTessellationSupported() const = 0;
    virtual bool IsGeometryStageSupported() const = 0;
    virtual bool IsComputeSupported() const = 0;
    virtual bool IsSampleQuerySupported() const = 0;
    virtual bool IsTimerQuerySupported() const = 0;
    virtual bool IsCommandSyncSupported() const = 0;
    virtual bool IsTextureArraySupported() const = 0;
    virtual bool IsStorageBufferSupported() const = 0;
    virtual bool IsAtomicCounterBufferSupported() const = 0;
    virtual bool IsShaderImageLoadStoreSupported() const = 0;
    virtual bool IsProgramPipelineSupported() const = 0;
    virtual bool IsPathRenderingSupported() const = 0;
    virtual bool IsBlendCoherencySupported() const = 0;
    virtual bool IsAdvancedBlendHwSupported() const = 0;
    virtual bool IsAdvancedBlendHwSupportedKHR() const = 0;
    virtual bool IsStandardDerivativesSupported() const = 0;
    virtual bool IsTextureLodSupported() const = 0;

    virtual void SetDefaultRenderTarget(quint64 targetID) = 0;
    virtual void SetDefaultDepthBufferBitCount(qint32 depthBits) = 0;

    virtual QDemonRenderDepthStencilState *
    CreateDepthStencilState(bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc,
                            bool enableStencil,
                            QDemonRenderStencilFunctionArgument &stencilFuncFront,
                            QDemonRenderStencilFunctionArgument &stencilFuncBack,
                            QDemonRenderStencilOperationArgument &depthStencilOpFront,
                            QDemonRenderStencilOperationArgument &depthStencilOpBack) = 0;

    virtual QDemonRenderRasterizerState *CreateRasterizerState(float depthBias, float depthScale,
                                                           QDemonRenderFaces::Enum cullFace) = 0;

    virtual QDemonRenderVertexBuffer *
    CreateVertexBuffer(QDemonRenderBufferUsageType::Enum usageType, size_t size, quint32 stride = 0,
                       QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) = 0;
    virtual QDemonRenderVertexBuffer *GetVertexBuffer(const void *implementationHandle) = 0;

    virtual QDemonRenderIndexBuffer *
    CreateIndexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                      QDemonRenderComponentTypes::Enum componentType, size_t size,
                      QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) = 0;
    virtual QDemonRenderIndexBuffer *GetIndexBuffer(const void *implementationHandle) = 0;

    virtual QDemonRenderConstantBuffer *
    CreateConstantBuffer(const char *bufferName,
                         QDemonRenderBufferUsageType::Enum usageType, size_t size,
                         QDemonConstDataRef<quint8> bufferData) = 0;
    virtual QDemonRenderConstantBuffer *GetConstantBuffer(const QString &bufferName) = 0;

    virtual QDemonRenderStorageBuffer *
    CreateStorageBuffer(const char *bufferName,
                        QDemonRenderBufferUsageType::Enum usageType, size_t size,
                        QDemonConstDataRef<quint8> bufferData, QDemonRenderDataBuffer *pBuffer) = 0;
    virtual QDemonRenderStorageBuffer *GetStorageBuffer(const QString &bufferName) = 0;

    virtual QDemonRenderAtomicCounterBuffer *
    CreateAtomicCounterBuffer(const char *bufferName,
                              QDemonRenderBufferUsageType::Enum usageType, size_t size,
                              QDemonConstDataRef<quint8> bufferData) = 0;
    virtual QDemonRenderAtomicCounterBuffer *
    GetAtomicCounterBuffer(const QString &bufferName) = 0;
    virtual QDemonRenderAtomicCounterBuffer *
    GetAtomicCounterBufferByParam(const QString &paramName) = 0;

    virtual QDemonRenderDrawIndirectBuffer *
    CreateDrawIndirectBuffer(QDemonRenderBufferUsageType::Enum usageType, size_t size,
                             QDemonConstDataRef<quint8> bufferData) = 0;
    virtual QDemonRenderDrawIndirectBuffer *GetDrawIndirectBuffer(
            QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle) = 0;

    virtual void SetMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) = 0;

    virtual QDemonRenderOcclusionQuery *CreateOcclusionQuery() = 0;
    virtual QDemonRenderTimerQuery *CreateTimerQuery() = 0;
    virtual QSharedPointer<QDemonRenderSync> CreateSync() = 0;

    virtual QSharedPointer<QDemonRenderTexture2D> CreateTexture2D() = 0;
    virtual QSharedPointer<QDemonRenderTexture2D> GetTexture2D(const void *implementationHandle) = 0;
    virtual QSharedPointer<QDemonRenderBackend> GetBackend() = 0;

    virtual QSharedPointer<QDemonRenderTexture2DArray> CreateTexture2DArray() = 0;

    virtual QSharedPointer<QDemonRenderTextureCube> CreateTextureCube() = 0;

    virtual QDemonRenderImage2D *CreateImage2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                                               QDemonRenderImageAccessType::Enum inAccess) = 0;

    virtual QSharedPointer<QDemonRenderRenderBuffer>
    CreateRenderBuffer(QDemonRenderRenderBufferFormats::Enum bufferFormat, quint32 width,
                       quint32 height) = 0;
    virtual QSharedPointer<QDemonRenderRenderBuffer> GetRenderBuffer(const void *implementationHandle) = 0;
    // Create a new frame buffer and set the current render target to that frame buffer.
    virtual QSharedPointer<QDemonRenderFrameBuffer> CreateFrameBuffer() = 0;
    virtual QSharedPointer<QDemonRenderFrameBuffer> GetFrameBuffer(const void *implementationHandle) = 0;

    virtual QSharedPointer<QDemonRenderAttribLayout> CreateAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) = 0;

    virtual QSharedPointer<QDemonRenderInputAssembler>
    CreateInputAssembler(QSharedPointer<QDemonRenderAttribLayout> attribLayout,
                         QDemonConstDataRef<QDemonRenderVertexBuffer *> buffers,
                         const QDemonRenderIndexBuffer *indexBuffer, QDemonConstDataRef<quint32> strides,
                         QDemonConstDataRef<quint32> offsets,
                         QDemonRenderDrawMode::Enum primType = QDemonRenderDrawMode::Triangles,
                         quint32 patchVertexCount = 1) = 0;
    virtual void SetInputAssembler(QSharedPointer<QDemonRenderInputAssembler> inputAssembler) = 0;

    virtual QDemonRenderVertFragCompilationResult CompileSource(
            const char *shaderName, QDemonConstDataRef<qint8> vertShader,
            QDemonConstDataRef<qint8> fragShader,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType::Enum type = QDemonRenderShaderProgramBinaryType::Unknown,
            bool binaryProgram = false) = 0;

    // You must figure out inVertLen and inFragLen yourself, this object doesn't do that.
    QDemonRenderVertFragCompilationResult
    CompileSource(const char *shaderName, const char *vertShader, quint32 inVertLen,
                  const char *fragShader, quint32 inFragLen, const char * = nullptr,
                  quint32 inTCLen = 0, const char *tessEvaluationShaderSource = nullptr,
                  quint32 inTELen = 0, const char *geometryShaderSource = nullptr, quint32 inGSLen = 0,
                  bool separableProgram = false);

    virtual QDemonRenderVertFragCompilationResult
    CompileBinary(const char *shaderName, QDemonRenderShaderProgramBinaryType::Enum type,
                  QDemonDataRef<qint8> vertShader, QDemonDataRef<qint8> fragShader,
                  QDemonDataRef<qint8> tessControlShaderSource = QDemonDataRef<qint8>(),
                  QDemonDataRef<qint8> tessEvaluationShaderSource = QDemonDataRef<qint8>(),
                  QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>()) = 0;

    virtual QDemonRenderVertFragCompilationResult
    CompileComputeSource(const char *shaderName, QDemonConstDataRef<qint8> computeShaderSource) = 0;

    virtual QSharedPointer<QDemonRenderShaderProgram> GetShaderProgram(const void *implementationHandle) = 0;

    virtual QDemonRenderProgramPipeline *CreateProgramPipeline() = 0;

    virtual QDemonRenderPathSpecification *CreatePathSpecification() = 0;
    virtual QDemonRenderPathRender *CreatePathRender(size_t range = 1) = 0;
    virtual void SetPathProjectionMatrix(const QMatrix4x4 inPathProjection) = 0;
    virtual void SetPathModelViewMatrix(const QMatrix4x4 inPathModelview) = 0;
    virtual void SetPathStencilDepthOffset(float inSlope, float inBias) = 0;
    virtual void SetPathCoverDepthFunc(QDemonRenderBoolOp::Enum inFunc) = 0;

    virtual QDemonRenderPathFontSpecification *
    CreatePathFontSpecification(const QString &fontName) = 0;
    virtual QDemonRenderPathFontItem *CreatePathFontItem() = 0;

    // Specific setters for the guaranteed-to-exist context properties to set them as fast as
    // possible.
    // Note that this bypasses the property manage so push/pop will have no effect (unless you
    // set these already
    // once after a push using the property manager.
    virtual void SetClearColor(QVector4D inClearColor) = 0;
    virtual QVector4D GetClearColor() const = 0;

    virtual void SetBlendFunction(QDemonRenderBlendFunctionArgument inFunctions) = 0;
    virtual QDemonRenderBlendFunctionArgument GetBlendFunction() const = 0;

    virtual void SetBlendEquation(QDemonRenderBlendEquationArgument inEquations) = 0;
    virtual QDemonRenderBlendEquationArgument GetBlendEquation() const = 0;

    virtual void SetCullingEnabled(bool inEnabled) = 0;
    virtual bool IsCullingEnabled() const = 0;

    virtual void SetDepthFunction(QDemonRenderBoolOp::Enum inFunction) = 0;
    virtual QDemonRenderBoolOp::Enum GetDepthFunction() const = 0;

    virtual void SetBlendingEnabled(bool inEnabled) = 0;
    virtual bool IsBlendingEnabled() const = 0;

    virtual void SetDepthWriteEnabled(bool inEnabled) = 0;
    virtual bool IsDepthWriteEnabled() const = 0;

    virtual void SetDepthTestEnabled(bool inEnabled) = 0;
    virtual bool IsDepthTestEnabled() const = 0;

    virtual void SetDepthStencilState(QDemonRenderDepthStencilState *inDepthStencilState) = 0;
    virtual void SetStencilTestEnabled(bool inEnabled) = 0;
    virtual bool IsStencilTestEnabled() const = 0;

    virtual void SetRasterizerState(QDemonRenderRasterizerState *inRasterizerState) = 0;

    virtual void SetScissorTestEnabled(bool inEnabled) = 0;
    virtual bool IsScissorTestEnabled() const = 0;

    virtual void SetScissorRect(QDemonRenderRect inRect) = 0;
    virtual QDemonRenderRect GetScissorRect() const = 0;

    virtual void SetViewport(QDemonRenderRect inViewport) = 0;
    virtual QDemonRenderRect GetViewport() const = 0;

    virtual void SetColorWritesEnabled(bool inEnabled) = 0;
    virtual bool IsColorWritesEnabled() const = 0;

    virtual void SetMultisampleEnabled(bool inEnabled) = 0;
    virtual bool IsMultisampleEnabled() const = 0;

    // Used during layer rendering because we can't set the *actual* viewport to what it should
    // be due to hardware problems.
    // Set during begin render.
    static QMatrix4x4
    ApplyVirtualViewportToProjectionMatrix(const QMatrix4x4 &inProjection,
                                           const QDemonRenderRectF &inViewport,
                                           const QDemonRenderRectF &inVirtualViewport);

    virtual void SetRenderTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer) = 0;
    virtual void SetReadTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer) = 0;
    virtual QSharedPointer<QDemonRenderFrameBuffer> GetRenderTarget() const = 0;

    virtual void SetActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader) = 0;
    virtual QSharedPointer<QDemonRenderShaderProgram> GetActiveShader() const = 0;

    virtual void SetActiveProgramPipeline(QDemonRenderProgramPipeline *inProgramPipeline) = 0;
    virtual QDemonRenderProgramPipeline *GetActiveProgramPipeline() const = 0;

    virtual void DispatchCompute(QSharedPointer<QDemonRenderShaderProgram> inShader, quint32 numGroupsX,
                                 quint32 numGroupsY, quint32 numGroupsZ) = 0;

    // Push the entire set of properties.
    virtual void PushPropertySet() = 0;
    // Pop the entire set of properties, potentially forcing the values
    // to opengl.  Will set the hardware state to whatever at the time of push.
    virtual void PopPropertySet(bool inForceSetProperties) = 0;

    // Ensure the hardware state matches the state we expect.
    virtual void ResetBlendState() = 0;

    // Draw buffers are what the system will render to.. Applies to the current render context.
    //-1 means none, else the integer is assumed to be an index past the draw buffer index.
    // This applies only to the currently bound framebuffer.
    virtual void SetDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet) = 0;
    virtual void SetReadBuffer(QDemonReadFaces::Enum inReadFace) = 0;

    virtual void ReadPixels(QDemonRenderRect inRect, QDemonRenderReadPixelFormats::Enum inFormat,
                            QDemonDataRef<quint8> inWriteBuffer) = 0;

    // Return the property manager for this render context
    // virtual NVRenderPropertyManager& GetPropertyManager() = 0;
    // Clear the current render target
    virtual void Clear(QDemonRenderClearFlags flags) = 0;
    // Clear this framebuffer without changing the active frame buffer
    virtual void Clear(QSharedPointer<QDemonRenderFrameBuffer> framebuffer, QDemonRenderClearFlags flags) = 0;
    // copy framebuffer content between read target and render target
    virtual void BlitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1, qint32 srcY1,
                                 qint32 dstX0, qint32 dstY0, qint32 dstX1, qint32 dstY1,
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
    void Draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset) override = 0;

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
    virtual void DrawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset) = 0;

    virtual QSurfaceFormat format() const = 0;

    virtual void resetStates() = 0;

    static QSharedPointer<QDemonRenderContext> CreateGL(const QSurfaceFormat &format);

    static QSharedPointer<QDemonRenderContext> CreateNULL();
};

// Now for scoped property access.
template <typename TDataType>
struct QDemonRenderContextScopedProperty
        : public QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>
{
    typedef typename QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>::TGetter TGetter;
    typedef typename QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>::TSetter TSetter;
    QDemonRenderContextScopedProperty(QDemonRenderContext &ctx, TGetter getter, TSetter setter)
        : QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>(ctx, getter, setter)
    {
    }
    QDemonRenderContextScopedProperty(QDemonRenderContext &ctx, TGetter getter, TSetter setter,
                                      const TDataType &inNewValue)
        : QDemonRenderGenericScopedProperty<QDemonRenderContext, TDataType>(ctx, getter, setter,
                                                                            inNewValue)
    {
    }
};

/**
 * A Render Context implementation class
 *
 */

#define ITERATE_HARDWARE_CONTEXT_PROPERTIES                                                        \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(RenderTarget, FrameBuffer)                                    \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ActiveShader, ActiveShader)                                   \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ActiveProgramPipeline, ActiveProgramPipeline)                 \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(InputAssembler, InputAssembler)                               \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(BlendFunction, BlendFunction)                                 \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(CullingEnabled, CullingEnabled)                               \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(DepthFunction, DepthFunction)                                 \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(BlendingEnabled, BlendingEnabled)                             \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(DepthWriteEnabled, DepthWriteEnabled)                         \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(DepthTestEnabled, DepthTestEnabled)                           \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(StencilTestEnabled, StencilTestEnabled)                       \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ScissorTestEnabled, ScissorTestEnabled)                       \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ScissorRect, ScissorRect)                                     \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(Viewport, Viewport)                                           \
    HANDLE_CONTEXT_HARDWARE_PROPERTY(ClearColor, ClearColor)

// forward declarations

class QDemonRenderContextImpl : public QDemonRenderContext, public QDemonNoCopy
{
public:
    // these variables represent the current hardware state of the render context.
    QDemonGLHardPropertyContext m_HardwarePropertyContext;

private:
    QSharedPointer<QDemonRenderBackend> m_backend; ///< pointer to our render backend
    QDemonRenderContextDirtyFlags m_DirtyFlags; ///< context dirty flags

    QDemonRenderBackend::QDemonRenderBackendRenderTargetObject
    m_DefaultOffscreenRenderTarget; ///< this is a special target set from outside if we
    ///never render to a window directly (GL only)
    qint32 m_DephBits; ///< this is the depth bits count of the default window render target
    qint32 m_StencilBits; ///< this is the stencil bits count of the default window render target

protected:
    QHash<const void *, QDemonRenderVertexBuffer *> m_VertToImpMap;
    QHash<const void *, QDemonRenderIndexBuffer *> m_IndexToImpMap;
    TContextConstantBufferMap m_ConstantToImpMap;
    TContextStorageBufferMap m_StorageToImpMap;
    TContextAtomicCounterBufferMap m_AtomicCounterToImpMap;
    TContextDrawIndirectBufferMap m_DrawIndirectToImpMap;
    TContextDepthStencilStateMap m_DepthStencilStateToImpMap;
    TContextRasterizerStateMap m_RasterizerStateToImpMap;
    TContextPathFontSpecificationMap m_PathFontSpecToImpMap;

    QHash<const void *, QDemonRenderTexture2D *> m_Tex2DToImpMap;
    TContextTex2DArrayToImpMap m_Tex2DArrayToImpMap;
    TContextTexCubeToImpMap m_TexCubeToImpMap;
    TContextImage2DToImpMap m_Image2DtoImpMap;
    QHash<const void *, QDemonRenderShaderProgram *> m_ShaderToImpMap;
    QHash<const void *, QDemonRenderRenderBuffer *> m_RenderBufferToImpMap;
    QHash<const void *, QDemonRenderFrameBuffer *> m_FrameBufferToImpMap;
    quint32 m_MaxTextureUnits;
    quint32 m_NextTextureUnit;
    quint32 m_MaxConstantBufferUnits;
    quint32 m_NextConstantBufferUnit;

    QVector<QDemonGLHardPropertyContext> m_PropertyStack;

    void DoSetClearColor(QVector4D inClearColor)
    {
        m_HardwarePropertyContext.m_ClearColor = inClearColor;
        m_backend->SetClearColor(&inClearColor);
    }

    void DoSetBlendFunction(QDemonRenderBlendFunctionArgument inFunctions)
    {
        qint32_4 values;
        m_HardwarePropertyContext.m_BlendFunction = inFunctions;

        m_backend->SetBlendFunc(inFunctions);
    }

    void DoSetBlendEquation(QDemonRenderBlendEquationArgument inEquations)
    {
        qint32_4 values;
        m_HardwarePropertyContext.m_BlendEquation = inEquations;

        m_backend->SetBlendEquation(inEquations);
    }

    void DoSetCullingEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_CullingEnabled = inEnabled;
        m_backend->SetRenderState(inEnabled, QDemonRenderState::CullFace);
    }

    void DoSetDepthFunction(QDemonRenderBoolOp::Enum inFunction)
    {
        m_HardwarePropertyContext.m_DepthFunction = inFunction;
        m_backend->SetDepthFunc(inFunction);
    }

    void DoSetBlendingEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_BlendingEnabled = inEnabled;
        m_backend->SetRenderState(inEnabled, QDemonRenderState::Blend);
    }

    void DoSetColorWritesEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_ColorWritesEnabled = inEnabled;
        m_backend->SetColorWrites(inEnabled, inEnabled, inEnabled, inEnabled);
    }

    void DoSetMultisampleEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_MultisampleEnabled = inEnabled;
        m_backend->SetMultisample(inEnabled);
    }

    void DoSetDepthWriteEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_DepthWriteEnabled = inEnabled;
        m_backend->SetDepthWrite(inEnabled);
    }

    void DoSetDepthTestEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_DepthTestEnabled = inEnabled;
        m_backend->SetRenderState(inEnabled, QDemonRenderState::DepthTest);
    }

    void DoSetStencilTestEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_StencilTestEnabled = inEnabled;
        m_backend->SetRenderState(inEnabled, QDemonRenderState::StencilTest);
    }

    void DoSetScissorTestEnabled(bool inEnabled)
    {
        m_HardwarePropertyContext.m_ScissorTestEnabled = inEnabled;
        m_backend->SetRenderState(inEnabled, QDemonRenderState::ScissorTest);
    }

    void DoSetScissorRect(QDemonRenderRect inRect)
    {
        m_HardwarePropertyContext.m_ScissorRect = inRect;
        m_backend->SetScissorRect(inRect);
    }

    void DoSetViewport(QDemonRenderRect inViewport)
    {
        m_HardwarePropertyContext.m_Viewport = inViewport;
        m_backend->SetViewportRect(inViewport);
    }

    // Circular dependencies between shader constants and shader programs preclude
    // implementation in header
    void DoSetActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader);
    void DoSetActiveProgramPipeline(QDemonRenderProgramPipeline *inProgramPipeline);

    void DoSetInputAssembler(QSharedPointer<QDemonRenderInputAssembler> inAssembler)
    {
        m_HardwarePropertyContext.m_InputAssembler = inAssembler;
        m_DirtyFlags |= QDemonRenderContextDirtyValues::InputAssembler;
    }

    void DoSetRenderTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer)
    {
        if (inBuffer)
            m_backend->SetRenderTarget(inBuffer->GetFrameBuffertHandle());
        else
            m_backend->SetRenderTarget(m_DefaultOffscreenRenderTarget);

        m_HardwarePropertyContext.m_FrameBuffer = inBuffer;
    }

    void DoSetReadTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer)
    {
        if (inBuffer)
            m_backend->SetReadTarget(inBuffer->GetFrameBuffertHandle());
        else
            m_backend->SetReadTarget(QDemonRenderBackend::QDemonRenderBackendRenderTargetObject(nullptr));
    }

    bool BindShaderToInputAssembler(const QSharedPointer<QDemonRenderInputAssembler> inputAssembler,
                                    QSharedPointer<QDemonRenderShaderProgram> shader);
    bool ApplyPreDrawProperties();
    void OnPostDraw();

public:
    QDemonRenderContextImpl(QSharedPointer<QDemonRenderBackend> inBackend);
    virtual ~QDemonRenderContextImpl();

    QSharedPointer<QDemonRenderBackend> GetBackend() override { return m_backend; }

    void getMaxTextureSize(quint32 &oWidth, quint32 &oHeight) override;

    const char *GetShadingLanguageVersion() override
    {
        return m_backend->GetShadingLanguageVersion();
    }

    QDemonRenderContextType GetRenderContextType() const override
    {
        return m_backend->GetRenderContextType();
    }

    quint32 GetDepthBits() const override
    {
        // only query this if a framebuffer is bound
        if (m_HardwarePropertyContext.m_FrameBuffer)
            return m_backend->GetDepthBits();
        else
            return m_DephBits;
    }

    quint32 GetStencilBits() const override
    {
        // only query this if a framebuffer is bound
        if (m_HardwarePropertyContext.m_FrameBuffer)
            return m_backend->GetStencilBits();
        else
            return m_StencilBits;
    }

    bool GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Enum inCap) const override
    {
        return m_backend->GetRenderBackendCap(inCap);
    }

    bool AreMultisampleTexturesSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::MsTexture);
    }

    bool GetConstantBufferSupport() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ConstantBuffer);
    }

    bool AreDXTImagesSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::DxtImages);
    }

    bool IsDepthStencilSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::DepthStencilTexture);
    }

    bool IsFpRenderTargetSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::FpRenderTarget);
    }

    bool IsTessellationSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Tessellation);
    }

    bool IsGeometryStageSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Geometry);
    }

    bool IsComputeSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::Compute);
    }

    bool IsSampleQuerySupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::SampleQuery);
    }

    bool IsTimerQuerySupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TimerQuery);
    }

    bool IsCommandSyncSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::CommandSync);
    }
    bool IsTextureArraySupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TextureArray);
    }
    bool IsStorageBufferSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::StorageBuffer);
    }
    bool IsAtomicCounterBufferSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AtomicCounterBuffer);
    }
    bool IsShaderImageLoadStoreSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ShaderImageLoadStore);
    }
    bool IsProgramPipelineSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::ProgramPipeline);
    }
    bool IsPathRenderingSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::PathRendering);
    }
    // Are blend modes really supported in HW?
    bool IsAdvancedBlendHwSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AdvancedBlend);
    }
    bool IsAdvancedBlendHwSupportedKHR() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::AdvancedBlendKHR);
    }
    bool IsBlendCoherencySupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::BlendCoherency);
    }
    bool IsStandardDerivativesSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::StandardDerivatives);
    }
    bool IsTextureLodSupported() const override
    {
        return GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TextureLod);
    }

    void SetDefaultRenderTarget(quint64 targetID) override
    {
        m_DefaultOffscreenRenderTarget = (QDemonRenderBackend::QDemonRenderBackendRenderTargetObject)targetID;
    }

    void SetDefaultDepthBufferBitCount(qint32 depthBits) override { m_DephBits = depthBits; }

    virtual QDemonRenderDepthStencilState *
    CreateDepthStencilState(bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc,
                            bool enableStencil,
                            QDemonRenderStencilFunctionArgument &stencilFuncFront,
                            QDemonRenderStencilFunctionArgument &stencilFuncBack,
                            QDemonRenderStencilOperationArgument &depthStencilOpFront,
                            QDemonRenderStencilOperationArgument &depthStencilOpBack) override;
    void SetDepthStencilState(QDemonRenderDepthStencilState *inDepthStencilState) override;
    virtual void StateDestroyed(QDemonRenderDepthStencilState &state);

    QDemonRenderRasterizerState *CreateRasterizerState(float depthBias, float depthScale,
                                                   QDemonRenderFaces::Enum cullFace) override;
    void SetRasterizerState(QDemonRenderRasterizerState *inRasterizerState) override;
    virtual void StateDestroyed(QDemonRenderRasterizerState &state);

    QDemonRenderVertexBuffer *CreateVertexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                 size_t size, quint32 stride,
                                                 QDemonConstDataRef<quint8> bufferData) override;
    QDemonRenderVertexBuffer *GetVertexBuffer(const void *implementationHandle) override;
    virtual void BufferDestroyed(QDemonRenderVertexBuffer &buffer);

    virtual QDemonRenderIndexBuffer *
    CreateIndexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                      QDemonRenderComponentTypes::Enum componentType, size_t size,
                      QDemonConstDataRef<quint8> bufferData) override;
    QDemonRenderIndexBuffer *GetIndexBuffer(const void *implementationHandle) override;
    virtual void BufferDestroyed(QDemonRenderIndexBuffer &buffer);

    virtual QDemonRenderConstantBuffer *
    CreateConstantBuffer(const char *bufferName,
                         QDemonRenderBufferUsageType::Enum usageType, size_t size,
                         QDemonConstDataRef<quint8> bufferData) override;
    QDemonRenderConstantBuffer *GetConstantBuffer(const QString &bufferName) override;
    virtual void BufferDestroyed(QDemonRenderConstantBuffer &buffer);

    virtual quint32 GetNextConstantBufferUnit();

    virtual QDemonRenderStorageBuffer *
    CreateStorageBuffer(const char *bufferName,
                        QDemonRenderBufferUsageType::Enum usageType, size_t size,
                        QDemonConstDataRef<quint8> bufferData, QDemonRenderDataBuffer *pBuffer) override;
    QDemonRenderStorageBuffer *GetStorageBuffer(const QString &bufferName) override;
    virtual void BufferDestroyed(QDemonRenderStorageBuffer &buffer);

    virtual QDemonRenderAtomicCounterBuffer *
    CreateAtomicCounterBuffer(const char *bufferName,
                              QDemonRenderBufferUsageType::Enum usageType, size_t size,
                              QDemonConstDataRef<quint8> bufferData) override;
    QDemonRenderAtomicCounterBuffer *GetAtomicCounterBuffer(const QString &bufferName) override;
    virtual QDemonRenderAtomicCounterBuffer *
    GetAtomicCounterBufferByParam(const QString &paramName) override;
    virtual void BufferDestroyed(QDemonRenderAtomicCounterBuffer &buffer);

    virtual QDemonRenderDrawIndirectBuffer *
    CreateDrawIndirectBuffer(QDemonRenderBufferUsageType::Enum usageType, size_t size,
                             QDemonConstDataRef<quint8> bufferData) override;
    virtual QDemonRenderDrawIndirectBuffer *
    GetDrawIndirectBuffer(QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle) override;
    virtual void BufferDestroyed(QDemonRenderDrawIndirectBuffer &buffer);

    void SetMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) override;

    QDemonRenderOcclusionQuery *CreateOcclusionQuery() override;
    QDemonRenderTimerQuery *CreateTimerQuery() override;
    QSharedPointer<QDemonRenderSync> CreateSync() override;

    QSharedPointer<QDemonRenderTexture2D> CreateTexture2D() override;
    QSharedPointer<QDemonRenderTexture2D> GetTexture2D(const void *implementationHandle) override;
    virtual void TextureDestroyed(QDemonRenderTexture2D *buffer);

    QSharedPointer<QDemonRenderTexture2DArray> CreateTexture2DArray() override;
    virtual void TextureDestroyed(QDemonRenderTexture2DArray *buffer);

    QSharedPointer<QDemonRenderTextureCube> CreateTextureCube() override;
    virtual void TextureDestroyed(QDemonRenderTextureCube *buffer);

    virtual quint32 GetNextTextureUnit();

    QDemonRenderImage2D *CreateImage2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                                       QDemonRenderImageAccessType::Enum inAccess) override;
    virtual void ImageDestroyed(QDemonRenderImage2D &buffer);

    virtual QSharedPointer<QDemonRenderRenderBuffer> CreateRenderBuffer(QDemonRenderRenderBufferFormats::Enum bufferFormat, quint32 width,
                       quint32 height) override;
    QSharedPointer<QDemonRenderRenderBuffer> GetRenderBuffer(const void *implementationHandle) override;
    virtual void RenderBufferDestroyed(QDemonRenderRenderBuffer *buffer);

    QSharedPointer<QDemonRenderFrameBuffer> CreateFrameBuffer() override;
    QSharedPointer<QDemonRenderFrameBuffer> GetFrameBuffer(const void *implementationHandle) override;
    virtual void FrameBufferDestroyed(QDemonRenderFrameBuffer *fb);

    virtual QSharedPointer<QDemonRenderAttribLayout>
    CreateAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) override;
    QSharedPointer<QDemonRenderInputAssembler> CreateInputAssembler(
            QSharedPointer<QDemonRenderAttribLayout> attribLayout, QDemonConstDataRef<QDemonRenderVertexBuffer *> buffers,
            const QDemonRenderIndexBuffer *indexBuffer, QDemonConstDataRef<quint32> strides,
            QDemonConstDataRef<quint32> offsets, QDemonRenderDrawMode::Enum primType, quint32 patchVertexCount) override;
    void SetInputAssembler(QSharedPointer<QDemonRenderInputAssembler> inputAssembler) override;

    QDemonRenderVertFragCompilationResult CompileSource(
            const char *shaderName, QDemonConstDataRef<qint8> vertShader,
            QDemonConstDataRef<qint8> fragShader,
            QDemonConstDataRef<qint8> tessControlShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> tessEvaluationShaderSource = QDemonConstDataRef<qint8>(),
            QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>(),
            bool separateProgram = false,
            QDemonRenderShaderProgramBinaryType::Enum type = QDemonRenderShaderProgramBinaryType::Unknown,
            bool binaryProgram = false) override;

    virtual QDemonRenderVertFragCompilationResult
    CompileBinary(const char *shaderName, QDemonRenderShaderProgramBinaryType::Enum type,
                  QDemonDataRef<qint8> vertShader, QDemonDataRef<qint8> fragShader,
                  QDemonDataRef<qint8> tessControlShaderSource = QDemonDataRef<qint8>(),
                  QDemonDataRef<qint8> tessEvaluationShaderSource = QDemonDataRef<qint8>(),
                  QDemonConstDataRef<qint8> geometryShaderSource = QDemonConstDataRef<qint8>()) override;

    virtual QDemonRenderVertFragCompilationResult
    CompileComputeSource(const char *shaderName, QDemonConstDataRef<qint8> computeShaderSource) override;

    QSharedPointer<QDemonRenderShaderProgram> GetShaderProgram(const void *implementationHandle) override;
    virtual void ShaderDestroyed(QDemonRenderShaderProgram *shader);

    QDemonRenderProgramPipeline *CreateProgramPipeline() override;
    QDemonRenderPathSpecification *CreatePathSpecification() override;
    QDemonRenderPathRender *CreatePathRender(size_t range = 1) override;
    void SetPathProjectionMatrix(const QMatrix4x4 inPathProjection) override;
    void SetPathModelViewMatrix(const QMatrix4x4 inPathModelview) override;
    void SetPathStencilDepthOffset(float inSlope, float inBias) override;
    void SetPathCoverDepthFunc(QDemonRenderBoolOp::Enum inFunc) override;

    virtual QDemonRenderPathFontSpecification *
    CreatePathFontSpecification(const QString &fontName) override;
    virtual void ReleasePathFontSpecification(QDemonRenderPathFontSpecification &inPathSpec);
    QDemonRenderPathFontItem *CreatePathFontItem() override;

    void SetClearColor(QVector4D inClearColor) override;
    QVector4D GetClearColor() const override { return m_HardwarePropertyContext.m_ClearColor; }

    void SetBlendFunction(QDemonRenderBlendFunctionArgument inFunctions) override;
    QDemonRenderBlendFunctionArgument GetBlendFunction() const override
    {
        return m_HardwarePropertyContext.m_BlendFunction;
    }

    void SetBlendEquation(QDemonRenderBlendEquationArgument inEquations) override;
    QDemonRenderBlendEquationArgument GetBlendEquation() const override
    {
        return m_HardwarePropertyContext.m_BlendEquation;
    }

    void SetCullingEnabled(bool inEnabled) override;
    bool IsCullingEnabled() const override { return m_HardwarePropertyContext.m_CullingEnabled; }

    void SetDepthFunction(QDemonRenderBoolOp::Enum inFunction) override;
    QDemonRenderBoolOp::Enum GetDepthFunction() const override
    {
        return m_HardwarePropertyContext.m_DepthFunction;
    }

    void SetBlendingEnabled(bool inEnabled) override;
    bool IsBlendingEnabled() const override
    {
        return m_HardwarePropertyContext.m_BlendingEnabled;
    }

    void SetDepthWriteEnabled(bool inEnabled) override;
    bool IsDepthWriteEnabled() const override
    {
        return m_HardwarePropertyContext.m_DepthWriteEnabled;
    }
    void SetDepthTestEnabled(bool inEnabled) override;
    bool IsDepthTestEnabled() const override
    {
        return m_HardwarePropertyContext.m_DepthTestEnabled;
    }

    void SetStencilTestEnabled(bool inEnabled) override;
    bool IsStencilTestEnabled() const override
    {
        return m_HardwarePropertyContext.m_StencilTestEnabled;
    }

    void SetScissorTestEnabled(bool inEnabled) override;
    bool IsScissorTestEnabled() const override
    {
        return m_HardwarePropertyContext.m_ScissorTestEnabled;
    }
    void SetScissorRect(QDemonRenderRect inRect) override;
    QDemonRenderRect GetScissorRect() const override
    {
        return m_HardwarePropertyContext.m_ScissorRect;
    }

    void SetViewport(QDemonRenderRect inViewport) override;
    QDemonRenderRect GetViewport() const override { return m_HardwarePropertyContext.m_Viewport; }

    void SetColorWritesEnabled(bool inEnabled) override;
    bool IsColorWritesEnabled() const override
    {
        return m_HardwarePropertyContext.m_ColorWritesEnabled;
    }

    void SetMultisampleEnabled(bool inEnabled) override;
    bool IsMultisampleEnabled() const override
    {
        return m_HardwarePropertyContext.m_MultisampleEnabled;
    }

    void SetActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader) override;
    QSharedPointer<QDemonRenderShaderProgram> GetActiveShader() const override
    {
        return m_HardwarePropertyContext.m_ActiveShader;
    }

    void SetActiveProgramPipeline(QDemonRenderProgramPipeline *inProgramPipeline) override;
    QDemonRenderProgramPipeline *GetActiveProgramPipeline() const override
    {
        return m_HardwarePropertyContext.m_ActiveProgramPipeline;
    }

    void DispatchCompute(QSharedPointer<QDemonRenderShaderProgram> inShader, quint32 numGroupsX,
                         quint32 numGroupsY, quint32 numGroupsZ) override;

    void SetDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet) override;
    void SetReadBuffer(QDemonReadFaces::Enum inReadFace) override;

    void ReadPixels(QDemonRenderRect inRect, QDemonRenderReadPixelFormats::Enum inFormat,
                    QDemonDataRef<quint8> inWriteBuffer) override;

    void SetRenderTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer) override;
    void SetReadTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer) override;
    QSharedPointer<QDemonRenderFrameBuffer> GetRenderTarget() const override
    {
        return m_HardwarePropertyContext.m_FrameBuffer;
    }

    void ResetBlendState() override;

    // Push the entire set of properties.
    void PushPropertySet() override { m_PropertyStack.push_back(m_HardwarePropertyContext); }

    // Pop the entire set of properties, potentially forcing the values
    // to opengl.
    void PopPropertySet(bool inForceSetProperties) override;

    // clear current bound render target
    void Clear(QDemonRenderClearFlags flags) override;
    // clear passed in rendertarget
    void Clear(QSharedPointer<QDemonRenderFrameBuffer> fb, QDemonRenderClearFlags flags) override;

    // copy framebuffer content between read target and render target
    void BlitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1, qint32 srcY1,
                         qint32 dstX0, qint32 dstY0, qint32 dstX1, qint32 dstY1,
                         QDemonRenderClearFlags flags,
                         QDemonRenderTextureMagnifyingOp::Enum filter) override;

    void Draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset) override;
    void DrawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset) override;

    QSurfaceFormat format() const override
    {
        return m_backend->format();
    }
    virtual void resetStates()
    {
        PushPropertySet();
        PopPropertySet(true);
    }
};

QT_END_NAMESPACE

#endif
