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
#ifndef QDEMON_RENDER_BACKEND_GL3_H
#define QDEMON_RENDER_BACKEND_GL3_H

/// @file qdemonrenderbackendgl3.h
///       NVRender OpenGL 3 backend definition.

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackendglbase.h>
#include <QtDemonRender/qdemonopenglextensions.h>

#include <QtGui/QOpenGLExtraFunctions>
#include <QtOpenGLExtensions/QtOpenGLExtensions>

QT_BEGIN_NAMESPACE

///< forward declaration
class QDemonRenderBackendMiscStateGL;

namespace QDemonGlExtStrings {
QByteArray extsAstcHDR();
QByteArray extsAstcLDR();
}

class QDemonRenderBackendGL3Impl : public QDemonRenderBackendGLBase
{
public:
    /// constructor
    QDemonRenderBackendGL3Impl(const QSurfaceFormat &format);
    /// destructor
    ~QDemonRenderBackendGL3Impl() override;

public:
    qint32 getDepthBits() const override;
    qint32 getStencilBits() const override;
    void generateMipMaps(QDemonRenderBackendTextureObject to,
                         QDemonRenderTextureTargetType::Enum target,
                         QDemonRenderHint::Enum genType) override;

    void setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType::Enum target,
                                      qint32 samples,
                                      QDemonRenderTextureFormats::Enum internalFormat,
                                      qint32 width,
                                      qint32 height,
                                      bool fixedsamplelocations) override;

    void setTextureData3D(QDemonRenderBackendTextureObject to,
                          QDemonRenderTextureTargetType::Enum target,
                          qint32 level,
                          QDemonRenderTextureFormats::Enum internalFormat,
                          qint32 width,
                          qint32 height,
                          qint32 depth,
                          qint32 border,
                          QDemonRenderTextureFormats::Enum format,
                          const void *hostPtr = nullptr) override;

    void updateSampler(QDemonRenderBackendSamplerObject so,
                       QDemonRenderTextureTargetType::Enum target,
                       QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
                       QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
                       QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
                       QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
                       QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
                       float minLod = -1000.0,
                       float maxLod = 1000.0,
                       float lodBias = 0.0,
                       QDemonRenderTextureCompareMode::Enum compareMode = QDemonRenderTextureCompareMode::NoCompare,
                       QDemonRenderTextureCompareOp::Enum compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
                       float anisotropy = 1.0,
                       float *borderColor = nullptr) override;

    void updateTextureObject(QDemonRenderBackendTextureObject to,
                             QDemonRenderTextureTargetType::Enum target,
                             qint32 baseLevel,
                             qint32 maxLevel) override;

    void updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                              QDemonRenderTextureTargetType::Enum target,
                              QDemonRenderTextureSwizzleMode::Enum swizzleMode) override;

    bool setInputAssembler(QDemonRenderBackendInputAssemblerObject iao, QDemonRenderBackendShaderProgramObject po) override;

    void setDrawBuffers(QDemonRenderBackendRenderTargetObject rto, QDemonConstDataRef<qint32> inDrawBufferSet) override;
    void setReadBuffer(QDemonRenderBackendRenderTargetObject rto, QDemonReadFaces::Enum inReadFace) override;

    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendRenderbufferObject rbo) override
    {
        QDemonRenderBackendGLBase::renderTargetAttach(rto, attachment, rbo);
    }

    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to,
                            QDemonRenderTextureTargetType::Enum target = QDemonRenderTextureTargetType::Texture2D) override
    {
        QDemonRenderBackendGLBase::renderTargetAttach(rto, attachment, to, target);
    }

    void renderTargetAttach(QDemonRenderBackendRenderTargetObject rto,
                            QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderBackendTextureObject to,
                            qint32 level,
                            qint32 layer) override;
    void setReadTarget(QDemonRenderBackendRenderTargetObject rto) override;

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
                                             qint32 *type,
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
    QDemonRenderBackendMiscStateGL *m_currentMiscState; ///< this holds the current misc state
#if defined(QT_OPENGL_ES_2)
    QDemonOpenGLES2Extensions *m_qdemonExtensions;
#else
    QOpenGLExtension_ARB_timer_query *m_timerExtension;
    QOpenGLExtension_ARB_tessellation_shader *m_tessellationShader;
    QOpenGLExtension_ARB_texture_multisample *m_multiSample;
    QDemonOpenGLExtensions *m_qdemonExtensions;
#endif
};

QT_END_NAMESPACE

#endif
