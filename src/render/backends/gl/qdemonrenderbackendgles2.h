/****************************************************************************
**
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

#ifndef QDEMON_RENDER_BACKEND_GLES2_H
#define QDEMON_RENDER_BACKEND_GLES2_H

#include <QtDemonRender/qdemonrenderbackendglbase.h>
#include <QtDemonRender/qdemonopenglextensions.h>

#include <QtGui/qopenglextrafunctions.h>
#include <QtOpenGLExtensions/QtOpenGLExtensions>

QT_BEGIN_NAMESPACE

///< forward declaration
class QDemonRenderBackendMiscStateGL;

class QDemonRenderBackendGLES2Impl : public QDemonRenderBackendGLBase
{
public:
    /// constructor
    QDemonRenderBackendGLES2Impl(const QSurfaceFormat &format);
    /// destructor
    virtual ~QDemonRenderBackendGLES2Impl();
    public:
        quint32 GetDepthBits() const override;
    quint32 GetStencilBits() const override;
    void GenerateMipMaps(QDemonRenderBackendTextureObject to,
                         QDemonRenderTextureTargetType::Enum target,
                         QDemonRenderHint::Enum genType) override;

    void SetMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType::Enum target,
                                      size_t samples,
                                      QDemonRenderTextureFormats::Enum internalFormat,
                                      size_t width, size_t height,
                                      bool fixedsamplelocations) override;

    void SetTextureData3D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target, quint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat, size_t width,
                          size_t height, size_t depth, qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;

    void SetTextureData2D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target, quint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat, size_t width,
                          size_t height, qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;

    void UpdateSampler(
            QDemonRenderBackendSamplerObject so, QDemonRenderTextureTargetType::Enum target,
            QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
            QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
            QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
            float minLod = -1000.0f, float maxLod = 1000.0f, float lodBias = 0.0f,
            QDemonRenderTextureCompareMode::Enum compareMode = QDemonRenderTextureCompareMode::NoCompare,
            QDemonRenderTextureCompareOp::Enum compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
            float anisotropy = 1.0f, float *borderColor = nullptr) override;

    void UpdateTextureObject(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType::Enum target, qint32 baseLevel,
                             qint32 maxLevel) override;

    void UpdateTextureSwizzle(QDemonRenderBackendTextureObject to,
                              QDemonRenderTextureTargetType::Enum target,
                              QDemonRenderTextureSwizzleMode::Enum swizzleMode) override;

    bool SetInputAssembler(QDemonRenderBackendInputAssemblerObject iao,
                           QDemonRenderBackendShaderProgramObject po) override;

    void SetDrawBuffers(QDemonRenderBackendRenderTargetObject rto,
                        QDemonConstDataRef<qint32> inDrawBufferSet) override;
    void SetReadBuffer(QDemonRenderBackendRenderTargetObject rto,
                       QDemonReadFaces::Enum inReadFace) override;

    void BlitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1, qint32 srcY1,
                         qint32 dstX0, qint32 dstY0, qint32 dstX1, qint32 dstY1,
                         QDemonRenderClearFlags flags,
                         QDemonRenderTextureMagnifyingOp::Enum filter) override;


    QDemonRenderBackendRenderTargetObject CreateRenderTarget() override;
    void ReleaseRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendRenderbufferObject rbo) override;
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to,
                            QDemonRenderTextureTargetType::Enum target
                            = QDemonRenderTextureTargetType::Texture2D) override;
    void RenderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to, qint32 level,
                            qint32 layer) override;
    void SetRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    bool RenderTargetIsValid(QDemonRenderBackendRenderTargetObject rto) override;

    virtual QDemonRenderBackendRenderbufferObject
    CreateRenderbuffer(QDemonRenderRenderBufferFormats::Enum storageFormat, size_t width,
                       size_t height) override;
    void SetReadTarget(QDemonRenderBackendRenderTargetObject rto) override;
    void ReleaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo) override;
    bool ResizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                            QDemonRenderRenderBufferFormats::Enum storageFormat,
                            size_t width, size_t height) override;

    void *MapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags,
                    size_t offset, size_t length,
                    QDemonRenderBufferAccessFlags accessFlags) override;
    bool UnmapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) override;

    qint32 GetConstantBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                     quint32 nameBufSize, qint32 *paramCount,
                                     qint32 *bufferSize, qint32 *length,
                                     char *nameBuf) override;
    void GetConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                       qint32 *indices) override;
    void GetConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                             quint32 count, quint32 *indices,
                                             qint32 *type,
                                             qint32 *size, qint32 *offset) override;
    void ProgramSetConstantBlock(QDemonRenderBackendShaderProgramObject po,
                                 quint32 blockIndex, quint32 binding) override;
    void ProgramSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

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

protected:
    QDemonRenderBackendMiscStateGL *m_pCurrentMiscState; ///< this holds the current misc state
#if defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2_ANGLE)
    QDemonOpenGLES2Extensions *m_qt3dsExtensions;
#endif
};

QT_END_NAMESPACE

#endif
