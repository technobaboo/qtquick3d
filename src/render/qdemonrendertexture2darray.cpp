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
#include <QtDemonRender/qdemonrendertexture2darray.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderTexture2DArray::QDemonRenderTexture2DArray(const QDemonRef<QDemonRenderContextImpl> &context,
                                                       QDemonRenderTextureTargetType::Enum texTarget)
    : QDemonRenderTextureBase(context, texTarget), m_width(0), m_height(0), m_slices(0)
{
}

QDemonRenderTexture2DArray::~QDemonRenderTexture2DArray()
{
    m_context->textureDestroyed(this);
}

void QDemonRenderTexture2DArray::setTextureData(QDemonDataRef<quint8> newBuffer,
                                                quint8 inMipLevel,
                                                qint32 width,
                                                qint32 height,
                                                qint32 slices,
                                                QDemonRenderTextureFormats::Enum format)
{
    Q_ASSERT(m_textureHandle);
    Q_ASSERT(width >= 0 && height >= 0 && slices >= 0);
    if (inMipLevel == 0) {
        m_width = width;
        m_height = height;
        m_slices = slices;
        m_format = format;
        m_maxMipLevel = inMipLevel;
    }

    if (m_maxMipLevel < inMipLevel) {
        m_maxMipLevel = inMipLevel;
    }

    // get max size and check value
    qint32 theMaxLayerSize, theMaxSize;
    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize, &theMaxSize);
    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureArrayLayers, &theMaxLayerSize);
    if (width > theMaxSize || height > theMaxSize || slices > theMaxLayerSize) {
        qCCritical(INVALID_OPERATION, "Width or height or Slices is greater than max texture size (%d, %d, %d)", theMaxSize, theMaxSize, theMaxLayerSize);
    }

    // currently we do not support compressed texture arrays
    Q_ASSERT(QDemonRenderTextureFormats::isUncompressedTextureFormat(format)
             || QDemonRenderTextureFormats::isDepthTextureFormat(format));

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(format) || QDemonRenderTextureFormats::isDepthTextureFormat(format)) {
        m_backend->setTextureData3D(m_textureHandle,
                                    m_texTarget,
                                    inMipLevel,
                                    m_format,
                                    width,
                                    height,
                                    slices,
                                    0,
                                    format,
                                    newBuffer.begin());
    }

    // Set our texture parameters to a default that will look the best
    if (inMipLevel > 0)
        setMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
}

QDemonTextureDetails QDemonRenderTexture2DArray::getTextureDetails() const
{
    return QDemonTextureDetails(m_width, m_height, m_slices, m_sampleCount, m_format);
}

void QDemonRenderTexture2DArray::bind()
{
    m_textureUnit = m_context->getNextTextureUnit();

    m_backend->bindTexture(m_textureHandle, m_texTarget, m_textureUnit);

    applyTexParams();
    applyTexSwizzle();
}

QDemonRef<QDemonRenderTexture2DArray> QDemonRenderTexture2DArray::create(const QDemonRef<QDemonRenderContextImpl> &context)
{
    return QDemonRef<QDemonRenderTexture2DArray>(new QDemonRenderTexture2DArray(context));
}
QT_END_NAMESPACE
