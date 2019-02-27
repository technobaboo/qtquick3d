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

namespace QDemonGlExtStrings
{
QByteArray exts3tc();
QByteArray extsdxt();
QByteArray extsAniso();
QByteArray extsTexSwizzle();
QByteArray extsFPRenderTarget();
QByteArray extsTimerQuery();
QByteArray extsGpuShader5();
}

class QDemonRenderBackendGLBase : public QDemonRenderBackend
{
    QDemonRenderBackendGLBase() = default;
public:
    /// constructor
    QDemonRenderBackendGLBase(const QSurfaceFormat &format);
    /// destructor
    ~QDemonRenderBackendGLBase() override;

public:
    /// API Interface
    QDemonRenderContextType getRenderContextType() const override;
    bool isESCompatible() const;

    const char *getShadingLanguageVersion() override;
    /// get implementation depended values
    quint32 getMaxCombinedTextureUnits() override;
    bool getRenderBackendCap(QDemonRenderBackendCaps::Enum inCap) const override;
    quint32 getDepthBits() const override;
    quint32 getStencilBits() const override;
    void getRenderBackendValue(QDemonRenderBackendQuery::Enum inQuery, qint32 *params) const override;

    /// state get/set functions
    void setRenderState(bool bEnable, const QDemonRenderState::Enum value) override;
    bool getRenderState(const QDemonRenderState::Enum value) override;
    virtual QDemonRenderBackendDepthStencilStateObject
    createDepthStencilState(bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc,
                            bool enableStencil,
                            QDemonRenderStencilFunctionArgument &stencilFuncFront,
                            QDemonRenderStencilFunctionArgument &stencilFuncBack,
                            QDemonRenderStencilOperationArgument &depthStencilOpFront,
                            QDemonRenderStencilOperationArgument &depthStencilOpBack) override;
    virtual void
    releaseDepthStencilState(QDemonRenderBackendDepthStencilStateObject inDepthStencilState) override;
    virtual QDemonRenderBackendRasterizerStateObject
    createRasterizerState(float depthBias, float depthScale, QDemonRenderFaces::Enum cullFace) override;
    void releaseRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState) override;
    virtual void
    setDepthStencilState(QDemonRenderBackendDepthStencilStateObject inDepthStencilState) override;
    void setRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState) override;
    QDemonRenderBoolOp::Enum getDepthFunc() override;
    void setDepthFunc(const QDemonRenderBoolOp::Enum func) override;
    bool getDepthWrite() override;
    void setDepthWrite(bool bEnable) override;
    void setColorWrites(bool bRed, bool bGreen, bool bBlue, bool bAlpha) override;
    void setMultisample(bool bEnable) override;
    void getBlendFunc(QDemonRenderBlendFunctionArgument *pBlendFuncArg) override;
    void setBlendFunc(const QDemonRenderBlendFunctionArgument &blendFuncArg) override;
    void setBlendEquation(const QDemonRenderBlendEquationArgument &pBlendEquArg) override;
    void setBlendBarrier(void) override;
    void getScissorRect(QRect *pRect) override;
    void setScissorRect(const QRect &rect) override;
    void getViewportRect(QRect *pRect) override;
    void setViewportRect(const QRect &rect) override;

    void setClearColor(const QVector4D *pClearColor) override;
    void clear(QDemonRenderClearFlags flags) override;

    /// resource handling
    QDemonRenderBackendBufferObject createBuffer(size_t size,
                                                 QDemonRenderBufferBindFlags bindFlags,
                                                 QDemonRenderBufferUsageType::Enum usage,
                                                 const void *hostPtr = nullptr) override;
    void bindBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) override;
    void releaseBuffer(QDemonRenderBackendBufferObject bo) override;
    void updateBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags,
                      size_t size, QDemonRenderBufferUsageType::Enum usage,
                      const void *data) override;
    void updateBufferRange(QDemonRenderBackendBufferObject bo,
                           QDemonRenderBufferBindFlags bindFlags, size_t offset,
                           size_t size, const void *data) override;
    void *mapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags,
                    size_t offset, size_t length,
                    QDemonRenderBufferAccessFlags accessFlags) override;
    bool unmapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) override;
    void setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) override;

    QDemonRenderBackendQueryObject createQuery() override;
    void releaseQuery(QDemonRenderBackendQueryObject qo) override;
    void beginQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType::Enum type) override;
    void endQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType::Enum type) override;
    void getQueryResult(QDemonRenderBackendQueryObject qo,
                        QDemonRenderQueryResultType::Enum resultType, quint32 *params) override;
    void getQueryResult(QDemonRenderBackendQueryObject qo,
                        QDemonRenderQueryResultType::Enum resultType, quint64 *params) override;
    void setQueryTimer(QDemonRenderBackendQueryObject qo) override;

    QDemonRenderBackendSyncObject createSync(QDemonRenderSyncType::Enum tpye,
                                             QDemonRenderSyncFlags syncFlags) override;
    void releaseSync(QDemonRenderBackendSyncObject so) override;
    void waitSync(QDemonRenderBackendSyncObject so, QDemonRenderCommandFlushFlags syncFlags,
                  quint64 timeout) override;

    QDemonRenderBackendRenderTargetObject createRenderTarget() override;
    void releaseRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;

    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendRenderbufferObject rbo) override;

    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to,
                            QDemonRenderTextureTargetType::Enum target = QDemonRenderTextureTargetType::Texture2D) override;

    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to,
                            qint32 level,
                            qint32 layer) override;

    void setRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    bool renderTargetIsValid(QDemonRenderBackendRenderTargetObject rto) override;

    virtual QDemonRenderBackendRenderbufferObject
    createRenderbuffer(QDemonRenderRenderBufferFormats::Enum storageFormat, size_t width,
                       size_t height) override;
    void releaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo) override;
    bool resizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                            QDemonRenderRenderBufferFormats::Enum storageFormat,
                            size_t width, size_t height) override;

    QDemonRenderBackendTextureObject createTexture() override;
    void bindTexture(QDemonRenderBackendTextureObject to,
                     QDemonRenderTextureTargetType::Enum target, quint32 unit) override;
    void bindImageTexture(QDemonRenderBackendTextureObject to, quint32 unit, qint32 level,
                          bool layered, qint32 layer,
                          QDemonRenderImageAccessType::Enum access,
                          QDemonRenderTextureFormats::Enum format) override;
    void releaseTexture(QDemonRenderBackendTextureObject to) override;
    void setTextureData2D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target, quint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat, size_t width,
                          size_t height, qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;
    void setTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                QDemonRenderTextureTargetType::Enum target, quint32 level,
                                QDemonRenderTextureFormats::Enum internalFormat,
                                size_t width, size_t height, qint32 border,
                                QDemonRenderTextureFormats::Enum format,
                                const void *hostPtr = nullptr) override;
    void createTextureStorage2D(QDemonRenderBackendTextureObject to,
                                QDemonRenderTextureTargetType::Enum target, quint32 levels,
                                QDemonRenderTextureFormats::Enum internalFormat,
                                size_t width, size_t height) override;
    void setTextureSubData2D(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType::Enum target, quint32 level,
                             qint32 xOffset, qint32 yOffset, size_t width, size_t height,
                             QDemonRenderTextureFormats::Enum format,
                             const void *hostPtr = nullptr) override;
    void setCompressedTextureData2D(QDemonRenderBackendTextureObject to,
                                    QDemonRenderTextureTargetType::Enum target, quint32 level,
                                    QDemonRenderTextureFormats::Enum internalFormat,
                                    size_t width, size_t height, qint32 border,
                                    size_t imageSize, const void *hostPtr = nullptr) override;
    void setCompressedTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                          QDemonRenderTextureTargetType::Enum target,
                                          quint32 level,
                                          QDemonRenderTextureFormats::Enum internalFormat,
                                          size_t width, size_t height, qint32 border,
                                          size_t imageSize, const void *hostPtr = nullptr) override;
    void setCompressedTextureSubData2D(QDemonRenderBackendTextureObject to,
                                       QDemonRenderTextureTargetType::Enum target,
                                       quint32 level, qint32 xOffset, qint32 yOffset,
                                       size_t width, size_t height,
                                       QDemonRenderTextureFormats::Enum format,
                                       size_t imageSize, const void *hostPtr = nullptr) override;
    void setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType::Enum target,
                                      size_t samples,
                                      QDemonRenderTextureFormats::Enum internalFormat,
                                      size_t width, size_t height,
                                      bool fixedsamplelocations) override = 0;

    void setTextureData3D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target, quint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat, size_t width,
                          size_t height, size_t depth, qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;

    void generateMipMaps(QDemonRenderBackendTextureObject to,
                         QDemonRenderTextureTargetType::Enum target,
                         QDemonRenderHint::Enum genType) override;

    virtual QDemonRenderTextureSwizzleMode::Enum
    getTextureSwizzleMode(const QDemonRenderTextureFormats::Enum inFormat) const override;

    QDemonRenderBackendSamplerObject createSampler(
            QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
            QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
            QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
            qint32 minLod = -1000, qint32 maxLod = 1000, float lodBias = 0.0,
            QDemonRenderTextureCompareMode::Enum compareMode = QDemonRenderTextureCompareMode::NoCompare,
            QDemonRenderTextureCompareOp::Enum compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
            float anisotropy = 1.0, float *borderColor = nullptr) override;

    void updateSampler(
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

    void updateTextureObject(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType::Enum target, qint32 baseLevel,
                             qint32 maxLevel) override;

    void updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                              QDemonRenderTextureTargetType::Enum target,
                              QDemonRenderTextureSwizzleMode::Enum swizzleMode) override;

    void releaseSampler(QDemonRenderBackendSamplerObject so) override;

    virtual QDemonRenderBackendAttribLayoutObject
    createAttribLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) override;
    void releaseAttribLayout(QDemonRenderBackendAttribLayoutObject ao) override;

    virtual QDemonRenderBackendInputAssemblerObject
    createInputAssembler(QDemonRenderBackendAttribLayoutObject attribLayout,
                         QDemonConstDataRef<QDemonRenderBackendBufferObject> buffers,
                         const QDemonRenderBackendBufferObject indexBuffer,
                         QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets,
                         quint32 patchVertexCount) override;
    void releaseInputAssembler(QDemonRenderBackendInputAssemblerObject iao) override;

    bool setInputAssembler(QDemonRenderBackendInputAssemblerObject iao,
                           QDemonRenderBackendShaderProgramObject po) override = 0;
    void setPatchVertexCount(QDemonRenderBackendInputAssemblerObject, quint32) override
    {
        Q_ASSERT(false);
    }

    // shader
    virtual QDemonRenderBackendVertexShaderObject
    createVertexShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary) override;
    virtual QDemonRenderBackendFragmentShaderObject
    createFragmentShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary) override;
    virtual QDemonRenderBackendTessControlShaderObject
    createTessControlShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage,
                            bool binary) override;
    virtual QDemonRenderBackendTessEvaluationShaderObject
    createTessEvaluationShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage,
                               bool binary) override;
    virtual QDemonRenderBackendGeometryShaderObject
    createGeometryShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary) override;
    virtual QDemonRenderBackendComputeShaderObject
    createComputeShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary) override;
    void releaseVertexShader(QDemonRenderBackendVertexShaderObject vso) override;
    void releaseFragmentShader(QDemonRenderBackendFragmentShaderObject fso) override;
    void releaseTessControlShader(QDemonRenderBackendTessControlShaderObject tcso) override;
    void releaseTessEvaluationShader(QDemonRenderBackendTessEvaluationShaderObject teso) override;
    void releaseGeometryShader(QDemonRenderBackendGeometryShaderObject gso) override;
    void releaseComputeShader(QDemonRenderBackendComputeShaderObject cso) override;
    void attachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendVertexShaderObject vso) override;
    void attachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendFragmentShaderObject fso) override;
    void attachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessControlShaderObject tcso) override;
    void attachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessEvaluationShaderObject teso) override;
    void attachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendGeometryShaderObject gso) override;
    void attachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendComputeShaderObject cso) override;
    void detachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendVertexShaderObject vso) override;
    void detachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendFragmentShaderObject fso) override;
    void detachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessControlShaderObject tcso) override;
    void detachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendTessEvaluationShaderObject teso) override;
    void detachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendGeometryShaderObject gso) override;
    void detachShader(QDemonRenderBackendShaderProgramObject po,
                      QDemonRenderBackendComputeShaderObject cso) override;
    QDemonRenderBackendShaderProgramObject createShaderProgram(bool isSeparable) override;
    void releaseShaderProgram(QDemonRenderBackendShaderProgramObject po) override;
    bool linkProgram(QDemonRenderBackendShaderProgramObject po,
                     QByteArray &errorMessage) override;
    void setActiveProgram(QDemonRenderBackendShaderProgramObject po) override;
    void dispatchCompute(QDemonRenderBackendShaderProgramObject po, quint32 numGroupsX,
                         quint32 numGroupsY, quint32 numGroupsZ) override;
    QDemonRenderBackendProgramPipeline createProgramPipeline() override;
    void releaseProgramPipeline(QDemonRenderBackendProgramPipeline po) override;
    void setActiveProgramPipeline(QDemonRenderBackendProgramPipeline po) override;
    void setProgramStages(QDemonRenderBackendProgramPipeline ppo,
                          QDemonRenderShaderTypeFlags flags,
                          QDemonRenderBackendShaderProgramObject po) override;

    // uniforms
    qint32 getConstantCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getConstantInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                               quint32 bufSize, qint32 *numElem,
                               QDemonRenderShaderDataTypes::Enum *type, qint32 *binding,
                               char *nameBuf) override;
    void setConstantValue(QDemonRenderBackendShaderProgramObject po, quint32 id,
                          QDemonRenderShaderDataTypes::Enum type, qint32 count,
                          const void *value, bool transpose) override;

    // uniform buffers
    qint32 getConstantBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                     quint32 nameBufSize, qint32 *paramCount,
                                     qint32 *bufferSize, qint32 *length, char *nameBuf) override;
    void getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                       qint32 *indices) override;
    void getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                             quint32 count, quint32 *indices, qint32 *type,
                                             qint32 *size, qint32 *offset) override;
    void programSetConstantBlock(QDemonRenderBackendShaderProgramObject po,
                                 quint32 blockIndex, quint32 binding) override;
    void programSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    // storage buffers
    qint32 getStorageBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                    quint32 nameBufSize, qint32 *paramCount,
                                    qint32 *bufferSize, qint32 *length, char *nameBuf) override;
    void programSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    // atomic counter buffers
    qint32 getAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                          quint32 id, quint32 nameBufSize, qint32 *paramCount,
                                          qint32 *bufferSize, qint32 *length,
                                          char *nameBuf) override;
    void programSetAtomicCounterBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    /// draw calls
    void draw(QDemonRenderDrawMode::Enum drawMode, quint32 start, quint32 count) override;
    void drawIndirect(QDemonRenderDrawMode::Enum drawMode, const void *indirect) override;
    void drawIndexed(QDemonRenderDrawMode::Enum drawMode, quint32 count,
                     QDemonRenderComponentTypes::Enum type, const void *indices) override;
    void drawIndexedIndirect(QDemonRenderDrawMode::Enum drawMode,
                             QDemonRenderComponentTypes::Enum type, const void *indirect) override;

    // read calls
    void readPixel(QDemonRenderBackendRenderTargetObject rto, qint32 x, qint32 y, qint32 width,
                   qint32 height, QDemonRenderReadPixelFormats::Enum inFormat, void *pixels) override;

    // NV path rendering
    QDemonRenderBackendPathObject createPathNVObject(size_t range) override;
    // Pathing requires gl4 backend.
    void setPathSpecification(QDemonRenderBackendPathObject, QDemonConstDataRef<quint8>,
                              QDemonConstDataRef<float>) override
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
    void releasePathNVObject(QDemonRenderBackendPathObject po, size_t range) override;

    void loadPathGlyphs(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum,
                        const void *, QDemonRenderPathFontStyleFlags, size_t,
                        QDemonRenderPathFormatType::Enum, const void *,
                        QDemonRenderPathMissingGlyphs::Enum, QDemonRenderBackendPathObject,
                        float) override;
    virtual QDemonRenderPathReturnValues::Enum
    loadPathGlyphsIndexed(QDemonRenderBackendPathObject po, QDemonRenderPathFontTarget::Enum fontTarget,
                          const void *fontName, QDemonRenderPathFontStyleFlags fontStyle,
                          quint32 firstGlyphIndex, size_t numGlyphs,
                          QDemonRenderBackendPathObject pathParameterTemplate, float emScale) override;
    virtual QDemonRenderBackendPathObject
    loadPathGlyphsIndexedRange(QDemonRenderPathFontTarget::Enum, const void *,
                               QDemonRenderPathFontStyleFlags,
                               QDemonRenderBackend::QDemonRenderBackendPathObject, float, quint32 *) override;
    void loadPathGlyphRange(QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum,
                            const void *, QDemonRenderPathFontStyleFlags, quint32, size_t,
                            QDemonRenderPathMissingGlyphs::Enum, QDemonRenderBackendPathObject,
                            float) override;
    void getPathMetrics(QDemonRenderBackendPathObject, size_t,
                        QDemonRenderPathGlyphFontMetricFlags, QDemonRenderPathFormatType::Enum,
                        const void *, size_t, float *) override;
    void getPathMetricsRange(QDemonRenderBackendPathObject, size_t,
                             QDemonRenderPathGlyphFontMetricFlags, size_t, float *) override;
    void getPathSpacing(QDemonRenderBackendPathObject, size_t, QDemonRenderPathListMode::Enum,
                        QDemonRenderPathFormatType::Enum, const void *, float, float,
                        QDemonRenderPathTransformType::Enum, float *) override;

    void stencilFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                  QDemonRenderPathFormatType::Enum, const void *,
                                  QDemonRenderPathFillMode::Enum, quint32,
                                  QDemonRenderPathTransformType::Enum, const float *) override;
    void stencilStrokePathInstancedN(QDemonRenderBackendPathObject, size_t,
                                     QDemonRenderPathFormatType::Enum, const void *, qint32,
                                     quint32, QDemonRenderPathTransformType::Enum,
                                     const float *) override;
    void coverFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                QDemonRenderPathFormatType::Enum, const void *,
                                QDemonRenderPathCoverMode::Enum,
                                QDemonRenderPathTransformType::Enum, const float *) override;
    void coverStrokePathInstanced(QDemonRenderBackendPathObject, size_t,
                                  QDemonRenderPathFormatType::Enum, const void *,
                                  QDemonRenderPathCoverMode::Enum,
                                  QDemonRenderPathTransformType::Enum, const float *) override;

    QSurfaceFormat format() const override
    {
        return m_format;
    }

