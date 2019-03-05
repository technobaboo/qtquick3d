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
struct QDemonNullBackend : public QDemonRenderBackend
{
    ~QDemonNullBackend() override = default;

    /// backend interface

    QDemonRenderContextType getRenderContextType() const override { return QDemonRenderContextValues::NullContext; }
    const char *getShadingLanguageVersion() override { return ""; }
    quint32 getMaxCombinedTextureUnits() override { return 32; }
    bool getRenderBackendCap(QDemonRenderBackendCaps::Enum) const override { return false; }
    void getRenderBackendValue(QDemonRenderBackendQuery::Enum inQuery, qint32 *params) const override
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
    quint32 getDepthBits() const override { return 16; }
    quint32 getStencilBits() const override { return 0; }
    void setRenderState(bool, const QDemonRenderState::Enum) override {}
    bool getRenderState(const QDemonRenderState::Enum) override { return false; }
    QDemonRenderBackendDepthStencilStateObject createDepthStencilState(bool,
                                                                       bool,
                                                                       QDemonRenderBoolOp::Enum,
                                                                       bool,
                                                                       QDemonRenderStencilFunctionArgument &,
                                                                       QDemonRenderStencilFunctionArgument &,
                                                                       QDemonRenderStencilOperationArgument &,
                                                                       QDemonRenderStencilOperationArgument &) override
    {
        return QDemonRenderBackendDepthStencilStateObject(1);
    }
    void releaseDepthStencilState(QDemonRenderBackendDepthStencilStateObject) override {}
    QDemonRenderBackendRasterizerStateObject createRasterizerState(float, float, QDemonRenderFaces::Enum) override
    {
        return QDemonRenderBackendRasterizerStateObject(1);
    }
    void releaseRasterizerState(QDemonRenderBackendRasterizerStateObject) override {}
    void setDepthStencilState(QDemonRenderBackendDepthStencilStateObject) override {}
    void setRasterizerState(QDemonRenderBackendRasterizerStateObject) override {}
    QDemonRenderBoolOp::Enum getDepthFunc() override { return QDemonRenderBoolOp::Equal; }
    void setDepthFunc(const QDemonRenderBoolOp::Enum) override {}
    bool getDepthWrite() override { return false; }

