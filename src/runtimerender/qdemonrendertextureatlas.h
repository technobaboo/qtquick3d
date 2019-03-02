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

class QDemonTextRendererInterface;
class QDemonRenderContext;

struct QDemonTextureAtlasRect
{

    QDemonTextureAtlasRect() = default;
    QDemonTextureAtlasRect(qint32 inX, qint32 inY, qint32 inW, qint32 inH)
        : x(inX)
        , y(inY)
        , width(inW)
        , height(inH)
    {
    }

    qint32 x = 0;
    qint32 y = 0;
    qint32 width = 0;
    qint32 height = 0;

    // normalized coordinates
    float normX;
    float normY;
    float normWidth;
    float normHeight;
};

typedef QPair<QDemonTextureAtlasRect, QDemonDataRef<quint8>> TTextureAtlasEntryAndBuffer;

/**
     *	Abstract class of a texture atlas representation
    */
class QDemonTextureAtlasInterface
{
protected:
    virtual ~QDemonTextureAtlasInterface() {}

public:
    virtual qint32 getWidth() const = 0;
    virtual qint32 getHeight() const = 0;
    virtual qint32 getAtlasEntryCount() const = 0;
    virtual TTextureAtlasEntryAndBuffer getAtlasEntryByIndex(quint32 index) = 0;

    virtual QDemonTextureAtlasRect addAtlasEntry(qint32 width,
                                            qint32 height,
                                            qint32 pitch,
                                            qint32 dataWidth,
                                            QDemonConstDataRef<quint8> bufferData) = 0;
    virtual void relaseEntries() = 0;

    static QDemonRef<QDemonTextureAtlasInterface> createTextureAtlas(QDemonRef<QDemonRenderContext> inRenderContext, qint32 width, qint32 height);
};
QT_END_NAMESPACE

#endif
