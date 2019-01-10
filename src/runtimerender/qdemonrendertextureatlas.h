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
#ifndef QDEMON_RENDER_TEXTURE_ATLAS_H
#define QDEMON_RENDER_TEXTURE_ATLAS_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

class ITextRenderer;
class QDemonRenderContext;

struct STextureAtlasRect
{

    STextureAtlasRect()
        : m_X(0)
        , m_Y(0)
        , m_Width(0)
        , m_Height(0)
    {
    }

    STextureAtlasRect(qint32 x, qint32 y, qint32 w, qint32 h)
        : m_X(x)
        , m_Y(y)
        , m_Width(w)
        , m_Height(h)
    {
    }

    qint32 m_X;
    qint32 m_Y;
    qint32 m_Width;
    qint32 m_Height;

    // normalized coordinates
    float m_NormX;
    float m_NormY;
    float m_NormWidth;
    float m_NormHeight;
};

typedef QPair<STextureAtlasRect, QDemonDataRef<quint8>> TTextureAtlasEntryAndBuffer;

/**
     *	Abstract class of a texture atlas representation
    */
class ITextureAtlas
{
protected:
    virtual ~ITextureAtlas() {}

public:
    virtual qint32 GetWidth() const = 0;
    virtual qint32 GetHeight() const = 0;
    virtual qint32 GetAtlasEntryCount() const = 0;
    virtual TTextureAtlasEntryAndBuffer GetAtlasEntryByIndex(quint32 index) = 0;

    virtual STextureAtlasRect AddAtlasEntry(qint32 width, qint32 height, qint32 pitch,
                                            qint32 dataWidth,
                                            QDemonConstDataRef<quint8> bufferData) = 0;
    virtual void RelaseEntries() = 0;

    static QSharedPointer<ITextureAtlas> CreateTextureAtlas(QSharedPointer<QDemonRenderContext> inRenderContext, qint32 width, qint32 height);
};
QT_END_NAMESPACE

#endif
