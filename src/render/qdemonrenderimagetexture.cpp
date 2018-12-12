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

QDemonRenderImage2D::QDemonRenderImage2D(QDemonRenderContextImpl &context,
                                         QSharedPointer<QDemonRenderTexture2D> inTexture,
                                         QDemonRenderImageAccessType::Enum inAccess)
    : m_Context(context)
    , m_Backend(context.GetBackend())
    , m_Texture2D(inTexture)
    , m_TextureUnit(std::numeric_limits<int>::max())
    , m_AccessType(inAccess)
    , m_TextureLevel(0)
{
}

QDemonRenderImage2D::~QDemonRenderImage2D()
{
    m_Context.ImageDestroyed(this);
}

void QDemonRenderImage2D::SetTextureLevel(qint32 inLevel)
{
    if (m_Texture2D && m_Texture2D->GetNumMipmaps() >= (quint32)inLevel) {
        m_TextureLevel = inLevel;
    }
}

void QDemonRenderImage2D::Bind(quint32 unit)
{
    if (unit == -1)
        m_TextureUnit = m_Context.GetNextTextureUnit();
    else
        m_TextureUnit = unit;

    STextureDetails theDetails(m_Texture2D->GetTextureDetails());

    // note it is the callers responsibility that the texture format is supported by the compute
    // shader
    m_Backend->BindImageTexture(m_Texture2D->GetTextureObjectHandle(), m_TextureUnit,
                                m_TextureLevel, false, 0, m_AccessType, theDetails.m_Format);
}

QDemonRenderBackend::QDemonRenderBackendTextureObject QDemonRenderImage2D::GetTextureObjectHandle()
{
    return m_Texture2D->GetTextureObjectHandle();
}

QSharedPointer<QDemonRenderImage2D> QDemonRenderImage2D::Create(QDemonRenderContextImpl &context,
                                                 QSharedPointer<QDemonRenderTexture2D> inTexture,
                                                 QDemonRenderImageAccessType::Enum inAccess)
{
    QSharedPointer<QDemonRenderImage2D> retval;
    if (inTexture)
        retval.reset(new QDemonRenderImage2D(context, inTexture, inAccess));
    return retval;
}
QT_END_NAMESPACE
