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
#include <QtDemonRender/qdemonrenderbackendnull.h>

#include <QSurfaceFormat>

QT_BEGIN_NAMESPACE

namespace {
struct SNullBackend : public QDemonRenderBackend
{
    SNullBackend()
    {
    }

    virtual ~SNullBackend()
    {
    }

    /// backend interface

    QDemonRenderContextType GetRenderContextType() const override
    {
        return QDemonRenderContextValues::NullContext;
    }
    const char *GetShadingLanguageVersion() override { return ""; }
    quint32 GetMaxCombinedTextureUnits() override { return 32; }
    bool GetRenderBackendCap(QDemonRenderBackendCaps::Enum) const override { return false; }
    void GetRenderBackendValue(QDemonRenderBackendQuery::Enum inQuery, qint32 *params) const override
    {
        if (params) {
            switch (inQuery) {
            case QDemonRenderBackendQuery::MaxTextureSize:
                *params = 4096;
                break;
            case QDemonRenderBackendQuery::MaxTextureArrayLayers:
                *params = 0;
                break;
            default:
                Q_ASSERT(false);
                *params = 0;
                break;
            }
        }
    }
    quint32 GetDepthBits() const override { return 16; }
    quint32 GetStencilBits() const override { return 0; }
    void SetRenderState(bool, const QDemonRenderState::Enum) override {}
    bool GetRenderState(const QDemonRenderState::Enum) override { return false; }
    virtual QDemonRenderBackendDepthStencilStateObject
    CreateDepthStencilState(bool, bool, QDemonRenderBoolOp::Enum, bool,
                            QDemonRenderStencilFunctionArgument &, QDemonRenderStencilFunctionArgument &,
                            QDemonRenderStencilOperationArgument &, QDemonRenderStencilOperationArgument &) override
    {
        return QDemonRenderBackendDepthStencilStateObject(1);
    }
    void ReleaseDepthStencilState(QDemonRenderBackendDepthStencilStateObject) override {}
    QDemonRenderBackendRasterizerStateObject CreateRasterizerState(float, float,
                                                                       QDemonRenderFaces::Enum) override
    {
        return QDemonRenderBackendRasterizerStateObject(1);
    }
    void ReleaseRasterizerState(QDemonRenderBackendRasterizerStateObject) override {}
    void SetDepthStencilState(QDemonRenderBackendDepthStencilStateObject) override {}
    void SetRasterizerState(QDemonRenderBackendRasterizerStateObject) override {}
    QDemonRenderBoolOp::Enum GetDepthFunc() override { return QDemonRenderBoolOp::Equal; }
    void SetDepthFunc(const QDemonRenderBoolOp::Enum) override {}
    bool GetDepthWrite() override { return false; }

    void SetDepthWrite(bool) override {}
    void SetColorWrites(bool, bool, bool, bool) override {}
    void SetMultisample(bool) override {}
    void GetBlendFunc(QDemonRenderBlendFunctionArgument *) override {}
    void SetBlendFunc(const QDemonRenderBlendFunctionArgument &) override {}
    void SetBlendEquation(const QDemonRenderBlendEquationArgument &) override {}
    void SetBlendBarrier(void) override {}
    void GetScissorRect(QDemonRenderRect *) override {}
    void SetScissorRect(const QDemonRenderRect &) override {}
    void GetViewportRect(QDemonRenderRect *) override {}
    void SetViewportRect(const QDemonRenderRect &) override {}
    void SetClearColor(const QVector4D *) override {}
    void Clear(QDemonRenderClearFlags) override {}
    QDemonRenderBackendBufferObject CreateBuffer(size_t, QDemonRenderBufferBindFlags,
                                                     QDemonRenderBufferUsageType::Enum, const void *) override
    {
        return QDemonRenderBackendBufferObject(1);
    }
    void BindBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags) override {}
    void ReleaseBuffer(QDemonRenderBackendBufferObject) override {}

