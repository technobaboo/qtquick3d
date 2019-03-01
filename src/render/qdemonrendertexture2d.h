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

#ifndef QDEMON_RENDER_RENDER_TEXTURE_2D_H
#define QDEMON_RENDER_RENDER_TEXTURE_2D_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRender/qdemonrenderbackend.h>
#include <QtDemonRender/QDemonRenderTextureBase>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderTextureSampler;

class Q_DEMONRENDER_EXPORT QDemonRenderTexture2D : public QDemonRenderTextureBase, public QDemonRenderImplemented
{

private:
    quint32 m_width; ///< texture width
    quint32 m_height; ///< texture height

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
    QDemonRenderTexture2D(const QDemonRef<QDemonRenderContextImpl> &context,
                          QDemonRenderTextureTargetType::Enum texTarget = QDemonRenderTextureTargetType::Texture2D);

    virtual ~QDemonRenderTexture2D() override;

    // Get the texture details for mipmap level 0 if it was set.
    QDemonTextureDetails getTextureDetails() const override;

    /**
         * @brief Create GL texture object and upload data
         *
         * @param[in] newBuffer			Texture data for level 0
         * @param[in] inMipLevel		Texture level count
         * @param[in] width				Texture width
         * @param[in] height			Texture height
         * @param[in] format			Texture data format
         * @param[in] formaInternal		Texture internal format
         *
         * @return No return.
         */
    virtual void setTextureData(QDemonDataRef<quint8> newBuffer,
                                quint8 inMipLevel,
                                quint32 width,
                                quint32 height,
                                QDemonRenderTextureFormats::Enum format,
                                QDemonRenderTextureFormats::Enum formaInternal = QDemonRenderTextureFormats::Unknown);

    /**
         * @brief Create memory storage for a texture object
         *		  This create a texture storage which is immutable in size and format
         *		  Use this for textures used within compute shaders
         *
         * @param[in] inLevels			Texture level count
         * @param[in] width				Texture width
         * @param[in] height			Texture height
         * @param[in] formaInternal		Texture internal format
         * @param[in] format			Texture data format of dataBuffer
         * @param[in] dataBuffer		Texture data for level 0
         *
         * @return No return.
         */
    virtual void setTextureStorage(quint32 inLevels,
                                   quint32 width,
                                   quint32 height,
                                   QDemonRenderTextureFormats::Enum formaInternal,
                                   QDemonRenderTextureFormats::Enum format = QDemonRenderTextureFormats::Unknown,
                                   QDemonDataRef<quint8> dataBuffer = QDemonDataRef<quint8>());

    virtual void setTextureDataMultisample(quint32 sampleCount,
                                           quint32 width,
                                           quint32 height,
                                           QDemonRenderTextureFormats::Enum format);

    bool isMultisampleTexture() const override
    {
        return (m_texTarget == QDemonRenderTextureTargetType::Texture2D_MS);
    }
    quint32 getSampleCount() const override { return m_sampleCount; }
    bool isImmutableTexture() const override { return m_immutable; }

    // Update a sub-rect of the image.  newBuffer is expected to be a continguous subrect of the
    // image.
    virtual void setTextureSubData(QDemonDataRef<quint8> newBuffer, quint8 inMipLevel, quint32 inXOffset,
                                   quint32 inYOffset, quint32 inSubImageWidth,
                                   quint32 inSubImageHeight, QDemonRenderTextureFormats::Enum format);
    // Generate a set of mipmaps from mipLevel( 0 ).  Uses the graphis layer to do this if
    // possible
    // glGenerateMipmap
    virtual void generateMipmaps(QDemonRenderHint::Enum genType = QDemonRenderHint::Nicest);

    /**
         * @brief Bind a texture for shader access
         *
         *
         * @return No return.
         */
    void bind() override;

    quint32 getNumMipmaps() override { return m_maxMipLevel; }

    /**
         * @brief Query if texture needs coordinate swizzle
         *
         * @return texture swizzle mode
         */
    QDemonRenderTextureSwizzleMode::Enum getTextureSwizzleMode() override
    {
        // if our backend supports hardware texture swizzle then there is no need for a shader
        // swizzle
        return (m_backend->getRenderBackendCap(
                    QDemonRenderBackend::QDemonRenderBackendCaps::TexSwizzle))
                ? QDemonRenderTextureSwizzleMode::NoSwizzle
                : m_backend->getTextureSwizzleMode(m_format);
    }

    // this will be obsolete
    const void *getImplementationHandle() const override
    {
        return reinterpret_cast<void *>(m_textureHandle);
    }

    static QDemonRef<QDemonRenderTexture2D> create(const QDemonRef<QDemonRenderContextImpl> &context);
};

QT_END_NAMESPACE

#endif
