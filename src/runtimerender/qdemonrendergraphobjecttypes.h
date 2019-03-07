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
#ifndef QDEMON_RENDER_GRAPH_OBJECT_TYPES_H
#define QDEMON_RENDER_GRAPH_OBJECT_TYPES_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

QT_BEGIN_NAMESPACE

// If you need a generic switch statement, then these macros will ensure
// you get all the types the first time.
#define QDEMON_RENDER_ITERATE_GRAPH_OBJECT_TYPES                                                                       \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Presentation)                                                                \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Scene)                                                                       \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Node)                                                                        \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Layer)                                                                       \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Light)                                                                       \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Camera)                                                                      \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Model)                                                                       \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(DefaultMaterial)                                                             \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Image)                                                                       \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Text)                                                                        \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Effect)                                                                      \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(RenderPlugin)                                                                \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(CustomMaterial)                                                              \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(ReferencedMaterial)                                                          \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(Path)                                                                        \
    QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(PathSubPath)

struct QDemonGraphObjectType
{
    enum Value {
        Unknown = 0,
        Presentation,
        Scene,
        Node,
        Layer,
        Light,
        Camera,
        Model,
        DefaultMaterial,
        Image,
        Text,
        Effect,
        CustomMaterial,
        RenderPlugin,
        ReferencedMaterial,
        Path,
        PathSubPath,
        Lightmaps,
        LastKnownGraphObjectType,
    };
    Value value;

    constexpr QDemonGraphObjectType(Value v) : value(v) {}

    bool isMaterialType() const
    {
        switch (value) {
        case ReferencedMaterial:
        case CustomMaterial:
        case DefaultMaterial:
            return true;
        default:
            return false;
        }
    }

    bool isLightmapType() const
    {
        switch (value) {
        case Lightmaps:
        case DefaultMaterial:
            return true;
        default:
            return false;
        }
    }

    bool isNodeType() const
    {
        switch (value) {
        case Node:
        case Layer:
        case Light:
        case Camera:
        case Model:
        case Text:
        case Path:
            return true;

        default:
            break;
        }
        return false;
    }

    bool isRenderableType() const
    {
        switch (value) {
        case Model:
        case Text:
        case Path:
            return true;
        default:
            break;
        }
        return false;
    }

    bool isLightCameraType() const
    {
        switch (value) {
        case Camera:
        case Light:
            return true;
        default:
            break;
        }
        return false;
    }
    const char *objectTypeName() const
    {
        switch (value) {
#define QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(type)                                                                    \
    case type:                                                                                                         \
        return #type;
            QDEMON_RENDER_ITERATE_GRAPH_OBJECT_TYPES
#undef QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE
        default:
            break;
        }
        Q_ASSERT(false);
        return "";
    }

    bool operator==(const QDemonGraphObjectType &other) const { return value == other.value; }
    bool operator!=(const QDemonGraphObjectType &other) const { return value != other.value; }
};
QT_END_NAMESPACE

#endif
