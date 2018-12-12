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

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendersampler.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderTexture2D::QDemonRenderTexture2D(QSharedPointer<QDemonRenderContextImpl> context,
                                             QDemonRenderTextureTargetType::Enum texTarget)
    : QDemonRenderTextureBase(context, texTarget)
    , m_Width(0)
    , m_Height(0)
{
}

QDemonRenderTexture2D::~QDemonRenderTexture2D()
{
    m_Context->TextureDestroyed(this);
}

STextureDetails QDemonRenderTexture2D::GetTextureDetails() const
{
    return STextureDetails(m_Width, m_Height, 0, m_SampleCount, m_Format);
}

void QDemonRenderTexture2D::SetTextureData(QDemonDataRef<quint8> newBuffer, quint8 inMipLevel, quint32 width,
                                           quint32 height, QDemonRenderTextureFormats::Enum format,
                                           QDemonRenderTextureFormats::Enum formatDest)
{
    Q_ASSERT(m_TextureHandle);

    // check if we should compress this texture

    if (inMipLevel == 0) {
        m_Width = width;
        m_Height = height;
        m_Format = format;

        // We re-use textures and this might have been a MSAA texture before
        // for resue we must completely destroy the texture object and create a new one
        // The same is true for immutable textures
        if (m_TexTarget == QDemonRenderTextureTargetType::Texture2D_MS || m_Immutable) {
            m_Backend->ReleaseTexture(m_TextureHandle);
            m_TexTarget = QDemonRenderTextureTargetType::Texture2D;
            m_SampleCount = 1;
            m_TextureHandle = m_Backend->CreateTexture();
        }

        if (QDemonRenderTextureFormats::isCompressedTextureFormat(formatDest)) {
            bool compress = QDemonRenderTextureFormats::isUncompressedTextureFormat(format);
            bool appropriateSizes = ((width % 4) || (height % 4)) == false;

            // we only compress multiple of 4 textures
            if (compress && !appropriateSizes)
                compress = false;

            if (compress) {
                // This seems like a very dubious line here.  If we are compressing then the
                // image
                // is really 1/4 the width and height? - CN
                m_Width = width / 4;
                m_Height = height / 4;
                m_Format = formatDest;
            }
        } else if (QDemonRenderTextureFormats::isUncompressedTextureFormat(formatDest)) {
            m_Format = formatDest;
        }
    }

    if (m_MaxMipLevel < inMipLevel) {
        m_MaxMipLevel = inMipLevel;
    }

    // get max size and check value
    quint32 maxWidth, maxHeight;
    m_Context->getMaxTextureSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)",
                   maxWidth, maxHeight);
    }
    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(format)
            || QDemonRenderTextureFormats::isDepthTextureFormat(format)) {
        m_Backend->SetTextureData2D(m_TextureHandle, m_TexTarget, inMipLevel, m_Format, width,
                                    height, 0, format, newBuffer.begin());
    } else if (QDemonRenderTextureFormats::isCompressedTextureFormat(format)) {
        m_Backend->SetCompressedTextureData2D(m_TextureHandle, m_TexTarget, inMipLevel, format,
                                              width, height, 0, newBuffer.size(),
                                              newBuffer.begin());
    }
    // Set our texture parameters to a default that will look the best
    if (inMipLevel > 0)
        SetMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
}

void QDemonRenderTexture2D::SetTextureStorage(quint32 inLevels, quint32 width, quint32 height,
                                              QDemonRenderTextureFormats::Enum formaInternal,
                                              QDemonRenderTextureFormats::Enum format,
                                              QDemonDataRef<quint8> dataBuffer)
{
    Q_ASSERT(m_TextureHandle);

    if (!m_Context->IsShaderImageLoadStoreSupported()) {
        qCCritical(INVALID_OPERATION, "The extension Shader_Image_Load_Store is not supported");
        return;
    }

    m_Width = width;
    m_Height = height;
    m_Format = formaInternal;
    if (format == QDemonRenderTextureFormats::Unknown)
        format = formaInternal;

    // get max size and check value
    quint32 maxWidth, maxHeight;
    m_Context->getMaxTextureSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)",
                   maxWidth, maxHeight);
    }

    if (inLevels < 1) {
        qCCritical(INVALID_PARAMETER, "inLevels is less than 1 (%d)", inLevels);
    }

    m_MaxMipLevel = inLevels - 1; // we count from 0

    // only uncompressed formats are supported and no depth
    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(formaInternal)) {
        m_Backend->CreateTextureStorage2D(m_TextureHandle, m_TexTarget, inLevels, formaInternal,
                                          width, height);

        m_Immutable = true;
        m_TexTarget = QDemonRenderTextureTargetType::Texture2D;

        if (dataBuffer.size() > 0)
            m_Backend->SetTextureSubData2D(m_TextureHandle, m_TexTarget, 0, 0, 0, width, height,
                                           format, dataBuffer.begin());

        if (inLevels > 1)
            SetMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
    }
}

