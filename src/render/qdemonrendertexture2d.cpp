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

QDemonRenderTexture2D::QDemonRenderTexture2D(const QDemonRef<QDemonRenderContextImpl> &context,
                                             QDemonRenderTextureTargetType texTarget)
    : QDemonRenderTextureBase(context, texTarget), m_width(0), m_height(0)
{
}

QDemonRenderTexture2D::~QDemonRenderTexture2D()
{
    m_context->textureDestroyed(this);
}

QDemonTextureDetails QDemonRenderTexture2D::getTextureDetails() const
{
    return QDemonTextureDetails(m_width, m_height, 0, m_sampleCount, m_format);
}

void QDemonRenderTexture2D::setTextureData(QDemonDataRef<quint8> newBuffer,
                                           quint8 inMipLevel,
                                           qint32 width,
                                           qint32 height,
                                           QDemonRenderTextureFormat format,
                                           QDemonRenderTextureFormat formatDest)
{
    Q_ASSERT(m_textureHandle);

    // check if we should compress this texture

    if (inMipLevel == 0) {
        m_width = width;
        m_height = height;
        m_format = format;

        // We re-use textures and this might have been a MSAA texture before
        // for resue we must completely destroy the texture object and create a new one
        // The same is true for immutable textures
        if (m_texTarget == QDemonRenderTextureTargetType::Texture2D_MS || m_immutable) {
            m_backend->releaseTexture(m_textureHandle);
            m_texTarget = QDemonRenderTextureTargetType::Texture2D;
            m_sampleCount = 1;
            m_textureHandle = m_backend->createTexture();
        }

        if (formatDest.isCompressedTextureFormat()) {
            bool compress = format.isUncompressedTextureFormat();
            bool appropriateSizes = !((width % 4) || (height % 4));

            // we only compress multiple of 4 textures
            if (compress && !appropriateSizes)
                compress = false;

            if (compress) {
                // This seems like a very dubious line here.  If we are compressing then the
                // image
                // is really 1/4 the width and height? - CN
                m_width = width / 4;
                m_height = height / 4;
                m_format = formatDest;
            }
        } else if (formatDest.isUncompressedTextureFormat()) {
            m_format = formatDest;
        }
    }

    if (m_maxMipLevel < inMipLevel) {
        m_maxMipLevel = inMipLevel;
    }

    // get max size and check value
    qint32 maxWidth, maxHeight;
    m_context->getMaxTextureSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)", maxWidth, maxHeight);
    }
    if (format.isUncompressedTextureFormat() || format.isDepthTextureFormat()) {
        m_backend->setTextureData2D(m_textureHandle,
                                    m_texTarget,
                                    inMipLevel,
                                    m_format,
                                    width,
                                    height,
                                    0,
                                    format,
                                    newBuffer.begin());
    } else if (format.isCompressedTextureFormat()) {
        m_backend->setCompressedTextureData2D(m_textureHandle,
                                              m_texTarget,
                                              inMipLevel,
                                              format,
                                              width,
                                              height,
                                              0,
                                              newBuffer.size(),
                                              newBuffer.begin());
    }
    // Set our texture parameters to a default that will look the best
    if (inMipLevel > 0)
        setMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
}

void QDemonRenderTexture2D::setTextureStorage(qint32 inLevels,
                                              qint32 width,
                                              qint32 height,
                                              QDemonRenderTextureFormat formaInternal,
                                              QDemonRenderTextureFormat format,
                                              QDemonDataRef<quint8> dataBuffer)
{
    Q_ASSERT(m_textureHandle);

    if (!m_context->isShaderImageLoadStoreSupported()) {
        qCCritical(INVALID_OPERATION, "The extension Shader_Image_Load_Store is not supported");
        return;
    }

    m_width = width;
    m_height = height;
    m_format = formaInternal;
    if (format == QDemonRenderTextureFormat::Unknown)
        format = formaInternal;

    // get max size and check value
    qint32 maxWidth, maxHeight;
    m_context->getMaxTextureSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)", maxWidth, maxHeight);
    }

    if (inLevels < 1) {
        qCCritical(INVALID_PARAMETER, "inLevels is less than 1 (%d)", inLevels);
    }

    m_maxMipLevel = inLevels - 1; // we count from 0

    // only uncompressed formats are supported and no depth
    if (formaInternal.isUncompressedTextureFormat()) {
        m_backend->createTextureStorage2D(m_textureHandle, m_texTarget, inLevels, formaInternal, width, height);

        m_immutable = true;
        m_texTarget = QDemonRenderTextureTargetType::Texture2D;

        if (dataBuffer.size() > 0)
            m_backend->setTextureSubData2D(m_textureHandle, m_texTarget, 0, 0, 0, width, height, format, dataBuffer.begin());

        if (inLevels > 1)
            setMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
    }
}

