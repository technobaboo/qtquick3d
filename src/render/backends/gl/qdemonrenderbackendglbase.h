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
#ifndef QDEMON_RENDER_BACKEND_GL_BASE_H
#define QDEMON_RENDER_BACKEND_GL_BASE_H

/// @file qdemonrenderbackendglbase.h
///       NVRender OpenGL Core backend definition.

#include <QtDemonRender/qdemonrenderbackend.h>
#include <QtDemonRender/qdemonopenglutil.h>

#include <QtCore/QVector>

#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLFunctions>

#include <QtOpenGLExtensions/QtOpenGLExtensions>

QT_BEGIN_NAMESPACE

#define NVRENDER_BACKEND_UNUSED(arg) (void)arg;

// Enable this to log opengl errors instead of an assert
//#define RENDER_BACKEND_LOG_GL_ERRORS

///< forward declaration
class QDemonRenderBackendRasterizerStateGL;
class QDemonRenderBackendDepthStencilStateGL;

class QDemonRenderBackendGLBase : public QDemonRenderBackend
{
public:
    /// constructor
    QDemonRenderBackendGLBase(const QSurfaceFormat &format);
    /// destructor
    virtual ~QDemonRenderBackendGLBase();

public:
    /// API Interface
    QDemonRenderContextType GetRenderContextType() const override;
    bool isESCompatible() const;

    const char *GetShadingLanguageVersion() override;
    /// get implementation depended values
    quint32 GetMaxCombinedTextureUnits() override;
    bool GetRenderBackendCap(QDemonRenderBackendCaps::Enum inCap) const override;
    quint32 GetDepthBits() const override;
    quint32 GetStencilBits() const override;
    void GetRenderBackendValue(QDemonRenderBackendQuery::Enum inQuery, qint32 *params) const override;

