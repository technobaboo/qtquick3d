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
#ifndef QDEMON_RENDER_PATH_H
#define QDEMON_RENDER_PATH_H

#include <QtDemonRuntimeRender/qdemonrendernode.h>

QT_BEGIN_NAMESPACE

struct QDemonRenderSubPath;

struct QDemonRenderPath : public QDemonRenderNode
{
    enum class Capping : quint8
    {
        None = 0,
        Taper = 1,
    };

    enum class PathType : quint8
    {
        None = 0,
        Painted,
        Geometry,
    };

    enum class PaintStyle : quint8
    {
        None = 0,
        FilledAndStroked,
        Filled,
        Stroked,
    };

    PathType m_pathType = PathType::Geometry;
    float m_width = 5.0f;
    float m_linearError = 100.0f;
    float m_edgeTessAmount = 8.0f;
    float m_innerTessAmount = 1.0f;
    Capping m_beginCapping = Capping::None;
    float m_beginCapOffset = 10.f;
    float m_beginCapOpacity = 0.2f;
    float m_beginCapWidth = 0.0f;
    Capping m_endCapping = Capping::None;
    float m_endCapOffset = 10.0f;
    float m_endCapOpacity = 0.2f;
    float m_endCapWidth = 0.0f;
    QDemonRenderGraphObject *m_material = nullptr;
    QDemonRenderGraphObject *m_secondMaterial = nullptr;
    // Paths can either be immediate - children attached define path
    // or they can link to a path buffer that defines the path.
    QDemonRenderSubPath *m_firstSubPath = nullptr;
    QString m_pathBuffer;
    QDemonRenderPath::PaintStyle m_paintStyle = PaintStyle::Stroked;

    bool m_wireframeMode = false;
    // Loaded onto the card just as data.
    QDemonRenderPath() : QDemonRenderNode(Type::Path) {}

    bool isStroked() const
    {
        return m_paintStyle == PaintStyle::Stroked || m_paintStyle == PaintStyle::FilledAndStroked;
    }

    bool isFilled() const
    {
        return m_paintStyle == PaintStyle::Filled || m_paintStyle == PaintStyle::FilledAndStroked;
    }

    void addMaterial(QDemonRenderGraphObject *inMaterial)
    {
        if (m_material == nullptr)
            m_material = inMaterial;
        else
            m_secondMaterial = inMaterial;
    }

    void clearMaterials()
    {
        m_material = nullptr;
        m_secondMaterial = nullptr;
    }

    void addSubPath(QDemonRenderSubPath &inSubPath);
    void clearSubPaths();
};
QT_END_NAMESPACE
#endif