void QDemonRenderTexture2D::setTextureDataMultisample(qint32 sampleCount,
                                                      qint32 width,
                                                      qint32 height,
                                                      QDemonRenderTextureFormat format)
{
    Q_ASSERT(m_textureHandle);
    Q_ASSERT(m_maxMipLevel == 0);

    m_texTarget = QDemonRenderTextureTargetType::Texture2D_MS;

    qint32 maxWidth, maxHeight;
    m_context->getMaxTextureSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)", maxWidth, maxHeight);
    }

    Q_ASSERT(format.isUncompressedTextureFormat()
             || format.isDepthTextureFormat());

    m_backend->setMultisampledTextureData2D(m_textureHandle, m_texTarget, sampleCount, format, width, height, true);

    m_width = width;
    m_height = height;
    m_sampleCount = sampleCount;
    m_format = format;
}

void QDemonRenderTexture2D::setTextureSubData(QDemonDataRef<quint8> newBuffer,
                                              quint8 inMipLevel,
                                              qint32 inXOffset,
                                              qint32 inYOffset,
                                              qint32 width,
                                              qint32 height,
                                              QDemonRenderTextureFormat format)
{
    Q_ASSERT(m_textureHandle);
    Q_ASSERT(inXOffset >= 0 && inYOffset >= 0 && width >= 0 && height >= 0);

    if (!format.isUncompressedTextureFormat()) {
        qCCritical(INVALID_PARAMETER, "Cannot set sub data for depth or compressed formats");
        Q_ASSERT(false);
        return;
    }
    qint32 subRectStride = width * format.getSizeofFormat();
    if (qint32(newBuffer.size()) < subRectStride * height) {
        qCCritical(INVALID_PARAMETER, "Invalid sub rect buffer size");
        Q_ASSERT(false);
        return;
    }
    // nop
    if (width == 0 || height == 0)
        return;

    if (inXOffset + width > m_width || inYOffset + height > m_height) {
        qCCritical(INVALID_PARAMETER, "Sub rect outside existing image bounds");
        Q_ASSERT(false);
        return;
    }

    // not handled yet
    Q_ASSERT(!format.isDepthTextureFormat());

    m_backend->setTextureSubData2D(m_textureHandle,
                                   m_texTarget,
                                   inMipLevel,
                                   inXOffset,
                                   inYOffset,
                                   width,
                                   height,
                                   format,
                                   newBuffer.begin());
}

void QDemonRenderTexture2D::generateMipmaps(QDemonRenderHint::Enum genType)
{
    applyTexParams();
    m_backend->generateMipMaps(m_textureHandle, m_texTarget, genType);
    qint32 maxDim = (m_width >= m_height) ? m_width : m_height;
    m_maxMipLevel = qint32(float(std::log(maxDim)) / std::log(2.0f));
    // we never create more level than m_maxLevel
    m_maxMipLevel = qMin(m_maxMipLevel, m_maxLevel);
}

void QDemonRenderTexture2D::bind()
{
    m_textureUnit = m_context->getNextTextureUnit();

    m_backend->bindTexture(m_textureHandle, m_texTarget, m_textureUnit);

    applyTexParams();
    applyTexSwizzle();
}

QDemonRef<QDemonRenderTexture2D> QDemonRenderTexture2D::create(const QDemonRef<QDemonRenderContextImpl> &context)
{
    return QDemonRef<QDemonRenderTexture2D>(new QDemonRenderTexture2D(context));
}

QT_END_NAMESPACE
