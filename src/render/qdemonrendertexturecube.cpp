/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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
#include <QtDemonRender/qdemonrendertexturecube.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderTextureCube::QDemonRenderTextureCube(const QDemonRef<QDemonRenderContext> &context)
    : QDemonRenderTextureBase(context, QDemonRenderTextureTargetType::TextureCube), m_width(0), m_height(0)
{
}

QDemonRenderTextureCube::~QDemonRenderTextureCube()
{
}

void QDemonRenderTextureCube::setTextureData(QDemonByteView newBuffer,
                                             quint8 inMipLevel,
                                             QDemonRenderTextureCubeFace inFace,
                                             quint32 width,
                                             quint32 height,
                                             QDemonRenderTextureFormat format)
{
    Q_ASSERT(m_handle);
    Q_ASSERT(inFace != QDemonRenderTextureCubeFace::InvalidFace);

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
    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize, &theMaxSize);
    if (width > (quint32)theMaxSize || height > (quint32)theMaxSize) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)", theMaxSize, theMaxSize);
    }

    QDemonRenderTextureTargetType outTarget = static_cast<QDemonRenderTextureTargetType>((int)m_texTarget + (int)inFace);
    if (format.isUncompressedTextureFormat() || format.isDepthTextureFormat()) {
        m_backend->setTextureDataCubeFace(m_handle,
                                          outTarget,
                                          inMipLevel,
                                          format,
                                          width,
                                          height,
                                          0,
                                          format,
                                          newBuffer);
    } else if (format.isCompressedTextureFormat()) {
        m_backend->setCompressedTextureDataCubeFace(m_handle,
                                                    outTarget,
                                                    inMipLevel,
                                                    format,
                                                    width,
                                                    height,
                                                    0,
                                                    newBuffer);
    }

    // Set our texture parameters to a default that will look the best
    if (inMipLevel > 0)
        setMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
}

QDemonTextureDetails QDemonRenderTextureCube::textureDetails() const
{
    return QDemonTextureDetails(m_width, m_height, 6, m_sampleCount, m_format);
}

void QDemonRenderTextureCube::bind()
{
    m_textureUnit = m_context->nextTextureUnit();

    m_backend->bindTexture(m_handle, m_texTarget, m_textureUnit);

    applyTexParams();
    applyTexSwizzle();
}

QT_END_NAMESPACE
