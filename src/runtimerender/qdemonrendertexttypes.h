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
#pragma once
#ifndef QDEMON_RENDER_TEXT_TYPES_H
#define QDEMON_RENDER_TEXT_TYPES_H
#include <qdemonrender.h>
#include <Qt3DSSimpleTypes.h>
#include <Qt3DSDataRef.h>
#include <StringTable.h>
#include <QVector2D.h>

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

struct STextDimensions
{
    quint32 m_TextWidth;
    quint32 m_TextHeight;
    STextDimensions(quint32 w, quint32 h)
        : m_TextWidth(w)
        , m_TextHeight(h)
    {
    }
    STextDimensions()
        : m_TextWidth(0)
        , m_TextHeight(0)
    {
    }
};

struct STextTextureDetails : public STextDimensions
{
    QVector2D m_ScaleFactor;
    bool m_FlipY;
    STextTextureDetails(quint32 w, quint32 h, bool inFlipY, QVector2D scaleF)
        : STextDimensions(w, h)
        , m_ScaleFactor(scaleF)
        , m_FlipY(inFlipY)
    {
    }
    STextTextureDetails()
        : m_ScaleFactor(1.0f)
        , m_FlipY(false)
    {
    }
};

struct STextTextureAtlasEntryDetails : public STextDimensions
{
    qint32 m_X, m_Y;
    STextTextureAtlasEntryDetails(quint32 w, quint32 h, qint32 x, qint32 y)
        : STextDimensions(w, h)
        , m_X(x)
        , m_Y(y)
    {
    }
    STextTextureAtlasEntryDetails()
        : m_X(0)
        , m_Y(0)
    {
    }
};

struct SRenderTextureAtlasDetails
{
    quint32 m_VertexCount;
    QDemonDataRef<quint8> m_Vertices;

    SRenderTextureAtlasDetails(quint32 count, QDemonDataRef<quint8> inVertices)
        : m_VertexCount(count)
        , m_Vertices(inVertices)
    {
    }
    SRenderTextureAtlasDetails()
        : m_VertexCount(0)
        , m_Vertices(QDemonDataRef<quint8>())
    {
    }
};

struct STextTextureAtlasDetails : public STextTextureDetails
{
    quint32 m_EntryCount;
    STextTextureAtlasDetails(quint32 w, quint32 h, bool inFlipY, quint32 count)
        : STextTextureDetails(w, h, inFlipY, QVector2D(1.0f))
        , m_EntryCount(count)
    {
    }
    STextTextureAtlasDetails()
        : m_EntryCount(0)
    {
    }
};

// Adding/removing a member to this object means you need to update the texture cache code
// - UICRenderTextTextureCache.cpp

struct STextRenderInfo
{
    CRegisteredString m_Text;
    CRegisteredString m_Font;
    float m_FontSize;
    TextHorizontalAlignment::Enum m_HorizontalAlignment;
    TextVerticalAlignment::Enum m_VerticalAlignment;
    float m_Leading; // space between lines
    float m_Tracking; // space between letters
    bool m_DropShadow;
    float m_DropShadowStrength;
    float m_DropShadowOffset;
    TextHorizontalAlignment::Enum m_DropShadowHorizontalAlignment;
    TextVerticalAlignment::Enum m_DropShadowVerticalAlignment;

    float m_ScaleX; // Pixel scale in X
    float m_ScaleY; // Pixel scale in Y

    bool m_EnableAcceleratedFont; ///< use NV path rendering

    STextRenderInfo();
    ~STextRenderInfo();
};
QT_END_NAMESPACE

#endif
