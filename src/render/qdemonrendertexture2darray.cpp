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

QDemonRenderTexture2DArray::QDemonRenderTexture2DArray(QDemonRenderContextImpl &context,
                                                       QDemonRenderTextureTargetType::Enum texTarget)
    : QDemonRenderTextureBase(context, texTarget)
    , m_Width(0)
    , m_Height(0)
    , m_Slices(0)
{
}

QDemonRenderTexture2DArray::~QDemonRenderTexture2DArray()
{
    m_Context.TextureDestroyed(this);
}

void QDemonRenderTexture2DArray::SetTextureData(QDemonDataRef<quint8> newBuffer, quint8 inMipLevel,
                                                quint32 width, quint32 height, quint32 slices,
                                                QDemonRenderTextureFormats::Enum format)
{
    Q_ASSERT(m_TextureHandle);

    if (inMipLevel == 0) {
        m_Width = width;
        m_Height = height;
        m_Slices = slices;
        m_Format = format;
        m_MaxMipLevel = inMipLevel;
    }

    if (m_MaxMipLevel < inMipLevel) {
        m_MaxMipLevel = inMipLevel;
    }

    // get max size and check value
    qint32 theMaxLayerSize, theMaxSize;
    m_Backend->GetRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize,
                                     &theMaxSize);
    m_Backend->GetRenderBackendValue(
                QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureArrayLayers, &theMaxLayerSize);
    if (width > (quint32)theMaxSize || height > (quint32)theMaxSize
            || slices > (quint32)theMaxLayerSize) {
        qCCritical(INVALID_OPERATION,
                   "Width or height or Slices is greater than max texture size (%d, %d, %d)",
                   theMaxSize, theMaxSize, theMaxLayerSize);
    }

    // currently we do not support compressed texture arrays
    Q_ASSERT(QDemonRenderTextureFormats::isUncompressedTextureFormat(format)
             || QDemonRenderTextureFormats::isDepthTextureFormat(format));

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(format)
            || QDemonRenderTextureFormats::isDepthTextureFormat(format)) {
        m_Backend->SetTextureData3D(m_TextureHandle, m_TexTarget, inMipLevel, m_Format, width,
                                    height, slices, 0, format, newBuffer.begin());
    }

    // Set our texture parameters to a default that will look the best
    if (inMipLevel > 0)
        SetMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
}

STextureDetails QDemonRenderTexture2DArray::GetTextureDetails() const
{
    return STextureDetails(m_Width, m_Height, m_Slices, m_SampleCount, m_Format);
}

void QDemonRenderTexture2DArray::Bind()
{
    m_TextureUnit = m_Context.GetNextTextureUnit();

    m_Backend->BindTexture(m_TextureHandle, m_TexTarget, m_TextureUnit);

    applyTexParams();
    applyTexSwizzle();
}

QSharedPointer<QDemonRenderTexture2DArray> QDemonRenderTexture2DArray::Create(QDemonRenderContextImpl &context)
{
    return QSharedPointer<QDemonRenderTexture2DArray>(new QDemonRenderTexture2DArray(context));
}
QT_END_NAMESPACE