void QDemonRenderTexture2D::SetTextureDataMultisample(quint32 sampleCount, quint32 width, quint32 height,
                                                      QDemonRenderTextureFormats::Enum format)
{
    Q_ASSERT(m_TextureHandle);
    Q_ASSERT(m_MaxMipLevel == 0);

    m_TexTarget = QDemonRenderTextureTargetType::Texture2D_MS;

    quint32 maxWidth, maxHeight;
    m_Context->getMaxTextureSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)",
                   maxWidth, maxHeight);
    }

    Q_ASSERT(QDemonRenderTextureFormats::isUncompressedTextureFormat(format)
             || QDemonRenderTextureFormats::isDepthTextureFormat(format));

    m_Backend->SetMultisampledTextureData2D(m_TextureHandle, m_TexTarget, sampleCount, format,
                                            width, height, true);

    m_Width = width;
    m_Height = height;
    m_SampleCount = sampleCount;
    m_Format = format;
}

void QDemonRenderTexture2D::SetTextureSubData(QDemonDataRef<quint8> newBuffer, quint8 inMipLevel,
                                              quint32 inXOffset, quint32 inYOffset, quint32 width,
                                              quint32 height, QDemonRenderTextureFormats::Enum format)
{
    Q_ASSERT(m_TextureHandle);

    if (!QDemonRenderTextureFormats::isUncompressedTextureFormat(format)) {
        qCCritical(INVALID_PARAMETER, "Cannot set sub data for depth or compressed formats");
        Q_ASSERT(false);
        return;
    }
    quint32 subRectStride = width * QDemonRenderTextureFormats::getSizeofFormat(format);
    if (newBuffer.size() < subRectStride * height) {
        qCCritical(INVALID_PARAMETER, "Invalid sub rect buffer size");
        Q_ASSERT(false);
        return;
    }
    // nop
    if (width == 0 || height == 0)
        return;

    if (inXOffset + width > m_Width || inYOffset + height > m_Height) {
        qCCritical(INVALID_PARAMETER, "Sub rect outside existing image bounds");
        Q_ASSERT(false);
        return;
    }

    // not handled yet
    Q_ASSERT(!QDemonRenderTextureFormats::isDepthTextureFormat(format));

    m_Backend->SetTextureSubData2D(m_TextureHandle, m_TexTarget, inMipLevel, inXOffset,
                                   inYOffset, width, height, format, newBuffer.begin());
}

void QDemonRenderTexture2D::GenerateMipmaps(QDemonRenderHint::Enum genType)
{
    applyTexParams();
    m_Backend->GenerateMipMaps(m_TextureHandle, m_TexTarget, genType);
    quint32 maxDim = (m_Width >= m_Height) ? m_Width : m_Height;
    m_MaxMipLevel = static_cast<quint32>(logf((float)maxDim) / logf(2.0f));
    // we never create more level than m_MaxLevel
    m_MaxMipLevel = qMin(m_MaxMipLevel, (quint32)m_MaxLevel);
}

void QDemonRenderTexture2D::Bind()
{
    m_TextureUnit = m_Context->GetNextTextureUnit();

    m_Backend->BindTexture(m_TextureHandle, m_TexTarget, m_TextureUnit);

    applyTexParams();
    applyTexSwizzle();
}

QSharedPointer<QDemonRenderTexture2D> QDemonRenderTexture2D::Create(QSharedPointer<QDemonRenderContextImpl> context)
{
    return QSharedPointer<QDemonRenderTexture2D>(new QDemonRenderTexture2D(context));
}

QT_END_NAMESPACE
