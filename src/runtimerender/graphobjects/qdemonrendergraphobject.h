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
#ifndef QDEMON_RENDER_GRAPH_OBJECT_H
#define QDEMON_RENDER_GRAPH_OBJECT_H

#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemonRuntimeRender/qdemonrendertaggedpointer.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobjecttypes.h>

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

// Types should be setup on construction.  Change the type
// at your own risk as the type is used for RTTI purposes.
struct Q_DEMONRUNTIMERENDER_EXPORT SGraphObject
{
    // Id's help debugging the object and are optionally set
    QString m_Id;
    // Type is used for RTTI purposes down the road.
    GraphObjectTypes::Enum m_Type;
    STaggedPointer m_UserData;

    SGraphObject(GraphObjectTypes::Enum inType)
        : m_Type(inType)
    {
    }
    SGraphObject(const SGraphObject &inCloningObject)
        : m_Id(inCloningObject.m_Id)
        , m_Type(inCloningObject.m_Type)
    {
    }

    // If you change any detail of the scene graph, or even *breath* on a
    // scene graph object, you need to bump this binary version so at least
    // we know if we can load a file or not.
    static quint32 GetSceneGraphBinaryVersion() { return 1; }

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        inRemapper.Remap(m_Id);
        inRemapper.NullPtr(m_UserData.m_UserData);
    }
};

QT_END_NAMESPACE

#endif
