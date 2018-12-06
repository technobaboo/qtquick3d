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
#ifndef QDEMON_RENDER_PATH_H
#define QDEMON_RENDER_PATH_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>

QT_BEGIN_NAMESPACE
struct PathCapping
{
    enum Enum {
        Noner = 0,
        Taper = 1,
    };
};

struct PathTypes
{
    enum Enum {
        Noner = 0,
        Painted,
        Geometry,
    };
};

struct PathPaintStyles
{
    enum Enum {
        Noner = 0,
        FilledAndStroked,
        Filled,
        Stroked,
    };
};

struct SPath : public SNode
{
    PathTypes::Enum m_PathType;
    float m_Width;
    float m_LinearError;
    float m_EdgeTessAmount;
    float m_InnerTessAmount;
    PathCapping::Enum m_BeginCapping;
    float m_BeginCapOffset;
    float m_BeginCapOpacity;
    float m_BeginCapWidth;
    PathCapping::Enum m_EndCapping;
    float m_EndCapOffset;
    float m_EndCapOpacity;
    float m_EndCapWidth;
    SGraphObject *m_Material;
    SGraphObject *m_SecondMaterial;
    // Paths can either be immediate - children attached define path
    // or they can link to a path buffer that defines the path.
    SPathSubPath *m_FirstSubPath;
    CRegisteredString m_PathBuffer;
    PathPaintStyles::Enum m_PaintStyle;

    bool m_WireframeMode;
    // Loaded onto the card just as data.
    SPath()
        : SNode(GraphObjectTypes::Path)
        , m_PathType(PathTypes::Geometry)
        , m_Width(5.0f)
        , m_LinearError(100.0f)
        , m_EdgeTessAmount(8.0f)
        , m_InnerTessAmount(1.0f)
        , m_BeginCapping(PathCapping::Noner)
        , m_BeginCapOffset(10.0f)
        , m_BeginCapOpacity(.2f)
        , m_BeginCapWidth(0.0f)
        , m_EndCapping(PathCapping::Noner)
        , m_EndCapOffset(10.0f)
        , m_EndCapOpacity(.2f)
        , m_EndCapWidth(0.0f)
        , m_Material(nullptr)
        , m_SecondMaterial(nullptr)
        , m_FirstSubPath(nullptr)
        , m_PaintStyle(PathPaintStyles::Stroked)
        , m_WireframeMode(false)
    {
    }

    bool IsStroked() const
    {
        return m_PaintStyle == PathPaintStyles::Stroked
                || m_PaintStyle == PathPaintStyles::FilledAndStroked;
    }

    bool IsFilled() const
    {
        return m_PaintStyle == PathPaintStyles::Filled
                || m_PaintStyle == PathPaintStyles::FilledAndStroked;
    }

    void AddMaterial(SGraphObject *inMaterial)
    {
        if (m_Material == nullptr)
            m_Material = inMaterial;
        else
            m_SecondMaterial = inMaterial;
    }

    void ClearMaterials()
    {
        m_Material = nullptr;
        m_SecondMaterial = nullptr;
    }

    void AddSubPath(SPathSubPath &inSubPath);
    void ClearSubPaths();

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SNode::Remap(inRemapper);
        inRemapper.Remap(m_PathBuffer);
        inRemapper.RemapMaterial(m_Material);
        inRemapper.RemapMaterial(m_SecondMaterial);
        inRemapper.Remap(m_FirstSubPath);
    }
};
QT_END_NAMESPACE
#endif
