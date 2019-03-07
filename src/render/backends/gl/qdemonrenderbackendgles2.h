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
    qint32 getDepthBits() const override;
    qint32 getStencilBits() const override;
    void generateMipMaps(QDemonRenderBackendTextureObject to,
                         QDemonRenderTextureTargetType target,
                         QDemonRenderHint genType) override;

    void setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType target,
                                      qint32 samples,
                                      QDemonRenderTextureFormat internalFormat,
                                      qint32 width,
                                      qint32 height,
                                      bool fixedsamplelocations) override;

    void setTextureData3D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType target,
                          qint32 level,
                          QDemonRenderTextureFormat internalFormat,
                          qint32 width,
                          qint32 height,
                          qint32 depth,
                          qint32 border,
                          QDemonRenderTextureFormat format,
                          const void *hostPtr = nullptr) override;

    void setTextureData2D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType target,
                          qint32 level,
                          QDemonRenderTextureFormat internalFormat,
                          qint32 width,
                          qint32 height,
                          qint32 border,
                          QDemonRenderTextureFormat format,
                          const void *hostPtr = nullptr) override;

    void updateSampler(QDemonRenderBackendSamplerObject so,
                       QDemonRenderTextureTargetType target,
                       QDemonRenderTextureMinifyingOp minFilter = QDemonRenderTextureMinifyingOp::Linear,
                       QDemonRenderTextureMagnifyingOp magFilter = QDemonRenderTextureMagnifyingOp::Linear,
                       QDemonRenderTextureCoordOp wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
                       QDemonRenderTextureCoordOp wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
                       QDemonRenderTextureCoordOp wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
                       float minLod = -1000.0f,
                       float maxLod = 1000.0f,
                       float lodBias = 0.0f,
                       QDemonRenderTextureCompareMode compareMode = QDemonRenderTextureCompareMode::NoCompare,
                       QDemonRenderTextureCompareOp compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
                       float anisotropy = 1.0f,
                       float *borderColor = nullptr) override;

    void updateTextureObject(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType target,
                             qint32 baseLevel,
                             qint32 maxLevel) override;

    void updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                              QDemonRenderTextureTargetType target,
                              QDemonRenderTextureSwizzleMode swizzleMode) override;

    bool setInputAssembler(QDemonRenderBackendInputAssemblerObject iao, QDemonRenderBackendShaderProgramObject po) override;

    void setDrawBuffers(QDemonRenderBackendRenderTargetObject rto, QDemonConstDataRef<qint32> inDrawBufferSet) override;
    void setReadBuffer(QDemonRenderBackendRenderTargetObject rto, QDemonReadFace inReadFace) override;

    void blitFramebuffer(qint32 srcX0,
                         qint32 srcY0,
                         qint32 srcX1,
                         qint32 srcY1,
                         qint32 dstX0,
                         qint32 dstY0,
                         qint32 dstX1,
                         qint32 dstY1,
                         QDemonRenderClearFlags flags,
                         QDemonRenderTextureMagnifyingOp filter) override;

    QDemonRenderBackendRenderTargetObject createRenderTarget() override;
    void releaseRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachment attachment,
                            QDemonRenderBackendRenderbufferObject rbo) override;
    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachment attachment,
                            QDemonRenderBackendTextureObject to,
                            QDemonRenderTextureTargetType target = QDemonRenderTextureTargetType::Texture2D) override;
    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachment attachment,
                            QDemonRenderBackendTextureObject to,
                            qint32 level,
                            qint32 layer) override;
    void setRenderTarget(QDemonRenderBackendRenderTargetObject rto) override;
    bool renderTargetIsValid(QDemonRenderBackendRenderTargetObject rto) override;

    QDemonRenderBackendRenderbufferObject createRenderbuffer(QDemonRenderRenderBufferFormat storageFormat,
                                                             qint32 width,
                                                             qint32 height) override;
    void setReadTarget(QDemonRenderBackendRenderTargetObject rto) override;
    void releaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo) override;
    bool resizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                            QDemonRenderRenderBufferFormat storageFormat,
                            qint32 width,
                            qint32 height) override;

    void *mapBuffer(QDemonRenderBackendBufferObject bo,
                    QDemonRenderBufferBindFlags bindFlags,
                    size_t offset,
                    size_t length,
                    QDemonRenderBufferAccessFlags accessFlags) override;
    bool unmapBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags) override;

    qint32 getConstantBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                     quint32 id,
                                     quint32 nameBufSize,
                                     qint32 *paramCount,
                                     qint32 *bufferSize,
                                     qint32 *length,
                                     char *nameBuf) override;
    void getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id, qint32 *indices) override;
    void getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                             quint32 count,
                                             quint32 *indices,
                                             QDemonRenderShaderDataType *type,
                                             qint32 *size,
                                             qint32 *offset) override;
    void programSetConstantBlock(QDemonRenderBackendShaderProgramObject po, quint32 blockIndex, quint32 binding) override;
    void programSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    QDemonRenderBackendQueryObject createQuery() override;
    void releaseQuery(QDemonRenderBackendQueryObject qo) override;
    void beginQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType type) override;
    void endQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType type) override;
    void getQueryResult(QDemonRenderBackendQueryObject qo, QDemonRenderQueryResultType resultType, quint32 *params) override;
    void getQueryResult(QDemonRenderBackendQueryObject qo, QDemonRenderQueryResultType resultType, quint64 *params) override;
    void setQueryTimer(QDemonRenderBackendQueryObject qo) override;

    QDemonRenderBackendSyncObject createSync(QDemonRenderSyncType tpye, QDemonRenderSyncFlags syncFlags) override;
    void releaseSync(QDemonRenderBackendSyncObject so) override;
    void waitSync(QDemonRenderBackendSyncObject so, QDemonRenderCommandFlushFlags syncFlags, quint64 timeout) override;

protected:
    QDemonRenderBackendMiscStateGL *m_pCurrentMiscState; ///< this holds the current misc state
#if defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2_ANGLE)
    QDemonOpenGLES2Extensions *m_qdemonExtensions;
#endif
};

QT_END_NAMESPACE

#endif
