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
#ifndef QDEMON_RENDER_TEXTURE_BUFFER_H
#define QDEMON_RENDER_TEXTURE_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderTextureSampler;

struct QDemonTextureDetails
{
    qint32 width = 0;
    qint32 height = 0;
    qint32 depth = 0;
    qint32 sampleCount = 1;
    QDemonRenderTextureFormat format = QDemonRenderTextureFormat::Unknown;

    QDemonTextureDetails(qint32 w, qint32 h, qint32 d, qint32 samples, QDemonRenderTextureFormat f)
        : width(w), height(h), depth(d), sampleCount(samples), format(f)
    {
    }
    QDemonTextureDetails() = default;
};

class QDemonRenderTextureBase
{
public:
    QAtomicInt ref;

protected:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendTextureObject m_textureHandle; ///< opaque backend handle
    qint32 m_textureUnit; ///< texture unit this texture should use
    bool m_samplerParamsDirty; ///< true if sampler state is dirty
    bool m_texStateDirty; ///< true if texture object state is dirty
    qint32 m_sampleCount; ///< texture height
    QDemonRenderTextureFormat m_format; ///< texture format
    QDemonRenderTextureTargetType m_texTarget; ///< texture target
    QDemonRenderTextureSampler *m_sampler; ///< current texture sampler state
    qint32 m_baseLevel; ///< minimum lod specified
    qint32 m_maxLevel; ///< maximum lod specified
    qint32 m_maxMipLevel; ///< highest mip level
    bool m_immutable; ///< true if this is a immutable texture ( size and format )

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] fnd			Pointer to foundation
     * @param[in] texTarget		Texture target
     *
     * @return No return.
     */
    QDemonRenderTextureBase(const QDemonRef<QDemonRenderContextImpl> &context, QDemonRenderTextureTargetType texTarget);

    virtual ~QDemonRenderTextureBase();

    virtual void setMinFilter(QDemonRenderTextureMinifyingOp value);
    virtual void setMagFilter(QDemonRenderTextureMagnifyingOp value);

    virtual void setBaseLevel(qint32 value);
    virtual void setMaxLevel(qint32 value);

    virtual void setTextureWrapS(QDemonRenderTextureCoordOp value);
    virtual void setTextureWrapT(QDemonRenderTextureCoordOp value);

    virtual void setTextureCompareMode(QDemonRenderTextureCompareMode value);
    virtual void setTextureCompareFunc(QDemonRenderTextureCompareOp value);

    virtual void setTextureUnit(quint32 unit) { m_textureUnit = unit; }
    virtual qint32 getTextureUnit() const { return m_textureUnit; }

    // Get the texture details for mipmap level 0 if it was set.
    virtual QDemonTextureDetails getTextureDetails() const = 0;

    virtual bool isMultisampleTexture() const { return (m_texTarget == QDemonRenderTextureTargetType::Texture2D_MS); }
    virtual qint32 getSampleCount() const { return m_sampleCount; }
    virtual bool isImmutableTexture() const { return m_immutable; }

    /**
     * @brief Bind a texture for shader access
     *
     *
     * @return No return.
     */
    virtual void bind() = 0;

    virtual quint32 getNumMipmaps() { return m_maxMipLevel; }

    /**
     * @brief Query if texture needs coordinate swizzle
     *
     * @return texture swizzle mode
     */
    virtual QDemonRenderTextureSwizzleMode getTextureSwizzleMode()
    {
        // if our backend supports hardware texture swizzle then there is no need for a shader
        // swizzle
        return (m_backend->getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::TexSwizzle))
                ? QDemonRenderTextureSwizzleMode::NoSwizzle
                : m_backend->getTextureSwizzleMode(m_format);
    }

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    virtual QDemonRenderBackend::QDemonRenderBackendTextureObject getTextureObjectHandle() { return m_textureHandle; }

protected:
    void applyTexParams();
    void applyTexSwizzle();
};

QT_END_NAMESPACE

#endif
