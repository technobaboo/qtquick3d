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
#ifndef QDEMON_RENDER_SCENE_H
#define QDEMON_RENDER_SCENE_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QVector3D.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

QT_BEGIN_NAMESPACE
struct SLayer;
struct SPresentation;
typedef void *SRenderInstanceId;

struct SScene : public SGraphObject
{
    SPresentation *m_Presentation;
    SLayer *m_FirstChild;
    QVector3D m_ClearColor;
    bool m_UseClearColor;
    bool m_Dirty;

    enum RenderClearCommand {
        ClearIsOptional = 0,
        DoNotClear = 1,
        AlwaysClear = 2,
    };

    SScene();

    void AddChild(SLayer &inLayer);
    SLayer *GetLastChild();

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SGraphObject::Remap(inRemapper);
        inRemapper.Remap(m_Presentation);
        inRemapper.Remap(m_FirstChild);
    }
    // returns true if any of the layers were dirty or if this object was dirty
    bool PrepareForRender(const QVector2D &inViewportDimensions, IQt3DSRenderContext &inContext,
                          const SRenderInstanceId id = nullptr);
    void Render(const QVector2D &inViewportDimensions, IQt3DSRenderContext &inContext,
                RenderClearCommand command = ClearIsOptional,
                const SRenderInstanceId id = nullptr);
    void RenderWithClear(const QVector2D &inViewportDimensions, IQt3DSRenderContext &inContext,
                         RenderClearCommand inClearColorBuffer,
                         QVector3D inclearColor, const SRenderInstanceId id = nullptr);
};
QT_END_NAMESPACE

#endif
