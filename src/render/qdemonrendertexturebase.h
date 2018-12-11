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
#ifndef QDEMON_RENDER_TEXTURE_BUFFER_H
#define QDEMON_RENDER_TEXTURE_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderTextureSampler;

struct STextureDetails
{
    quint32 m_Width;
    quint32 m_Height;
    quint32 m_Depth;
    quint32 m_SampleCount;
    QDemonRenderTextureFormats::Enum m_Format;

    STextureDetails(quint32 w, quint32 h, quint32 d, quint32 samples, QDemonRenderTextureFormats::Enum f)
        : m_Width(w)
        , m_Height(h)
        , m_Depth(d)
        , m_SampleCount(samples)
        , m_Format(f)
    {
    }
    STextureDetails()
        : m_Width(0)
        , m_Height(0)
        , m_Depth(0)
        , m_SampleCount(1)
        , m_Format(QDemonRenderTextureFormats::Unknown)
    {
    }
};

class QDemonRenderTextureBase
{

protected:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendTextureObject m_TextureHandle; ///< opaque backend handle
    quint32 m_TextureUnit; ///< texture unit this texture should use
    bool m_SamplerParamsDirty; ///< true if sampler state is dirty
    bool m_TexStateDirty; ///< true if texture object state is dirty
    quint32 m_SampleCount; ///< texture height
    QDemonRenderTextureFormats::Enum m_Format; ///< texture format
    QDemonRenderTextureTargetType::Enum m_TexTarget; ///< texture target
    QDemonRenderTextureSampler *m_Sampler; ///< current texture sampler state
    qint32 m_BaseLevel; ///< minimum lod specified
    qint32 m_MaxLevel; ///< maximum lod specified
    quint32 m_MaxMipLevel; ///< highest mip level
    bool m_Immutable; ///< true if this is a immutable texture ( size and format )

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
    QDemonRenderTextureBase(QDemonRenderContextImpl &context,
                            QDemonRenderTextureTargetType::Enum texTarget);

    virtual ~QDemonRenderTextureBase();

    virtual void SetMinFilter(QDemonRenderTextureMinifyingOp::Enum value);
    virtual void SetMagFilter(QDemonRenderTextureMagnifyingOp::Enum value);

    virtual void SetBaseLevel(qint32 value);
    virtual void SetMaxLevel(qint32 value);

    virtual void SetTextureWrapS(QDemonRenderTextureCoordOp::Enum value);
    virtual void SetTextureWrapT(QDemonRenderTextureCoordOp::Enum value);

    virtual void SetTextureCompareMode(QDemonRenderTextureCompareMode::Enum value);
    virtual void SetTextureCompareFunc(QDemonRenderTextureCompareOp::Enum value);

    virtual void SetTextureUnit(quint32 unit) { m_TextureUnit = unit; }
    virtual quint32 GetTextureUnit() const { return m_TextureUnit; }

    // Get the texture details for mipmap level 0 if it was set.
    virtual STextureDetails GetTextureDetails() const = 0;

    virtual bool IsMultisampleTexture() const
    {
        return (m_TexTarget == QDemonRenderTextureTargetType::Texture2D_MS);
    }
    virtual quint32 GetSampleCount() const { return m_SampleCount; }
    virtual bool IsImmutableTexture() const { return m_Immutable; }

    /**
         * @brief Bind a texture for shader access
         *
         *
         * @return No return.
         */
    virtual void Bind() = 0;

    virtual quint32 GetNumMipmaps() { return m_MaxMipLevel; }

    /**
         * @brief Query if texture needs coordinate swizzle
         *
         * @return texture swizzle mode
         */
    virtual QDemonRenderTextureSwizzleMode::Enum GetTextureSwizzleMode()
    {
        // if our backend supports hardware texture swizzle then there is no need for a shader
        // swizzle
        return (m_Backend->GetRenderBackendCap(
                    QDemonRenderBackend::QDemonRenderBackendCaps::TexSwizzle))
                ? QDemonRenderTextureSwizzleMode::NoSwizzle
                : m_Backend->GetTextureSwizzleMode(m_Format);
    }

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    virtual QDemonRenderBackend::QDemonRenderBackendTextureObject GetTextureObjectHandle()
    {
        return m_TextureHandle;
    }

protected:
    void applyTexParams();
    void applyTexSwizzle();
};

QT_END_NAMESPACE

#endif