    void setDepthWrite(bool) override {}
    void setColorWrites(bool, bool, bool, bool) override {}
    void setMultisample(bool) override {}
    void getBlendFunc(QDemonRenderBlendFunctionArgument *) override {}
    void setBlendFunc(const QDemonRenderBlendFunctionArgument &) override {}
    void setBlendEquation(const QDemonRenderBlendEquationArgument &) override {}
    void setBlendBarrier(void) override {}
    void getScissorRect(QRect *) override {}
    void setScissorRect(const QRect &) override {}
    void getViewportRect(QRect *) override {}
    void setViewportRect(const QRect &) override {}
    void setClearColor(const QVector4D *) override {}
    void clear(QDemonRenderClearFlags) override {}
    QDemonRenderBackendBufferObject createBuffer(size_t, QDemonRenderBufferBindFlags, QDemonRenderBufferUsageType::Enum, const void *) override
    {
        return QDemonRenderBackendBufferObject(1);
    }
    void bindBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags) override {}
    void releaseBuffer(QDemonRenderBackendBufferObject) override {}

    void updateBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t, QDemonRenderBufferUsageType::Enum, const void *) override
    {
    }
    void updateBufferRange(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t, size_t, const void *) override
    {
    }
    void *mapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t, size_t, QDemonRenderBufferAccessFlags) override
    {
        return nullptr;
    }
    bool unmapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags) override { return true; }
    void setMemoryBarrier(QDemonRenderBufferBarrierFlags) override {}
    QDemonRenderBackendQueryObject createQuery() override { return QDemonRenderBackendQueryObject(1); }
    void releaseQuery(QDemonRenderBackendQueryObject) override {}
    void beginQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum) override {}
    void endQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum) override {}
    void getQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType::Enum, quint32 *) override {}
    void getQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType::Enum, quint64 *) override {}
    void setQueryTimer(QDemonRenderBackendQueryObject) override {}
    QDemonRenderBackendSyncObject createSync(QDemonRenderSyncType::Enum, QDemonRenderSyncFlags) override
    {
        return QDemonRenderBackendSyncObject(1);
    }
    void releaseSync(QDemonRenderBackendSyncObject) override {}
    void waitSync(QDemonRenderBackendSyncObject, QDemonRenderCommandFlushFlags, quint64) override {}
    QDemonRenderBackendRenderTargetObject createRenderTarget() override
    {
        return QDemonRenderBackendRenderTargetObject(1);
    }
    void releaseRenderTarget(QDemonRenderBackendRenderTargetObject) override {}
    void renderTargetAttach(QDemonRenderBackendRenderTargetObject, QDemonRenderFrameBufferAttachments::Enum, QDemonRenderBackendRenderbufferObject) override
    {
    }
    void renderTargetAttach(QDemonRenderBackendRenderTargetObject,
                            QDemonRenderFrameBufferAttachments::Enum,
                            QDemonRenderBackendTextureObject,
                            QDemonRenderTextureTargetType::Enum) override
    {
    }
    void renderTargetAttach(QDemonRenderBackendRenderTargetObject,
                            QDemonRenderFrameBufferAttachments::Enum,
                            QDemonRenderBackendTextureObject,
                            qint32,
                            qint32) override
    {
    }
    void setRenderTarget(QDemonRenderBackendRenderTargetObject) override {}
    bool renderTargetIsValid(QDemonRenderBackendRenderTargetObject) override { return false; }
    void setReadTarget(QDemonRenderBackendRenderTargetObject) override {}
    void setDrawBuffers(QDemonRenderBackendRenderTargetObject, QDemonConstDataRef<qint32>) override {}
    void setReadBuffer(QDemonRenderBackendRenderTargetObject, QDemonReadFaces::Enum) override {}

    void blitFramebuffer(qint32, qint32, qint32, qint32, qint32, qint32, qint32, qint32, QDemonRenderClearFlags, QDemonRenderTextureMagnifyingOp::Enum) override
    {
    }
    QDemonRenderBackendRenderbufferObject createRenderbuffer(QDemonRenderRenderBufferFormats::Enum, size_t, size_t) override
    {
        return QDemonRenderBackendRenderbufferObject(1);
    }
    void releaseRenderbuffer(QDemonRenderBackendRenderbufferObject) override {}

    bool resizeRenderbuffer(QDemonRenderBackendRenderbufferObject, QDemonRenderRenderBufferFormats::Enum, size_t, size_t) override
    {
        return false;
    }
    QDemonRenderBackendTextureObject createTexture() override { return QDemonRenderBackendTextureObject(1); }

    void setTextureData2D(QDemonRenderBackendTextureObject,
                          QDemonRenderTextureTargetType::Enum,
                          quint32,
                          QDemonRenderTextureFormats::Enum,
                          size_t,
                          size_t,
                          qint32,
                          QDemonRenderTextureFormats::Enum,
                          const void *) override
    {
    }
    void setTextureDataCubeFace(QDemonRenderBackendTextureObject,
                                QDemonRenderTextureTargetType::Enum,
                                quint32,
                                QDemonRenderTextureFormats::Enum,
                                size_t,
                                size_t,
                                qint32,
                                QDemonRenderTextureFormats::Enum,
                                const void *) override
    {
    }
    void createTextureStorage2D(QDemonRenderBackendTextureObject,
                                QDemonRenderTextureTargetType::Enum,
                                quint32,
                                QDemonRenderTextureFormats::Enum,
                                size_t,
                                size_t) override
    {
    }
    void setTextureSubData2D(QDemonRenderBackendTextureObject,
                             QDemonRenderTextureTargetType::Enum,
                             quint32,
                             qint32,
                             qint32,
                             size_t,
                             size_t,
                             QDemonRenderTextureFormats::Enum,
                             const void *) override
    {
    }
    void setCompressedTextureData2D(QDemonRenderBackendTextureObject,
                                    QDemonRenderTextureTargetType::Enum,
                                    quint32,
                                    QDemonRenderTextureFormats::Enum,
                                    size_t,
                                    size_t,
                                    qint32,
                                    size_t,
                                    const void *) override
    {
    }
    void setCompressedTextureDataCubeFace(QDemonRenderBackendTextureObject,
                                          QDemonRenderTextureTargetType::Enum,
                                          quint32,
                                          QDemonRenderTextureFormats::Enum,
                                          size_t,
                                          size_t,
                                          qint32,
                                          size_t,
                                          const void *) override
    {
    }
    void setCompressedTextureSubData2D(QDemonRenderBackendTextureObject,
                                       QDemonRenderTextureTargetType::Enum,
                                       quint32,
                                       qint32,
                                       qint32,
                                       size_t,
                                       size_t,
                                       QDemonRenderTextureFormats::Enum,
                                       size_t,
                                       const void *) override
    {
    }
    void setMultisampledTextureData2D(QDemonRenderBackendTextureObject,
                                      QDemonRenderTextureTargetType::Enum,
                                      size_t,
                                      QDemonRenderTextureFormats::Enum,
                                      size_t,
                                      size_t,
                                      bool) override
    {
    }
    void setTextureData3D(QDemonRenderBackendTextureObject,
                          QDemonRenderTextureTargetType::Enum,
                          quint32,
                          QDemonRenderTextureFormats::Enum,
                          size_t,
                          size_t,
                          size_t,
                          qint32,
                          QDemonRenderTextureFormats::Enum,
                          const void *) override
    {
    }
    void generateMipMaps(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum, QDemonRenderHint::Enum) override
    {
    }
    void bindTexture(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum, quint32) override {}
    void bindImageTexture(QDemonRenderBackendTextureObject, quint32, qint32, bool, qint32, QDemonRenderImageAccessType::Enum, QDemonRenderTextureFormats::Enum) override
    {
    }
    void releaseTexture(QDemonRenderBackendTextureObject) override {}

    QDemonRenderTextureSwizzleMode::Enum getTextureSwizzleMode(const QDemonRenderTextureFormats::Enum) const override
    {
        return QDemonRenderTextureSwizzleMode::NoSwizzle;
    }

    QDemonRenderBackendSamplerObject createSampler(QDemonRenderTextureMinifyingOp::Enum,
                                                   QDemonRenderTextureMagnifyingOp::Enum,
                                                   QDemonRenderTextureCoordOp::Enum,
                                                   QDemonRenderTextureCoordOp::Enum,
                                                   QDemonRenderTextureCoordOp::Enum,
                                                   qint32,
                                                   qint32,
                                                   float,
                                                   QDemonRenderTextureCompareMode::Enum,
                                                   QDemonRenderTextureCompareOp::Enum,
                                                   float,
                                                   float *) override
    {
        return QDemonRenderBackendSamplerObject(1);
    }

    void updateSampler(QDemonRenderBackendSamplerObject,
                       QDemonRenderTextureTargetType::Enum,
                       QDemonRenderTextureMinifyingOp::Enum,
                       QDemonRenderTextureMagnifyingOp::Enum,
                       QDemonRenderTextureCoordOp::Enum,
                       QDemonRenderTextureCoordOp::Enum,
                       QDemonRenderTextureCoordOp::Enum,
                       float,
                       float,
                       float,
                       QDemonRenderTextureCompareMode::Enum,
                       QDemonRenderTextureCompareOp::Enum,
                       float,
                       float *) override
    {
    }

    void updateTextureObject(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum, qint32, qint32) override
    {
    }

    void updateTextureSwizzle(QDemonRenderBackendTextureObject, QDemonRenderTextureTargetType::Enum, QDemonRenderTextureSwizzleMode::Enum) override
    {
    }

    void releaseSampler(QDemonRenderBackendSamplerObject) override {}

    QDemonRenderBackendAttribLayoutObject createAttribLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry>) override
    {
        return QDemonRenderBackendAttribLayoutObject(1);
    }

    void releaseAttribLayout(QDemonRenderBackendAttribLayoutObject) override {}

    QDemonRenderBackendInputAssemblerObject createInputAssembler(QDemonRenderBackendAttribLayoutObject,
                                                                 QDemonConstDataRef<QDemonRenderBackendBufferObject>,
                                                                 const QDemonRenderBackendBufferObject,
                                                                 QDemonConstDataRef<quint32>,
                                                                 QDemonConstDataRef<quint32>,
                                                                 quint32) override
    {
        return QDemonRenderBackendInputAssemblerObject(1);
    }
    void releaseInputAssembler(QDemonRenderBackendInputAssemblerObject) override {}
    bool setInputAssembler(QDemonRenderBackendInputAssemblerObject, QDemonRenderBackendShaderProgramObject) override
    {
        return false;
    }
    void setPatchVertexCount(QDemonRenderBackendInputAssemblerObject, quint32) override {}
    QDemonRenderBackendVertexShaderObject createVertexShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendVertexShaderObject(1);
    }
    void releaseVertexShader(QDemonRenderBackendVertexShaderObject) override {}
    QDemonRenderBackendFragmentShaderObject createFragmentShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendFragmentShaderObject(1);
    }
    void releaseFragmentShader(QDemonRenderBackendFragmentShaderObject) override {}
    QDemonRenderBackendTessControlShaderObject createTessControlShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendTessControlShaderObject(1);
    }
    void releaseTessControlShader(QDemonRenderBackendTessControlShaderObject) override {}
    QDemonRenderBackendTessEvaluationShaderObject createTessEvaluationShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendTessEvaluationShaderObject(1);
    }
    void releaseTessEvaluationShader(QDemonRenderBackendTessEvaluationShaderObject) override {}
    QDemonRenderBackendGeometryShaderObject createGeometryShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendGeometryShaderObject(1);
    }
    void releaseGeometryShader(QDemonRenderBackendGeometryShaderObject) override {}
    QDemonRenderBackendComputeShaderObject createComputeShader(QDemonConstDataRef<qint8>, QByteArray &, bool) override
    {
        return QDemonRenderBackendComputeShaderObject(1);
    }
    void releaseComputeShader(QDemonRenderBackendComputeShaderObject) override {}
    void attachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendVertexShaderObject) override {}
    void attachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendFragmentShaderObject) override {}
    void attachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendTessControlShaderObject) override {}
    void attachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendTessEvaluationShaderObject) override {}
    void attachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendGeometryShaderObject) override {}
    void attachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendComputeShaderObject) override {}
    void detachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendVertexShaderObject) override {}
    void detachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendFragmentShaderObject) override {}
    void detachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendTessControlShaderObject) override {}
    void detachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendTessEvaluationShaderObject) override {}
    void detachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendGeometryShaderObject) override {}
    void detachShader(QDemonRenderBackendShaderProgramObject, QDemonRenderBackendComputeShaderObject) override {}
    QDemonRenderBackendShaderProgramObject createShaderProgram(bool) override
    {
        return QDemonRenderBackendShaderProgramObject(1);
    }
    void releaseShaderProgram(QDemonRenderBackendShaderProgramObject) override {}
    QDemonRenderBackendProgramPipeline createProgramPipeline() override
    {
        return QDemonRenderBackendProgramPipeline(1);
    }
    void releaseProgramPipeline(QDemonRenderBackendProgramPipeline) override {}

    bool linkProgram(QDemonRenderBackendShaderProgramObject, QByteArray &) override { return false; }
    void setActiveProgram(QDemonRenderBackendShaderProgramObject) override {}
    void setActiveProgramPipeline(QDemonRenderBackendProgramPipeline) override {}
    void setProgramStages(QDemonRenderBackendProgramPipeline, QDemonRenderShaderTypeFlags, QDemonRenderBackendShaderProgramObject) override
    {
    }
    void dispatchCompute(QDemonRenderBackendShaderProgramObject, quint32, quint32, quint32) override {}
    qint32 getConstantCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 getConstantBufferCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 getConstantInfoByID(QDemonRenderBackendShaderProgramObject,
                               quint32,
                               quint32,
                               qint32 *,
                               QDemonRenderShaderDataTypes::Enum *,
                               qint32 *,
                               char *) override
    {
        return 0;
    }

    qint32 getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32, qint32 *, qint32 *, qint32 *, char *) override
    {
        return 0;
    }

    void getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject, quint32, qint32 *) override {}
    void getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject, quint32, quint32 *, qint32 *, qint32 *, qint32 *) override
    {
    }
    void programSetConstantBlock(QDemonRenderBackendShaderProgramObject, quint32, quint32) override {}
    void programSetConstantBuffer(quint32, QDemonRenderBackendBufferObject) override {}

    qint32 getStorageBufferCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 getStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32, qint32 *, qint32 *, qint32 *, char *) override
    {
        return -1;
    }
    void programSetStorageBuffer(quint32, QDemonRenderBackendBufferObject) override {}

    qint32 getAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject) override { return 0; }
    qint32 getAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject, quint32, quint32, qint32 *, qint32 *, qint32 *, char *) override
    {
        return -1;
    }
    void programSetAtomicCounterBuffer(quint32, QDemonRenderBackendBufferObject) override {}

    void setConstantValue(QDemonRenderBackendShaderProgramObject, quint32, QDemonRenderShaderDataTypes::Enum, qint32, const void *, bool) override
    {
    }

    void draw(QDemonRenderDrawMode::Enum, quint32, quint32) override {}
    void drawIndirect(QDemonRenderDrawMode::Enum, const void *) override {}

    void drawIndexed(QDemonRenderDrawMode::Enum, quint32, QDemonRenderComponentTypes::Enum, const void *) override {}
    void drawIndexedIndirect(QDemonRenderDrawMode::Enum, QDemonRenderComponentTypes::Enum, const void *) override {}

    void readPixel(QDemonRenderBackendRenderTargetObject, qint32, qint32, qint32, qint32, QDemonRenderReadPixelFormats::Enum, void *) override
    {
    }

    QDemonRenderBackendPathObject createPathNVObject(size_t) override { return QDemonRenderBackendPathObject(1); }
    void setPathSpecification(QDemonRenderBackendPathObject, QDemonConstDataRef<quint8>, QDemonConstDataRef<float>) override
    {
    }

    ///< Bounds of the fill and stroke
    QDemonBounds3 getPathObjectBoundingBox(QDemonRenderBackendPathObject /*inPathObject*/) override
    {
        return QDemonBounds3();
    }
    QDemonBounds3 getPathObjectFillBox(QDemonRenderBackendPathObject /*inPathObject*/) override
    {
        return QDemonBounds3();
    }
    QDemonBounds3 getPathObjectStrokeBox(QDemonRenderBackendPathObject /*inPathObject*/) override
    {
        return QDemonBounds3();
    }

    /**
     *	Defaults to 0 if unset.
     */
    void setStrokeWidth(QDemonRenderBackendPathObject /*inPathObject*/, float) override {}
    void setPathProjectionMatrix(const QMatrix4x4 /*inPathProjection*/) override {}
    void setPathModelViewMatrix(const QMatrix4x4 /*inPathModelview*/) override {}

    void setPathStencilDepthOffset(float /*inSlope*/, float /*inBias*/) override {}
    void setPathCoverDepthFunc(QDemonRenderBoolOp::Enum /*inDepthFunction*/) override {}
    void stencilStrokePath(QDemonRenderBackendPathObject /*inPathObject*/) override {}
    void stencilFillPath(QDemonRenderBackendPathObject /*inPathObject*/) override {}
    void releasePathNVObject(QDemonRenderBackendPathObject, size_t) override {}

    void loadPathGlyphs(QDemonRenderBackendPathObject,
                        QDemonRenderPathFontTarget::Enum,
                        const void *,
                        QDemonRenderPathFontStyleFlags,
                        size_t,
                        QDemonRenderPathFormatType::Enum,
                        const void *,
                        QDemonRenderPathMissingGlyphs::Enum,
                        QDemonRenderBackendPathObject,
                        float) override
    {
    }
    QDemonRenderPathReturnValues::Enum loadPathGlyphsIndexed(QDemonRenderBackendPathObject,
                                                             QDemonRenderPathFontTarget::Enum,
                                                             const void *,
                                                             QDemonRenderPathFontStyleFlags,
                                                             quint32,
                                                             size_t,
                                                             QDemonRenderBackendPathObject,
                                                             float) override
    {
        return QDemonRenderPathReturnValues::FontUnavailable;
    }
    QDemonRenderBackendPathObject loadPathGlyphsIndexedRange(QDemonRenderPathFontTarget::Enum,
                                                             const void *,
                                                             QDemonRenderPathFontStyleFlags,
                                                             QDemonRenderBackendPathObject,
                                                             float,
                                                             quint32 *) override
    {
        return QDemonRenderBackendPathObject(1);
    }
    void loadPathGlyphRange(QDemonRenderBackendPathObject,
                            QDemonRenderPathFontTarget::Enum,
                            const void *,
                            QDemonRenderPathFontStyleFlags,
                            quint32,
                            size_t,
                            QDemonRenderPathMissingGlyphs::Enum,
                            QDemonRenderBackendPathObject,
                            float) override
    {
    }
    void getPathMetrics(QDemonRenderBackendPathObject,
                        size_t,
                        QDemonRenderPathGlyphFontMetricFlags,
                        QDemonRenderPathFormatType::Enum,
                        const void *,
                        size_t,
                        float *) override
    {
    }
    void getPathMetricsRange(QDemonRenderBackendPathObject, size_t, QDemonRenderPathGlyphFontMetricFlags, size_t, float *) override
    {
    }
    void getPathSpacing(QDemonRenderBackendPathObject,
                        size_t,
                        QDemonRenderPathListMode::Enum,
                        QDemonRenderPathFormatType::Enum,
                        const void *,
                        float,
                        float,
                        QDemonRenderPathTransformType::Enum,
                        float *) override
    {
    }

    void stencilFillPathInstanced(QDemonRenderBackendPathObject,
                                  size_t,
                                  QDemonRenderPathFormatType::Enum,
                                  const void *,
                                  QDemonRenderPathFillMode::Enum,
                                  quint32,
                                  QDemonRenderPathTransformType::Enum,
                                  const float *) override
    {
    }
    void stencilStrokePathInstancedN(QDemonRenderBackendPathObject,
                                     size_t,
                                     QDemonRenderPathFormatType::Enum,
                                     const void *,
                                     qint32,
                                     quint32,
                                     QDemonRenderPathTransformType::Enum,
                                     const float *) override
    {
    }
    void coverFillPathInstanced(QDemonRenderBackendPathObject,
                                size_t,
                                QDemonRenderPathFormatType::Enum,
                                const void *,
                                QDemonRenderPathCoverMode::Enum,
                                QDemonRenderPathTransformType::Enum,
                                const float *) override
    {
    }
    void coverStrokePathInstanced(QDemonRenderBackendPathObject,
                                  size_t,
                                  QDemonRenderPathFormatType::Enum,
                                  const void *,
                                  QDemonRenderPathCoverMode::Enum,
                                  QDemonRenderPathTransformType::Enum,
                                  const float *) override
    {
    }
    QSurfaceFormat format() const override { return QSurfaceFormat(); }
};
}

QDemonRef<QDemonRenderBackend> QDemonRenderBackendNULL::createBackend()
{
    return QDemonRef<QDemonRenderBackend>(new QDemonNullBackend());
}

QT_END_NAMESPACE
