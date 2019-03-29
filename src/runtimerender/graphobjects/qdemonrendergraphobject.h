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
#ifndef QDEMON_RENDER_GRAPH_OBJECT_H
#define QDEMON_RENDER_GRAPH_OBJECT_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

// Types should be setup on construction.  Change the type
// at your own risk as the type is used for RTTI purposes.
struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderGraphObject
{
    enum class Type : quint8
    {
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
        Effect,
        CustomMaterial,
        RenderPlugin,
        ReferencedMaterial,
        Path,
        PathSubPath,
        Lightmaps,
        LastKnownGraphObjectType,
    };

    QAtomicInt ref;
    // Id's help debugging the object and are optionally set
    QString id;
    // Type is used for RTTI purposes down the road.
    Type type;

    QDemonRenderGraphObject(QDemonRenderGraphObject::Type inType) : type(inType) {}
    virtual ~QDemonRenderGraphObject();

    // If you change any detail of the scene graph, or even *breath* on a
    // scene graph object, you need to bump this binary version so at least
    // we know if we can load a file or not.
    static quint32 getSceneGraphBinaryVersion() { return 1; }

    inline bool isMaterialType() const Q_DECL_NOTHROW
    {
        return (type == Type::ReferencedMaterial || type == Type::CustomMaterial || type == Type::DefaultMaterial);
    }

    inline bool isLightmapType() const Q_DECL_NOTHROW
    {
        return (type == Type::Lightmaps || type == Type::DefaultMaterial);
    }

    inline bool isNodeType() const Q_DECL_NOTHROW
    {
        return (type == Type::Node ||
                type == Type::Layer ||
                type == Type::Light ||
                type == Type::Camera ||
                type == Type::Model ||
                type == Type::Path);
    }

    inline bool isRenderableType() const Q_DECL_NOTHROW
    {
        return (type == Type::Model || type == Type::Path);
    }

    inline bool isLightCameraType() const Q_DECL_NOTHROW
    {
        return (type == Type::Camera || type == Type::Light);
    }


    inline bool isMaterial() const Q_DECL_NOTHROW
    {
        return type == QDemonRenderGraphObject::Type::CustomMaterial ||
               type == QDemonRenderGraphObject::Type::DefaultMaterial ||
               type == QDemonRenderGraphObject::Type::ReferencedMaterial;
    }

    QDemonRenderGraphObject *nextMaterialSibling();
    void setNextMaterialSibling(QDemonRenderGraphObject *sibling);

};

QT_END_NAMESPACE

#endif
