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
#ifndef QDEMON_RENDER_RESOURCE_TEXTURE_2D_H
#define QDEMON_RENDER_RESOURCE_TEXTURE_2D_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendertexture2darray.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>

QT_BEGIN_NAMESPACE
class QDemonResourceTexture2D
{
public:
    QAtomicInt ref;
protected:
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QDemonRef<QDemonRenderTexture2D> m_texture;
    QDemonTextureDetails m_textureDetails;

public:
    QDemonResourceTexture2D(QDemonRef<QDemonResourceManagerInterface> mgr,
                            QDemonRef<QDemonRenderTexture2D> inTexture = nullptr);
    // create and allocate the texture right away.
    QDemonResourceTexture2D(QDemonRef<QDemonResourceManagerInterface> mgr,
                            quint32 width,
                            quint32 height,
                            QDemonRenderTextureFormats::Enum inFormat,
                            quint32 inSamples = 1);
    ~QDemonResourceTexture2D();
    // Returns true if the texture matches the specs, false if the texture needs to be
    // reallocated
    bool textureMatches(quint32 width,
                        quint32 height,
                        QDemonRenderTextureFormats::Enum inFormat,
                        quint32 inSamples = 1);

    // Returns true if the texture was allocated, false if nothing changed (no allocation).
    // Note this is the exact opposite of TextureMatches.
    bool ensureTexture(quint32 width,
                       quint32 height,
                       QDemonRenderTextureFormats::Enum inFormat,
                       quint32 inSamples = 1);

    // Force release the texture.
    void releaseTexture();
    QDemonRenderTexture2D &operator*()
    {
        Q_ASSERT(m_texture);
        return *m_texture;
    }
    QDemonRef<QDemonRenderTexture2D> operator->()
    {
        Q_ASSERT(m_texture);
        return m_texture;
    }
    operator QDemonRef<QDemonRenderTexture2D> () { return m_texture; }
    QDemonRef<QDemonRenderTexture2D> getTexture() { return m_texture; }
    void forgetTexture();
    // Enforces single ownership rules.
    void stealTexture(QDemonResourceTexture2D &inOther);
};

class QDemonResourceTexture2DArray
{
protected:
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QDemonRef<QDemonRenderTexture2DArray> m_texture;
    QDemonTextureDetails m_textureDetails;

public:
    QDemonResourceTexture2DArray(QDemonRef<QDemonResourceManagerInterface> mgr);
    // create and allocate the texture right away.
    QDemonResourceTexture2DArray(QDemonRef<QDemonResourceManagerInterface> mgr,
                                 quint32 width,
                                 quint32 height,
                                 quint32 slices,
                                 QDemonRenderTextureFormats::Enum inFormat,
                                 quint32 inSamples = 1);
    ~QDemonResourceTexture2DArray();
    // Returns true if the texture matches the specs, false if the texture needs to be
    // reallocated
    bool textureMatches(quint32 width,
                        quint32 height,
                        quint32 slices,
                        QDemonRenderTextureFormats::Enum inFormat,
                        quint32 inSamples = 1);

    // Returns true if the texture was allocated, false if nothing changed (no allocation).
    // Note this is the exact opposite of TextureMatches.
    bool ensureTexture(quint32 width,
                       quint32 height,
                       quint32 slices,
                       QDemonRenderTextureFormats::Enum inFormat,
                       quint32 inSamples = 1);

    // Force release the texture.
    void releaseTexture();
    QDemonRenderTexture2DArray &operator*()
    {
        Q_ASSERT(m_texture);
        return *m_texture;
    }
    QDemonRef<QDemonRenderTexture2DArray> operator->()
    {
        Q_ASSERT(m_texture);
        return m_texture;
    }
    operator QDemonRef<QDemonRenderTexture2DArray> () { return m_texture; }
    QDemonRef<QDemonRenderTexture2DArray> getTexture() { return m_texture; }
    // Enforces single ownership rules.
    void stealTexture(QDemonResourceTexture2DArray &inOther);
};
QT_END_NAMESPACE

#endif