    void UpdateBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t,
                              QDemonRenderBufferUsageType::Enum, const void *) override
    {
    }
    void UpdateBufferRange(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t, size_t,
                           const void *) override
    {
    }
    void *MapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t, size_t,
                            QDemonRenderBufferAccessFlags) override
    {
        return nullptr;
    }
    bool UnmapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags) override { return true; }
    void SetMemoryBarrier(QDemonRenderBufferBarrierFlags) override {}
    QDemonRenderBackendQueryObject CreateQuery() override { return QDemonRenderBackendQueryObject(1); }
    void ReleaseQuery(QDemonRenderBackendQueryObject) override {}
    void BeginQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum) override {}
    void EndQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum) override {}
    void GetQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType::Enum,
                                quint32 *) override {}
    void GetQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType::Enum,
                                quint64 *) override {}
    void SetQueryTimer(QDemonRenderBackendQueryObject) override {}
    QDemonRenderBackendSyncObject CreateSync(QDemonRenderSyncType::Enum, QDemonRenderSyncFlags) override
    {
        return QDemonRenderBackendSyncObject(1);
    }
    void ReleaseSync(QDemonRenderBackendSyncObject) override {}
    void WaitSync(QDemonRenderBackendSyncObject, QDemonRenderCommandFlushFlags, quint64) override {}
    QDemonRenderBackendRenderTargetObject CreateRenderTarget() override
    {
        return QDemonRenderBackendRenderTargetObject(1);
    }
    void ReleaseRenderTarget(QDemonRenderBackendRenderTargetObject) override {}
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                    QDemonRenderFrameBufferAttachments::Enum,
                                    QDemonRenderBackendRenderbufferObject) override
    {
    }
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                    QDemonRenderFrameBufferAttachments::Enum,
                                    QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum) override
    {
    }
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                    QDemonRenderFrameBufferAttachments::Enum,
                                    QDemonRenderBackendTextureObject, qint32, qint32) override
    {
    }
    void SetRenderTarget(QDemonRenderBackendRenderTargetObject) override {}
    bool RenderTargetIsValid(QDemonRenderBackendRenderTargetObject) override { return false; }
    void SetReadTarget(QDemonRenderBackendRenderTargetObject) override {}
    void SetDrawBuffers(QDemonRenderBackendRenderTargetObject, QDemonConstDataRef<qint32>) override {}
    void SetReadBuffer(QDemonRenderBackendRenderTargetObject, QDemonReadFaces::Enum) override {}

    void BlitFramebuffer(qint32, qint32, qint32, qint32, qint32, qint32, qint32, qint32,
                                 QDemonRenderClearFlags, QDemonRenderTextureMagnifyingOp::Enum) override
    {
    }
    QDemonRenderBackendRenderbufferObject CreateRenderbuffer(QDemonRenderRenderBufferFormats::Enum,
                                                                 size_t, size_t) override
    {
        return QDemonRenderBackendRenderbufferObject(1);
    }
    void ReleaseRenderbuffer(QDemonRenderBackendRenderbufferObject) override {}

    bool ResizeRenderbuffer(QDemonRenderBackendRenderbufferObject,
                                    QDemonRenderRenderBufferFormats::Enum, size_t, size_t) override
    {
        return false;
    }
    QDemonRenderBackendTextureObject CreateTexture() override { return QDemonRenderBackendTextureObject(1); }

    void SetTextureData2D(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum,
                                  quint32, QDemonRenderTextureFormats::Enum, size_t, size_t, qint32,
                                  QDemonRenderTextureFormats::Enum, const void *) override
    {
    }
    void SetTextureDataCubeFace(QDemonRenderBackendTextureObject,
                                        QDemonRenderTextureTargetType::Enum, quint32,
                                        QDemonRenderTextureFormats::Enum, size_t, size_t, qint32,
                                        QDemonRenderTextureFormats::Enum, const void *) override
    {
    }
    void CreateTextureStorage2D(QDemonRenderBackendTextureObject,
                                        QDemonRenderTextureTargetType::Enum, quint32,
                                        QDemonRenderTextureFormats::Enum, size_t, size_t) override
    {
    }
    void SetTextureSubData2D(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum,
                                     quint32, qint32, qint32, size_t, size_t,
                                     QDemonRenderTextureFormats::Enum, const void *) override
    {
    }
    void SetCompressedTextureData2D(QDemonRenderBackendTextureObject,
                                            QDemonRenderTextureTargetType::Enum, quint32,
                                            QDemonRenderTextureFormats::Enum, size_t, size_t, qint32,
                                            size_t, const void *) override
    {
    }
    void SetCompressedTextureDataCubeFace(QDemonRenderBackendTextureObject,
                                                  QDemonRenderTextureTargetType::Enum, quint32,
                                                  QDemonRenderTextureFormats::Enum, size_t, size_t,
                                                  qint32, size_t, const void *) override
    {
    }
    void SetCompressedTextureSubData2D(QDemonRenderBackendTextureObject,
                                               QDemonRenderTextureTargetType::Enum, quint32, qint32, qint32,
                                               size_t, size_t, QDemonRenderTextureFormats::Enum, size_t,
                                               const void *) override
    {
    }
    void SetMultisampledTextureData2D(QDemonRenderBackendTextureObject,
                                              QDemonRenderTextureTargetType::Enum, size_t,
                                              QDemonRenderTextureFormats::Enum, size_t, size_t, bool) override
    {
    }
    void SetTextureData3D(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum,
                                  quint32, QDemonRenderTextureFormats::Enum, size_t, size_t, size_t,
                                  qint32, QDemonRenderTextureFormats::Enum, const void *) override
    {
    }
    void GenerateMipMaps(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum,
                                 QDemonRenderHint::Enum) override
    {
    }
    void BindTexture(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum, quint32) override
    {
    }
    void BindImageTexture(QDemonRenderBackendTextureObject, quint32, qint32, bool, qint32,
                                  QDemonRenderImageAccessType::Enum, QDemonRenderTextureFormats::Enum) override
    {
    }
    void ReleaseTexture(QDemonRenderBackendTextureObject) override {}

    virtual QDemonRenderTextureSwizzleMode::Enum
    GetTextureSwizzleMode(const QDemonRenderTextureFormats::Enum) const override
    {
        return QDemonRenderTextureSwizzleMode::NoSwizzle;
    }

    virtual QDemonRenderBackendSamplerObject
    CreateSampler(QDemonRenderTextureMinifyingOp::Enum, QDemonRenderTextureMagnifyingOp::Enum,
                  QDemonRenderTextureCoordOp::Enum, QDemonRenderTextureCoordOp::Enum,
                  QDemonRenderTextureCoordOp::Enum, qint32, qint32, float,
                  QDemonRenderTextureCompareMode::Enum, QDemonRenderTextureCompareOp::Enum, float, float *) override
    {
        return QDemonRenderBackendSamplerObject(1);
    }

    void UpdateSampler(QDemonRenderBackendSamplerObject, QDemonRenderTextureTargetType::Enum,
                               QDemonRenderTextureMinifyingOp::Enum, QDemonRenderTextureMagnifyingOp::Enum,
                               QDemonRenderTextureCoordOp::Enum, QDemonRenderTextureCoordOp::Enum,
                               QDemonRenderTextureCoordOp::Enum, float, float, float,
                               QDemonRenderTextureCompareMode::Enum, QDemonRenderTextureCompareOp::Enum,
                               float, float *) override
    {
    }

    void UpdateTextureObject(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum,
                                     qint32, qint32) override
    {
    }

    void UpdateTextureSwizzle(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum,
                                      QDemonRenderTextureSwizzleMode::Enum) override
    {
    }

    void ReleaseSampler(QDemonRenderBackendSamplerObject) override {}

    virtual QDemonRenderBackendAttribLayoutObject
        CreateAttribLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry>) override
    {
        return QDemonRenderBackendAttribLayoutObject(1);
    }

    void ReleaseAttribLayout(QDemonRenderBackendAttribLayoutObject) override {}

    QDemonRenderBackendInputAssemblerObject CreateInputAssembler(
        QDemonRenderBackendAttribLayoutObject, QDemonConstDataRef<QDemonRenderBackendBufferObject>,
        const QDemonRenderBackendBufferObject, QDemonConstDataRef<quint32>, QDemonConstDataRef<quint32>, quint32) override
    {
        return QDemonRenderBackendInputAssemblerObject(1);
    }
    void ReleaseInputAssembler(QDemonRenderBackendInputAssemblerObject) override {}
    bool SetInputAssembler(QDemonRenderBackendInputAssemblerObject,
                                   QDemonRenderBackendShaderProgramObject) override
    {
        return false;
    }
    void SetPatchVertexCount(QDemonRenderBackendInputAssemblerObject, quint32) override {}
    QDemonRenderBackendVertexShaderObject CreateVertexShader(QDemonConstDataRef<qint8>,
                                                                 QByteArray &, bool) override
    {
        return QDemonRenderBackendVertexShaderObject(1);
    }
    void ReleaseVertexShader(QDemonRenderBackendVertexShaderObject) override {}
    QDemonRenderBackendFragmentShaderObject CreateFragmentShader(QDemonConstDataRef<qint8>,
                                                                     QByteArray &, bool) override
    {
        return QDemonRenderBackendFragmentShaderObject(1);
    }
    void ReleaseFragmentShader(QDemonRenderBackendFragmentShaderObject) override {}
    QDemonRenderBackendTessControlShaderObject CreateTessControlShader(QDemonConstDataRef<qint8>,
                                                                           QByteArray &, bool) override
    {
        return QDemonRenderBackendTessControlShaderObject(1);
    }
    void ReleaseTessControlShader(QDemonRenderBackendTessControlShaderObject) override {}
    virtual QDemonRenderBackendTessEvaluationShaderObject
    CreateTessEvaluationShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendTessEvaluationShaderObject(1);
    }
    void ReleaseTessEvaluationShader(QDemonRenderBackendTessEvaluationShaderObject) override {}
    QDemonRenderBackendGeometryShaderObject CreateGeometryShader(QDemonConstDataRef<qint8>,
                                                                     QByteArray &, bool) override
    {
        return QDemonRenderBackendGeometryShaderObject(1);
    }
    void ReleaseGeometryShader(QDemonRenderBackendGeometryShaderObject) override {}
    QDemonRenderBackendComputeShaderObject CreateComputeShader(QDemonConstDataRef<qint8>,
                                                                   QByteArray &, bool) override
    {
        return QDemonRenderBackendComputeShaderObject(1);
    }
    void ReleaseComputeShader(QDemonRenderBackendComputeShaderObject) override {}
    void AttachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendVertexShaderObject) override
    {
    }
    void AttachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendFragmentShaderObject) override
    {
    }
    void AttachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendTessControlShaderObject) override
    {
    }
    void AttachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendTessEvaluationShaderObject) override
    {
    }
    void AttachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendGeometryShaderObject) override
    {
    }
    void AttachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendComputeShaderObject) override
    {
    }
    void DetachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendVertexShaderObject) override
    {
    }
    void DetachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendFragmentShaderObject) override
    {
    }
    void DetachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendTessControlShaderObject) override
    {
    }
    void DetachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendTessEvaluationShaderObject) override
    {
    }
    void DetachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendGeometryShaderObject) override
    {
    }
    void DetachShader(QDemonRenderBackendShaderProgramObject,
                              QDemonRenderBackendComputeShaderObject) override
    {
    }
    QDemonRenderBackendShaderProgramObject CreateShaderProgram(bool) override
    {
        return QDemonRenderBackendShaderProgramObject(1);
    }
    void ReleaseShaderProgram(QDemonRenderBackendShaderProgramObject) override {}
    QDemonRenderBackendProgramPipeline CreateProgramPipeline() override
    {
        return QDemonRenderBackendProgramPipeline(1);
    }
    void ReleaseProgramPipeline(QDemonRenderBackendProgramPipeline) override {}

    bool LinkProgram(QDemonRenderBackendShaderProgramObject, QByteArray &) override { return false; }
    void SetActiveProgram(QDemonRenderBackendShaderProgramObject) override {}
    void SetActiveProgramPipeline(QDemonRenderBackendProgramPipeline) override {}
    void SetProgramStages(QDemonRenderBackendProgramPipeline, QDemonRenderShaderTypeFlags,
                                  QDemonRenderBackendShaderProgramObject) override {}
    void DispatchCompute(QDemonRenderBackendShaderProgramObject, quint32, quint32, quint32) override {}
    qint32 GetConstantCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 GetConstantBufferCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 GetConstantInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32, qint32 *,
                                      QDemonRenderShaderDataTypes::Enum *, qint32 *, char *) override
    {
        return 0;
    }

    qint32 GetConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32,
                                            qint32 *, qint32 *, qint32 *, char *) override
    {
        return 0;
    }

    void GetConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject, quint32, qint32 *) override
    {
    }
    void GetConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject, quint32,
                                                     quint32 *, qint32 *, qint32 *, qint32 *) override {}
    void ProgramSetConstantBlock(QDemonRenderBackendShaderProgramObject, quint32, quint32) override {}
    void ProgramSetConstantBuffer(quint32, QDemonRenderBackendBufferObject) override {}

    qint32 GetStorageBufferCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 GetStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32,
                                           qint32 *, qint32 *, qint32 *, char *) override
    {
        return -1;
    }
    void ProgramSetStorageBuffer(quint32, QDemonRenderBackendBufferObject) override {}

    qint32 GetAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 GetAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32,
                                                 qint32 *, qint32 *, qint32 *, char *) override
    {
        return -1;
    }
    void ProgramSetAtomicCounterBuffer(quint32, QDemonRenderBackendBufferObject) override {}

    void SetConstantValue(QDemonRenderBackendShaderProgramObject, quint32,
                                  QDemonRenderShaderDataTypes::Enum, qint32, const void *, bool) override
    {
    }

    void Draw(QDemonRenderDrawMode::Enum, quint32, quint32) override {}
    void DrawIndirect(QDemonRenderDrawMode::Enum, const void *) override {}

    void DrawIndexed(QDemonRenderDrawMode::Enum, quint32, QDemonRenderComponentTypes::Enum,
                             const void *) override
    {
    }
    void DrawIndexedIndirect(QDemonRenderDrawMode::Enum, QDemonRenderComponentTypes::Enum,
                                     const void *) override
    {
    }

    void ReadPixel(QDemonRenderBackendRenderTargetObject, qint32, qint32, qint32, qint32,
                           QDemonRenderReadPixelFormats::Enum, void *) override
    {
    }

    QDemonRenderBackendPathObject CreatePathNVObject(size_t) override
    {
        return QDemonRenderBackendPathObject(1);
    }
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
    void ReleasePathNVObject(QDemonRenderBackendPathObject, size_t) override {}

    void LoadPathGlyphs(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum,
                                const void *, QDemonRenderPathFontStyleFlags, size_t,
                                QDemonRenderPathFormatType::Enum, const void *,
                                QDemonRenderPathMissingGlyphs::Enum, QDemonRenderBackendPathObject, float) override
    {
    }
    virtual QDemonRenderPathReturnValues::Enum
    LoadPathGlyphsIndexed(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum, const void *,
                          QDemonRenderPathFontStyleFlags, quint32, size_t, QDemonRenderBackendPathObject,
                          float) override
    {
        return QDemonRenderPathReturnValues::FontUnavailable;
    }
    QDemonRenderBackendPathObject LoadPathGlyphsIndexedRange(QDemonRenderPathFontTarget::Enum,
                                                                 const void *,
                                                                 QDemonRenderPathFontStyleFlags,
                                                                 QDemonRenderBackendPathObject, float,
                                                                 quint32 *) override
    {
        return QDemonRenderBackendPathObject(1);
    }
    void LoadPathGlyphRange(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum,
                                    const void *, QDemonRenderPathFontStyleFlags, quint32, size_t,
                                    QDemonRenderPathMissingGlyphs::Enum, QDemonRenderBackendPathObject,
                                    float) override
    {
    }
    void GetPathMetrics(QDemonRenderBackendPathObject, size_t, QDemonRenderPathGlyphFontMetricFlags,
                                QDemonRenderPathFormatType::Enum, const void *, size_t, float *) override
    {
    }
    void GetPathMetricsRange(QDemonRenderBackendPathObject, size_t,
                                     QDemonRenderPathGlyphFontMetricFlags, size_t, float *) override
    {
    }
    void GetPathSpacing(QDemonRenderBackendPathObject, size_t, QDemonRenderPathListMode::Enum,
                                QDemonRenderPathFormatType::Enum, const void *, float, float,
                                QDemonRenderPathTransformType::Enum, float *) override
    {
    }

    void StencilFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                          QDemonRenderPathFormatType::Enum, const void *,
                                          QDemonRenderPathFillMode::Enum, quint32,
                                          QDemonRenderPathTransformType::Enum, const float *) override
    {
    }
    void StencilStrokePathInstancedN(QDemonRenderBackendPathObject, size_t,
                                             QDemonRenderPathFormatType::Enum, const void *, qint32,
                                             quint32, QDemonRenderPathTransformType::Enum, const float *) override
    {
    }
    void CoverFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                        QDemonRenderPathFormatType::Enum, const void *,
                                        QDemonRenderPathCoverMode::Enum,
                                        QDemonRenderPathTransformType::Enum, const float *) override
    {
    }
    void CoverStrokePathInstanced(QDemonRenderBackendPathObject, size_t,
                                          QDemonRenderPathFormatType::Enum, const void *,
                                          QDemonRenderPathCoverMode::Enum,
                                          QDemonRenderPathTransformType::Enum, const float *) override
    {
    }
    QSurfaceFormat format() const override
    {
        return QSurfaceFormat();
    }
};
}

QSharedPointer<QDemonRenderBackend> QDemonRenderBackendNULL::CreateBackend()
{
    return QSharedPointer<QDemonRenderBackend>(new SNullBackend());
}

QT_END_NAMESPACE
