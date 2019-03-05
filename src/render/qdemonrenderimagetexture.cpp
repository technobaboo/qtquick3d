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
#include <QtDemonRender/qdemonrenderimagetexture.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemon/qdemonutils.h>
#include <limits>

QT_BEGIN_NAMESPACE

QDemonRenderImage2D::QDemonRenderImage2D(const QDemonRef<QDemonRenderContextImpl> &context,
                                         const QDemonRef<QDemonRenderTexture2D> &inTexture,
                                         QDemonRenderImageAccessType::Enum inAccess)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_texture2D(inTexture)
    , m_textureUnit(std::numeric_limits<int>::max())
    , m_accessType(inAccess)
    , m_textureLevel(0)
{
}

QDemonRenderImage2D::~QDemonRenderImage2D()
{
    m_context->imageDestroyed(this);
}

void QDemonRenderImage2D::setTextureLevel(qint32 inLevel)
{
    if (m_texture2D && m_texture2D->getNumMipmaps() >= (quint32)inLevel) {
        m_textureLevel = inLevel;
    }
}

void QDemonRenderImage2D::bind(qint32 unit)
{
    if (unit == -1)
        m_textureUnit = m_context->getNextTextureUnit();
    else
        m_textureUnit = unit;

    QDemonTextureDetails theDetails(m_texture2D->getTextureDetails());

    // note it is the callers responsibility that the texture format is supported by the compute
    // shader
    m_backend->bindImageTexture(m_texture2D->getTextureObjectHandle(),
                                m_textureUnit,
                                m_textureLevel,
                                false,
                                0,
                                m_accessType,
                                theDetails.format);
}

QDemonRenderBackend::QDemonRenderBackendTextureObject QDemonRenderImage2D::getTextureObjectHandle()
{
    return m_texture2D->getTextureObjectHandle();
}

QDemonRef<QDemonRenderImage2D> QDemonRenderImage2D::create(const QDemonRef<QDemonRenderContextImpl> &context,
                                                           const QDemonRef<QDemonRenderTexture2D> &inTexture,
                                                           QDemonRenderImageAccessType::Enum inAccess)
{
    QDemonRef<QDemonRenderImage2D> retval;
    if (inTexture)
        retval = new QDemonRenderImage2D(context, inTexture, inAccess);
    return retval;
}
QT_END_NAMESPACE
