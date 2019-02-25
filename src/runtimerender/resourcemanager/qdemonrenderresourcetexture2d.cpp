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

#include "qdemonrenderresourcetexture2d.h"

QT_BEGIN_NAMESPACE

QDemonResourceTexture2D::QDemonResourceTexture2D(QSharedPointer<QDemonResourceManagerInterface> mgr, QSharedPointer<QDemonRenderTexture2D> inTexture)
    : m_resourceManager(mgr)
    , m_texture(inTexture)
{
    if (inTexture)
        m_textureDetails = inTexture->getTextureDetails();
}

QDemonResourceTexture2D::QDemonResourceTexture2D(QSharedPointer<QDemonResourceManagerInterface> mgr,
                                                 quint32 width,
                                                 quint32 height,
                                                 QDemonRenderTextureFormats::Enum inFormat,
                                                 quint32 inSamples)
    : m_resourceManager(mgr)
    , m_texture(nullptr)
{
    ensureTexture(width, height, inFormat, inSamples);
}

QDemonResourceTexture2D::~QDemonResourceTexture2D()
{
    releaseTexture();
}

// Returns true if the texture was allocated, false if nothing changed (no allocation).
bool QDemonResourceTexture2D::textureMatches(quint32 width,
                                             quint32 height,
                                             QDemonRenderTextureFormats::Enum inFormat,
                                             quint32 inSamples)
{
    return m_texture && m_textureDetails.width == width && m_textureDetails.height == height
            && m_textureDetails.format == inFormat && m_textureDetails.sampleCount == inSamples;
}

bool QDemonResourceTexture2D::ensureTexture(quint32 width,
                                            quint32 height,
                                            QDemonRenderTextureFormats::Enum inFormat,
                                            quint32 inSamples)
{
    if (textureMatches(width, height, inFormat, inSamples))
        return false;

    if (m_texture && inSamples > 1) {
        // we cannot resize MSAA textures though release first
        releaseTexture();
    }

    if (!m_texture)
        m_texture = m_resourceManager->allocateTexture2D(width, height, inFormat, inSamples);
    else {
        // multisampled textures are immuteable
        Q_ASSERT(inSamples == 1);
        m_texture->setTextureData(QDemonDataRef<quint8>(), 0, width, height, inFormat);
    }

    m_textureDetails = m_texture->getTextureDetails();
    return true;
}

void QDemonResourceTexture2D::releaseTexture()
{
    if (m_texture) {
        m_resourceManager->release(m_texture);
        forgetTexture();
    }
}

void QDemonResourceTexture2D::forgetTexture()
{
    m_texture = nullptr;
}

void QDemonResourceTexture2D::stealTexture(QDemonResourceTexture2D &inOther)
{
    releaseTexture();
    m_texture = inOther.m_texture;
    m_textureDetails = inOther.m_textureDetails;
    inOther.m_texture = nullptr;
}

QDemonResourceTexture2DArray::QDemonResourceTexture2DArray(QSharedPointer<QDemonResourceManagerInterface> mgr)
    : m_resourceManager(mgr)
    , m_texture(nullptr)
{
}

QDemonResourceTexture2DArray::QDemonResourceTexture2DArray(QSharedPointer<QDemonResourceManagerInterface> mgr, quint32 width, quint32 height,
                                                 quint32 slices,
                                                 QDemonRenderTextureFormats::Enum inFormat,
                                                 quint32 inSamples)
    : m_resourceManager(mgr)
    , m_texture(nullptr)
{
    ensureTexture(width, height, slices, inFormat, inSamples);
}

QDemonResourceTexture2DArray::~QDemonResourceTexture2DArray()
{
    releaseTexture();
}

bool QDemonResourceTexture2DArray::textureMatches(quint32 width, quint32 height, quint32 slices,
                                             QDemonRenderTextureFormats::Enum inFormat, quint32 inSamples)
{
    return m_texture && m_textureDetails.depth == slices && m_textureDetails.width == width
            && m_textureDetails.height == height && m_textureDetails.format == inFormat
            && m_textureDetails.sampleCount == inSamples;
}

bool QDemonResourceTexture2DArray::ensureTexture(quint32 width, quint32 height, quint32 slices,
                                            QDemonRenderTextureFormats::Enum inFormat, quint32 inSamples)
{
    if (textureMatches(width, height, slices, inFormat, inSamples))
        return false;

    if (m_texture && inSamples > 1) {
        // we cannot resize MSAA textures though release first
        releaseTexture();
    }

    if (!m_texture)
        m_texture = m_resourceManager->allocateTexture2DArray(width, height, slices, inFormat, inSamples);
    else {
        // multisampled textures are immuteable
        Q_ASSERT(inSamples == 1);
        m_texture->setTextureData(QDemonDataRef<quint8>(), 0, width, height, slices, inFormat);
    }

    m_textureDetails = m_texture->getTextureDetails();
    return true;
}

void QDemonResourceTexture2DArray::releaseTexture()
{
    if (m_texture) {
        m_resourceManager->release(m_texture);
        m_texture = nullptr;
    }
}

void QDemonResourceTexture2DArray::stealTexture(QDemonResourceTexture2DArray &inOther)
{
    releaseTexture();
    m_texture = inOther.m_texture;
    m_textureDetails = inOther.m_textureDetails;
    inOther.m_texture = nullptr;
}

QT_END_NAMESPACE
