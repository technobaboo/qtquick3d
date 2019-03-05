/****************************************************************************
**
** Copyright (C) 2008-2015 NVIDIA Corporation.
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
#ifndef QDEMON_RENDER_TEXT_TYPES_H
#define QDEMON_RENDER_TEXT_TYPES_H

#include <QtDemon/qdemondataref.h>

#include <QtCore/QString>

#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

struct TextHorizontalAlignment
{
    enum Enum {
        Unknown = 0,
        Left,
        Center,
        Right,
    };
};

struct TextVerticalAlignment
{
    enum Enum {
        Unknown = 0,
        Top,
        Middle,
        Bottom,
    };
};

struct QDemonTextDimensions
{
    quint32 textWidth = 0;
    quint32 textHeight = 0;
    QDemonTextDimensions(quint32 w, quint32 h) : textWidth(w), textHeight(h) {}
    QDemonTextDimensions() = default;
};

struct QDemonTextTextureDetails : public QDemonTextDimensions
{
    QVector2D scaleFactor;
    bool flipY;
    QDemonTextTextureDetails(quint32 w, quint32 h, bool inFlipY, QVector2D scaleF)
        : QDemonTextDimensions(w, h), scaleFactor(scaleF), flipY(inFlipY)
    {
    }
    QDemonTextTextureDetails() : scaleFactor(1.0f, 1.0f), flipY(false) {}
};

struct QDemonTextTextureAtlasEntryDetails : public QDemonTextDimensions
{
    qint32 x = 0;
    qint32 y = 0;
    QDemonTextTextureAtlasEntryDetails(quint32 inW, quint32 inH, qint32 inX, qint32 inY)
        : QDemonTextDimensions(inW, inH), x(inX), y(inY)
    {
    }
    QDemonTextTextureAtlasEntryDetails() = default;
};

struct QDemonRenderTextureAtlasDetails
{
    quint32 vertexCount = 0;
    QDemonDataRef<quint8> vertices;

    QDemonRenderTextureAtlasDetails(quint32 count, QDemonDataRef<quint8> inVertices)
        : vertexCount(count), vertices(inVertices)
    {
    }
    QDemonRenderTextureAtlasDetails() = default;
};

struct QDemonTextTextureAtlasDetails : public QDemonTextTextureDetails
{
    quint32 entryCount = 0;
    QDemonTextTextureAtlasDetails(quint32 w, quint32 h, bool inFlipY, quint32 count)
        : QDemonTextTextureDetails(w, h, inFlipY, QVector2D(1.0f, 1.0f)), entryCount(count)
    {
    }
    QDemonTextTextureAtlasDetails() = default;
};

// Adding/removing a member to this object means you need to update the texture cache code
// - UICRenderTextTextureCache.cpp

struct QDemonTextRenderInfo
{
    QString text;
    QString font;
    float fontSize = 24.f;
    TextHorizontalAlignment::Enum horizontalAlignment = TextHorizontalAlignment::Center;
    TextVerticalAlignment::Enum verticalAlignment = TextVerticalAlignment::Middle;
    float leading = 0; // space between lines
    float tracking = 0; // space between letters
    bool dropShadow = false;
    float dropShadowStrength = 80;
    float dropShadowOffset = 10;
    TextHorizontalAlignment::Enum dropShadowHorizontalAlignment = TextHorizontalAlignment::Right;
    TextVerticalAlignment::Enum dropShadowVerticalAlignment = TextVerticalAlignment::Bottom;

    float scaleX = 0; // Pixel scale in X
    float scaleY = 0; // Pixel scale in Y

    bool enableAcceleratedFont = false; ///< use NV path rendering

    ~QDemonTextRenderInfo();
};
QT_END_NAMESPACE

#endif
