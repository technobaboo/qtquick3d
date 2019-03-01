/****************************************************************************
**
** Copyright (C) 2014 NVIDIA Corporation.
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
#include <QtDemonRender/qdemonrendertexturecube.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderTextureCube::QDemonRenderTextureCube(const QSharedPointer<QDemonRenderContextImpl> &context,
                                                 QDemonRenderTextureTargetType::Enum texTarget)
    : QDemonRenderTextureBase(context, texTarget)
    , m_width(0)
    , m_height(0)
{
}

QDemonRenderTextureCube::~QDemonRenderTextureCube()
{
    m_context->textureDestroyed(this);
}

void QDemonRenderTextureCube::setTextureData(QDemonDataRef<quint8> newBuffer, quint8 inMipLevel,
                                             QDemonRenderTextureCubeFaces::Enum inFace, quint32 width,
                                             quint32 height, QDemonRenderTextureFormats::Enum format)
{
    Q_ASSERT(m_textureHandle);
    Q_ASSERT(inFace != QDemonRenderTextureCubeFaces::InvalidFace);

    if (inMipLevel == 0) {
        m_width = width;
        m_height = height;
        m_format = format;
        m_maxMipLevel = inMipLevel;
    }

    if (m_maxMipLevel < inMipLevel) {
        m_maxMipLevel = inMipLevel;
    }

    // get max size and check value
    qint32 theMaxSize;
    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize,
                                     &theMaxSize);
    if (width > (quint32)theMaxSize || height > (quint32)theMaxSize) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)",
                   theMaxSize, theMaxSize);
    }

    QDemonRenderTextureTargetType::Enum outTarget =
            static_cast<QDemonRenderTextureTargetType::Enum>((int)m_texTarget + (int)inFace);
    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(format)
            || QDemonRenderTextureFormats::isDepthTextureFormat(format)) {
        m_backend->setTextureDataCubeFace(m_textureHandle, outTarget, inMipLevel, format, width,
                                          height, 0, format, newBuffer.begin());
    } else if (QDemonRenderTextureFormats::isCompressedTextureFormat(format)) {
        m_backend->setCompressedTextureDataCubeFace(m_textureHandle, outTarget, inMipLevel,
                                                    format, width, height, 0, newBuffer.size(),
                                                    newBuffer.begin());
    }

    // Set our texture parameters to a default that will look the best
    if (inMipLevel > 0)
        setMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
}

QDemonTextureDetails QDemonRenderTextureCube::getTextureDetails() const
{
    return QDemonTextureDetails(m_width, m_height, 6, m_sampleCount, m_format);
}

void QDemonRenderTextureCube::bind()
{
    m_textureUnit = m_context->getNextTextureUnit();

    m_backend->bindTexture(m_textureHandle, m_texTarget, m_textureUnit);

    applyTexParams();
    applyTexSwizzle();
}

QSharedPointer<QDemonRenderTextureCube> QDemonRenderTextureCube::create(const QSharedPointer<QDemonRenderContextImpl> &context)
{
    return QSharedPointer<QDemonRenderTextureCube>(new QDemonRenderTextureCube(context));
}
QT_END_NAMESPACE