    /// state get/set functions
    void SetRenderState(bool bEnable, const QDemonRenderState::Enum value) override;
    bool GetRenderState(const QDemonRenderState::Enum value) override;
    virtual QDemonRenderBackendDepthStencilStateObject
    CreateDepthStencilState(bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc,
                            bool enableStencil,
                            QDemonRenderStencilFunctionArgument &stencilFuncFront,
                            QDemonRenderStencilFunctionArgument &stencilFuncBack,
                            QDemonRenderStencilOperationArgument &depthStencilOpFront,
                            QDemonRenderStencilOperationArgument &depthStencilOpBack) override;
    virtual void
    ReleaseDepthStencilState(QDemonRenderBackendDepthStencilStateObject inDepthStencilState) override;
    virtual QDemonRenderBackendRasterizerStateObject
    CreateRasterizerState(float depthBias, float depthScale, QDemonRenderFaces::Enum cullFace) override;
    void ReleaseRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState) override;
    virtual void
    SetDepthStencilState(QDemonRenderBackendDepthStencilStateObject inDepthStencilState) override;
    void SetRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState) override;
    QDemonRenderBoolOp::Enum GetDepthFunc() override;
    void SetDepthFunc(const QDemonRenderBoolOp::Enum func) override;
    bool GetDepthWrite() override;
    void SetDepthWrite(bool bEnable) override;
    void SetColorWrites(bool bRed, bool bGreen, bool bBlue, bool bAlpha) override;
    void SetMultisample(bool bEnable) override;
    void GetBlendFunc(QDemonRenderBlendFunctionArgument *pBlendFuncArg) override;
    void SetBlendFunc(const QDemonRenderBlendFunctionArgument &blendFuncArg) override;
    void SetBlendEquation(const QDemonRenderBlendEquationArgument &pBlendEquArg) override;
    void SetBlendBarrier(void) override;
    void GetScissorRect(QDemonRenderRect *pRect) override;
    void SetScissorRect(const QDemonRenderRect &rect) override;
    void GetViewportRect(QDemonRenderRect *pRect) override;
    void SetViewportRect(const QDemonRenderRect &rect) override;

    void SetClearColor(const QVector4D *pClearColor) override;
    void Clear(QDemonRenderClearFlags flags) override;

    /// resource handling
    QDemonRenderBackendBufferObject CreateBuffer(size_t size,
                                                 QDemonRenderBufferBindFlags bindFlags,
                                                 QDemonRenderBufferUsageType::Enum usage,
                                                 const void *hostPtr = nullptr) override;
    void BindBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) override;
    void ReleaseBuffer(QDemonRenderBackendBufferObject bo) override;
    void UpdateBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags,
                      size_t size, QDemonRenderBufferUsageType::Enum usage,
                      const void *data) override;
    void UpdateBufferRange(QDemonRenderBackendBufferObject bo,
                           QDemonRenderBufferBindFlags bindFlags, size_t offset,
                           size_t size, const void *data) override;
    void *MapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags,
                    size_t offset, size_t length,
                    QDemonRenderBufferAccessFlags accessFlags) override;
    bool UnmapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) override;
    void SetMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) override;

    QDemonRenderBackendQueryObject CreateQuery() override;
    void ReleaseQuery(QDemonRenderBackendQueryObject qo) override;
    void BeginQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType::Enum type) override;
    void EndQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType::Enum type) override;
    void GetQueryResult(QDemonRenderBackendQueryObject qo,
                        QDemonRenderQueryResultType::Enum resultType, quint32 *params) override;
    void GetQueryResult(QDemonRenderBackendQueryObject qo,
                        QDemonRenderQueryResultType::Enum resultType, quint64 *params) override;
    void SetQueryTimer(QDemonRenderBackendQueryObject qo) override;

    QDemonRenderBackendSyncObject CreateSync(QDemonRenderSyncType::Enum tpye,
                                             QDemonRenderSyncFlags syncFlags) override;
    void ReleaseSync(QDemonRenderBackendSyncObject so) override;
    void WaitSync(QDemonRenderBackendSyncObject so, QDemonRenderCommandFlushFlags syncFlags,
                  quint64 timeout) override;

    QDemonRenderBackendRenderTargetObject CreateRenderTarget() override;
    void ReleaseRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendRenderbufferObject rbo) override;
    void RenderTargetAttach(
            QDemonRenderBackendRenderTargetObject rto, QDemonRenderFrameBufferAttachments::Enum attachment,
            QDemonRenderBackendTextureObject to,
            QDemonRenderTextureTargetType::Enum target = QDemonRenderTextureTargetType::Texture2D) override;
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to, qint32 level, qint32 layer) override;
    void SetRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    bool RenderTargetIsValid(QDemonRenderBackendRenderTargetObject rto) override;

    virtual QDemonRenderBackendRenderbufferObject
    CreateRenderbuffer(QDemonRenderRenderBufferFormats::Enum storageFormat, size_t width,
                       size_t height) override;
    void ReleaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo) override;
    bool ResizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                            QDemonRenderRenderBufferFormats::Enum storageFormat,
                            size_t width, size_t height) override;

    QDemonRenderBackendTextureObject CreateTexture() override;
    void BindTexture(QDemonRenderBackendTextureObject to,
                     QDemonRenderTextureTargetType::Enum target, quint32 unit) override;
    void BindImageTexture(QDemonRenderBackendTextureObject to, quint32 unit, qint32 level,
                          bool layered, qint32 layer,
                          QDemonRenderImageAccessType::Enum access,
                          QDemonRenderTextureFormats::Enum format) override;
    void ReleaseTexture(QDemonRenderBackendTextureObject to) override;
    void SetTextureData2D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target, quint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat, size_t width,
                          size_t height, qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;
    void SetTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                QDemonRenderTextureTargetType::Enum target, quint32 level,
                                QDemonRenderTextureFormats::Enum internalFormat,
                                size_t width, size_t height, qint32 border,
                                QDemonRenderTextureFormats::Enum format,
                                const void *hostPtr = nullptr) override;
    void CreateTextureStorage2D(QDemonRenderBackendTextureObject to,
                                QDemonRenderTextureTargetType::Enum target, quint32 levels,
                                QDemonRenderTextureFormats::Enum internalFormat,
                                size_t width, size_t height) override;
    void SetTextureSubData2D(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType::Enum target, quint32 level,
                             qint32 xOffset, qint32 yOffset, size_t width, size_t height,
                             QDemonRenderTextureFormats::Enum format,
                             const void *hostPtr = nullptr) override;
    void SetCompressedTextureData2D(QDemonRenderBackendTextureObject to,
                                    QDemonRenderTextureTargetType::Enum target, quint32 level,
                                    QDemonRenderTextureFormats::Enum internalFormat,
                                    size_t width, size_t height, qint32 border,
                                    size_t imageSize, const void *hostPtr = nullptr) override;
    void SetCompressedTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                          QDemonRenderTextureTargetType::Enum target,
                                          quint32 level,
                                          QDemonRenderTextureFormats::Enum internalFormat,
                                          size_t width, size_t height, qint32 border,
                                          size_t imageSize, const void *hostPtr = nullptr) override;
    void SetCompressedTextureSubData2D(QDemonRenderBackendTextureObject to,
                                       QDemonRenderTextureTargetType::Enum target,
                                       quint32 level, qint32 xOffset, qint32 yOffset,
                                       size_t width, size_t height,
                                       QDemonRenderTextureFormats::Enum format,
                                       size_t imageSize, const void *hostPtr = nullptr) override;
    void SetMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType::Enum target,
                                      size_t samples,
                                      QDemonRenderTextureFormats::Enum internalFormat,
                                      size_t width, size_t height,
                                      bool fixedsamplelocations) override = 0;

    void SetTextureData3D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target, quint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat, size_t width,
                          size_t height, size_t depth, qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;

    void GenerateMipMaps(QDemonRenderBackendTextureObject to,
                         QDemonRenderTextureTargetType::Enum target,
                         QDemonRenderHint::Enum genType) override;

    virtual QDemonRenderTextureSwizzleMode::Enum
    GetTextureSwizzleMode(const QDemonRenderTextureFormats::Enum inFormat) const override;

    QDemonRenderBackendSamplerObject CreateSampler(
            QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
            QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
            QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
            qint32 minLod = -1000, qint32 maxLod = 1000, float lodBias = 0.0,
            QDemonRenderTextureCompareMode::Enum compareMode = QDemonRenderTextureCompareMode::NoCompare,
            QDemonRenderTextureCompareOp::Enum compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
            float anisotropy = 1.0, float *borderColor = nullptr) override;

    void UpdateSampler(
            QDemonRenderBackendSamplerObject so, QDemonRenderTextureTargetType::Enum target,
            QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
            QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
            QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
            float minLod = -1000.0, float maxLod = 1000.0, float lodBias = 0.0,
            QDemonRenderTextureCompareMode::Enum compareMode = QDemonRenderTextureCompareMode::NoCompare,
            QDemonRenderTextureCompareOp::Enum compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
            float anisotropy = 1.0, float *borderColor = nullptr) override;

    void UpdateTextureObject(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType::Enum target, qint32 baseLevel,
                             qint32 maxLevel) override;

    void UpdateTextureSwizzle(QDemonRenderBackendTextureObject to,
                              QDemonRenderTextureTargetType::Enum target,
                              QDemonRenderTextureSwizzleMode::Enum swizzleMode) override;

    void ReleaseSampler(QDemonRenderBackendSamplerObject so) override;

    virtual QDemonRenderBackendAttribLayoutObject
    CreateAttribLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) override;
    void ReleaseAttribLayout(QDemonRenderBackendAttribLayoutObject ao) override;

    virtual QDemonRenderBackendInputAssemblerObject
    CreateInputAssembler(QDemonRenderBackendAttribLayoutObject attribLayout,
                         QDemonConstDataRef<QDemonRenderBackendBufferObject> buffers,
                         const QDemonRenderBackendBufferObject indexBuffer,
                         QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets,
                         quint32 patchVertexCount) override;
    void ReleaseInputAssembler(QDemonRenderBackendInputAssemblerObject iao) override;

    bool SetInputAssembler(QDemonRenderBackendInputAssemblerObject iao,
                           QDemonRenderBackendShaderProgramObject po) override = 0;
    void SetPatchVertexCount(QDemonRenderBackendInputAssemblerObject, quint32) override
    {
        Q_ASSERT(false);
    }

    // shader
    virtual QDemonRenderBackendVertexShaderObject
    CreateVertexShader(QDemonConstDataRef<qint8> source, QString &errorMessage, bool binary) override;
    virtual QDemonRenderBackendFragmentShaderObject
    CreateFragmentShader(QDemonConstDataRef<qint8> source, QString &errorMessage, bool binary) override;
    virtual QDemonRenderBackendTessControlShaderObject
    CreateTessControlShader(QDemonConstDataRef<qint8> source, QString &errorMessage,
                            bool binary) override;
    virtual QDemonRenderBackendTessEvaluationShaderObject
    CreateTessEvaluationShader(QDemonConstDataRef<qint8> source, QString &errorMessage,
                               bool binary) override;
    virtual QDemonRenderBackendGeometryShaderObject
    CreateGeometryShader(QDemonConstDataRef<qint8> source, QString &errorMessage, bool binary) override;
    virtual QDemonRenderBackendComputeShaderObject
    CreateComputeShader(QDemonConstDataRef<qint8> source, QString &errorMessage, bool binary) override;
    void ReleaseVertexShader(QDemonRenderBackendVertexShaderObject vso) override;
    void ReleaseFragmentShader(QDemonRenderBackendFragmentShaderObject fso) override;
    void ReleaseTessControlShader(QDemonRenderBackendTessControlShaderObject tcso) override;
    void ReleaseTessEvaluationShader(QDemonRenderBackendTessEvaluationShaderObject teso) override;
    void ReleaseGeometryShader(QDemonRenderBackendGeometryShaderObject gso) override;
    void ReleaseComputeShader(QDemonRenderBackendComputeShaderObject cso) override;
    void AttachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendVertexShaderObject vso) override;
    void AttachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendFragmentShaderObject fso) override;
    void AttachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessControlShaderObject tcso) override;
    void AttachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessEvaluationShaderObject teso) override;
    void AttachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendGeometryShaderObject gso) override;
    void AttachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendComputeShaderObject cso) override;
    void DetachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendVertexShaderObject vso) override;
    void DetachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendFragmentShaderObject fso) override;
    void DetachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessControlShaderObject tcso) override;
    void DetachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessEvaluationShaderObject teso) override;
    void DetachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendGeometryShaderObject gso) override;
    void DetachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendComputeShaderObject cso) override;
    QDemonRenderBackendShaderProgramObject CreateShaderProgram(bool isSeparable) override;
    void ReleaseShaderProgram(QDemonRenderBackendShaderProgramObject po) override;
    bool LinkProgram(QDemonRenderBackendShaderProgramObject po,
                     QString &errorMessage) override;
    void SetActiveProgram(QDemonRenderBackendShaderProgramObject po) override;
    void DispatchCompute(QDemonRenderBackendShaderProgramObject po, quint32 numGroupsX,
                         quint32 numGroupsY, quint32 numGroupsZ) override;
    QDemonRenderBackendProgramPipeline CreateProgramPipeline() override;
    void ReleaseProgramPipeline(QDemonRenderBackendProgramPipeline po) override;
    void SetActiveProgramPipeline(QDemonRenderBackendProgramPipeline po) override;
    void SetProgramStages(QDemonRenderBackendProgramPipeline ppo,
                          QDemonRenderShaderTypeFlags flags,
                          QDemonRenderBackendShaderProgramObject po) override;

    // uniforms
    qint32 GetConstantCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetConstantInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                               quint32 bufSize, qint32 *numElem,
                               QDemonRenderShaderDataTypes::Enum *type, qint32 *binding,
                               char *nameBuf) override;
    void SetConstantValue(QDemonRenderBackendShaderProgramObject po, quint32 id,
                          QDemonRenderShaderDataTypes::Enum type, qint32 count,
                          const void *value, bool transpose) override;

    // uniform buffers
    qint32 GetConstantBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                     quint32 nameBufSize, qint32 *paramCount,
                                     qint32 *bufferSize, qint32 *length, char *nameBuf) override;
    void GetConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                       qint32 *indices) override;
    void GetConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                             quint32 count, quint32 *indices, qint32 *type,
                                             qint32 *size, qint32 *offset) override;
    void ProgramSetConstantBlock(QDemonRenderBackendShaderProgramObject po,
                                 quint32 blockIndex, quint32 binding) override;
    void ProgramSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    // storage buffers
    qint32 GetStorageBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                    quint32 nameBufSize, qint32 *paramCount,
                                    qint32 *bufferSize, qint32 *length, char *nameBuf) override;
    void ProgramSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    // atomic counter buffers
    qint32 GetAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                          quint32 id, quint32 nameBufSize, qint32 *paramCount,
                                          qint32 *bufferSize, qint32 *length,
                                          char *nameBuf) override;
    void ProgramSetAtomicCounterBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    /// draw calls
    void Draw(QDemonRenderDrawMode::Enum drawMode, quint32 start, quint32 count) override;
    void DrawIndirect(QDemonRenderDrawMode::Enum drawMode, const void *indirect) override;
    void DrawIndexed(QDemonRenderDrawMode::Enum drawMode, quint32 count,
                     QDemonRenderComponentTypes::Enum type, const void *indices) override;
    void DrawIndexedIndirect(QDemonRenderDrawMode::Enum drawMode,
                             QDemonRenderComponentTypes::Enum type, const void *indirect) override;

    // read calls
    void ReadPixel(QDemonRenderBackendRenderTargetObject rto, qint32 x, qint32 y, qint32 width,
                   qint32 height, QDemonRenderReadPixelFormats::Enum inFormat, void *pixels) override;

    // NV path rendering
    QDemonRenderBackendPathObject CreatePathNVObject(size_t range) override;
    // Pathing requires gl4 backend.
    void SetPathSpecification(QDemonRenderBackendPathObject, QDemonConstDataRef<quint8>,
                              QDemonConstDataRef<float>) override
    {
    }

    ///< Bounds of the fill and stroke
    QDemonBounds3 GetPathObjectBoundingBox(QDemonRenderBackendPathObject /*inPathObject*/) override
    {
        return QDemonBounds3();
    }
    QDemonBounds3 GetPathObjectFillBox(QDemonRenderBackendPathObject /*inPathObject*/) override
    {
        return QDemonBounds3();
    }
    QDemonBounds3 GetPathObjectStrokeBox(QDemonRenderBackendPathObject /*inPathObject*/) override
    {
        return QDemonBounds3();
    }

    /**
         *	Defaults to 0 if unset.
         */
    void SetStrokeWidth(QDemonRenderBackendPathObject /*inPathObject*/, float) override {}
    void SetPathProjectionMatrix(const QMatrix4x4 /*inPathProjection*/) override {}
    void SetPathModelViewMatrix(const QMatrix4x4 /*inPathModelview*/) override {}
    void SetPathStencilDepthOffset(float /*inSlope*/, float /*inBias*/) override {}
    void SetPathCoverDepthFunc(QDemonRenderBoolOp::Enum /*inDepthFunction*/) override {}
    void StencilStrokePath(QDemonRenderBackendPathObject /*inPathObject*/) override {}
    void StencilFillPath(QDemonRenderBackendPathObject /*inPathObject*/) override {}
    void ReleasePathNVObject(QDemonRenderBackendPathObject po, size_t range) override;

    void LoadPathGlyphs(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum,
                        const void *, QDemonRenderPathFontStyleFlags, size_t,
                        QDemonRenderPathFormatType::Enum, const void *,
                        QDemonRenderPathMissingGlyphs::Enum, QDemonRenderBackendPathObject,
                        float) override;
    virtual QDemonRenderPathReturnValues::Enum
    LoadPathGlyphsIndexed(QDemonRenderBackendPathObject po, QDemonRenderPathFontTarget::Enum fontTarget,
                          const void *fontName, QDemonRenderPathFontStyleFlags fontStyle,
                          quint32 firstGlyphIndex, size_t numGlyphs,
                          QDemonRenderBackendPathObject pathParameterTemplate, float emScale) override;
    virtual QDemonRenderBackendPathObject
    LoadPathGlyphsIndexedRange(QDemonRenderPathFontTarget::Enum, const void *,
                               QDemonRenderPathFontStyleFlags,
                               QDemonRenderBackend::QDemonRenderBackendPathObject, float, quint32 *) override;
    void LoadPathGlyphRange(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum,
                            const void *, QDemonRenderPathFontStyleFlags, quint32, size_t,
                            QDemonRenderPathMissingGlyphs::Enum, QDemonRenderBackendPathObject,
                            float) override;
    void GetPathMetrics(QDemonRenderBackendPathObject, size_t,
                        QDemonRenderPathGlyphFontMetricFlags, QDemonRenderPathFormatType::Enum,
                        const void *, size_t, float *) override;
    void GetPathMetricsRange(QDemonRenderBackendPathObject, size_t,
                             QDemonRenderPathGlyphFontMetricFlags, size_t, float *) override;
    void GetPathSpacing(QDemonRenderBackendPathObject, size_t, QDemonRenderPathListMode::Enum,
                        QDemonRenderPathFormatType::Enum, const void *, float, float,
                        QDemonRenderPathTransformType::Enum, float *) override;

    void StencilFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                  QDemonRenderPathFormatType::Enum, const void *,
                                  QDemonRenderPathFillMode::Enum, quint32,
                                  QDemonRenderPathTransformType::Enum, const float *) override;
    void StencilStrokePathInstancedN(QDemonRenderBackendPathObject, size_t,
                                     QDemonRenderPathFormatType::Enum, const void *, qint32,
                                     quint32, QDemonRenderPathTransformType::Enum,
                                     const float *) override;
    void CoverFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                QDemonRenderPathFormatType::Enum, const void *,
                                QDemonRenderPathCoverMode::Enum,
                                QDemonRenderPathTransformType::Enum, const float *) override;
    void CoverStrokePathInstanced(QDemonRenderBackendPathObject, size_t,
                                  QDemonRenderPathFormatType::Enum, const void *,
                                  QDemonRenderPathCoverMode::Enum,
                                  QDemonRenderPathTransformType::Enum, const float *) override;

    QSurfaceFormat format() const override
    {
        return m_format;
    }

protected:
    virtual bool compileSource(GLuint shaderID, QDemonConstDataRef<qint8> source,
                               QString &errorMessage, bool binary);
    virtual const char *getVersionString();
    virtual const char *getVendorString();
    virtual const char *getRendererString();
    virtual const char *getExtensionString();

    virtual void setAndInspectHardwareCaps();

protected:
    GLConversion m_Conversion; ///< Class for conversion from base type to GL types
    QStringList m_extensions; ///< contains the OpenGL extension string
    qint32 m_MaxAttribCount; ///< Maximum attributes which can be used
    QVector<GLenum> m_DrawBuffersArray; ///< Contains the drawbuffer enums
    QSurfaceFormat m_format;

    QDemonRenderBackendRasterizerStateGL *m_pCurrentRasterizerState; ///< this holds the current rasterizer state
    QDemonRenderBackendDepthStencilStateGL *m_pCurrentDepthStencilState; ///< this holds the current depth stencil state

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
    void checkGLError(const char *function, const char *file, const unsigned int line) const;
#else
    void checkGLError() const;
#endif
    QOpenGLFunctions *m_glFunctions;
    QOpenGLExtraFunctions *m_glExtraFunctions;
};

QT_END_NAMESPACE

#endif