protected:
    virtual bool compileSource(GLuint shaderID, QDemonConstDataRef<qint8> source,
                               QByteArray &errorMessage, bool binary);
    virtual const char *getVersionString();
    virtual const char *getVendorString();
    virtual const char *getRendererString();
    virtual const char *getExtensionString();

    virtual void setAndInspectHardwareCaps();

protected:
    GLConversion m_conversion; ///< Class for conversion from base type to GL types
    QList<QByteArray> m_extensions; ///< contains the OpenGL extension string
    qint32 m_maxAttribCount; ///< Maximum attributes which can be used
    QVector<GLenum> m_drawBuffersArray; ///< Contains the drawbuffer enums
    QSurfaceFormat m_format;

    QDemonRenderBackendRasterizerStateGL *m_currentRasterizerState = nullptr; ///< this holds the current rasterizer state
    QDemonRenderBackendDepthStencilStateGL *m_currentDepthStencilState = nullptr; ///< this holds the current depth stencil state

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
    void checkGLError(const char *function, const char *file, const unsigned int line) const;
#else
    void checkGLError() const;
#endif
    QOpenGLFunctions *m_glFunctions = nullptr;
    QOpenGLExtraFunctions *m_glExtraFunctions = nullptr;
};

QT_END_NAMESPACE

#endif
