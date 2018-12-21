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
#ifndef QDEMON_RENDER_RESOURCE_TEXTURE_2D_H
#define QDEMON_RENDER_RESOURCE_TEXTURE_2D_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendertexture2darray.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>

QT_BEGIN_NAMESPACE
class CResourceTexture2D
{
protected:
    IResourceManager &m_ResourceManager;
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    STextureDetails m_TextureDetails;

public:
    CResourceTexture2D(IResourceManager &mgr, QDemonRenderTexture2D *inTexture = nullptr);
    // create and allocate the texture right away.
    CResourceTexture2D(IResourceManager &mgr, quint32 width, quint32 height,
                       QDemonRenderTextureFormats::Enum inFormat, quint32 inSamples = 1);
    ~CResourceTexture2D();
    // Returns true if the texture matches the specs, false if the texture needs to be
    // reallocated
    bool TextureMatches(quint32 width, quint32 height, QDemonRenderTextureFormats::Enum inFormat,
                        quint32 inSamples = 1);

    // Returns true if the texture was allocated, false if nothing changed (no allocation).
    // Note this is the exact opposite of TextureMatches.
    bool EnsureTexture(quint32 width, quint32 height, QDemonRenderTextureFormats::Enum inFormat,
                       quint32 inSamples = 1);

    // Force release the texture.
    void ReleaseTexture();
    QDemonRenderTexture2D &operator*()
    {
        Q_ASSERT(m_Texture);
        return *m_Texture;
    }
    QSharedPointer<QDemonRenderTexture2D> operator->()
    {
        Q_ASSERT(m_Texture);
        return m_Texture;
    }
    operator QSharedPointer<QDemonRenderTexture2D> () { return m_Texture; }
    QSharedPointer<QDemonRenderTexture2D> GetTexture() { return m_Texture; }
    void ForgetTexture();
    // Enforces single ownership rules.
    void StealTexture(CResourceTexture2D &inOther);
};

class CResourceTexture2DArray
{
protected:
    IResourceManager &m_ResourceManager;
    QSharedPointer<QDemonRenderTexture2DArray> m_Texture;
    STextureDetails m_TextureDetails;

public:
    CResourceTexture2DArray(IResourceManager &mgr);
    // create and allocate the texture right away.
    CResourceTexture2DArray(IResourceManager &mgr, quint32 width, quint32 height, quint32 slices,
                            QDemonRenderTextureFormats::Enum inFormat, quint32 inSamples = 1);
    ~CResourceTexture2DArray();
    // Returns true if the texture matches the specs, false if the texture needs to be
    // reallocated
    bool TextureMatches(quint32 width, quint32 height, quint32 slices,
                        QDemonRenderTextureFormats::Enum inFormat, quint32 inSamples = 1);

    // Returns true if the texture was allocated, false if nothing changed (no allocation).
    // Note this is the exact opposite of TextureMatches.
    bool EnsureTexture(quint32 width, quint32 height, quint32 slices,
                       QDemonRenderTextureFormats::Enum inFormat, quint32 inSamples = 1);

    // Force release the texture.
    void ReleaseTexture();
    QDemonRenderTexture2DArray &operator*()
    {
        Q_ASSERT(m_Texture);
        return *m_Texture;
    }
    QSharedPointer<QDemonRenderTexture2DArray> operator->()
    {
        Q_ASSERT(m_Texture);
        return m_Texture;
    }
    operator QSharedPointer<QDemonRenderTexture2DArray> () { return m_Texture; }
    QSharedPointer<QDemonRenderTexture2DArray> GetTexture() { return m_Texture; }
    // Enforces single ownership rules.
    void StealTexture(CResourceTexture2DArray &inOther);
};
QT_END_NAMESPACE

#endif
