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
#ifndef QDEMON_RENDER_PRESENTATION_H
#define QDEMON_RENDER_PRESENTATION_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

class IQDemonRenderContext;
struct SScene;

struct RenderRotationValues
{
    enum Enum {
        NoRotation = 0,
        Clockwise90,
        Clockwise180,
        Clockwise270,
    };
};

struct SPresentation : public SGraphObject
{
    QVector2D m_PresentationDimensions;
    RenderRotationValues::Enum m_PresentationRotation;
    SScene *m_Scene;

    QString m_PresentationDirectory;

    SPresentation()
        : SGraphObject(GraphObjectTypes::Presentation)
        , m_PresentationDimensions(800, 400)
        , m_PresentationRotation(RenderRotationValues::NoRotation)
        , m_Scene(nullptr)
    {
    }

    SPresentation(float w, float h, const QString &presDir)
        : SGraphObject(GraphObjectTypes::Presentation)
        , m_PresentationDimensions(w, h)
        , m_PresentationRotation(RenderRotationValues::NoRotation)
        , m_Scene(nullptr)
        , m_PresentationDirectory(presDir)
    {
    }
    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SGraphObject::Remap(inRemapper);
        inRemapper.Remap(m_Scene);
        inRemapper.Remap(m_PresentationDirectory);
    }

    void Render(IQDemonRenderContext &inContext);
};

QT_END_NAMESPACE

#endif
