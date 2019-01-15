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

#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

SScene::SScene()
    : SGraphObject(GraphObjectTypes::Scene)
    , m_Presentation(nullptr)
    , m_FirstChild(nullptr)
    , m_ClearColor(0, 0, 0)
    , m_UseClearColor(true)
    , m_Dirty(true)
{
}

void SScene::AddChild(SLayer &inLayer)
{
    if (m_FirstChild == nullptr)
        m_FirstChild = &inLayer;
    else
        GetLastChild()->m_NextSibling = &inLayer;
    inLayer.m_Scene = this;
}

SLayer *SScene::GetLastChild()
{
    // empty loop intentional
    SLayer *child;
    for (child = m_FirstChild; child && child->m_NextSibling;
         child = (SLayer *)child->m_NextSibling) {
    }

    return child;
}

bool SScene::PrepareForRender(const QVector2D &inViewportDimensions, IQDemonRenderContext &inContext,
                              const SRenderInstanceId id)
{
    // We need to iterate through the layers in reverse order and ask them to render.
    bool wasDirty = m_Dirty;
    m_Dirty = false;
    if (m_FirstChild) {
        wasDirty |=
            inContext.GetRenderer()->PrepareLayerForRender(*m_FirstChild, inViewportDimensions,
                                                           true, id);
    }
    return wasDirty;
}

void SScene::Render(const QVector2D &inViewportDimensions, IQDemonRenderContext &inContext,
                    RenderClearCommand inClearColorBuffer, const SRenderInstanceId id)
{
    if ((inClearColorBuffer == SScene::ClearIsOptional && m_UseClearColor)
        || inClearColorBuffer == SScene::AlwaysClear) {
        float clearColorAlpha
                = inContext.IsInSubPresentation() && !m_UseClearColor ? 0.0f : 1.0f;
        QVector4D clearColor(0.0f, 0.0f, 0.0f, clearColorAlpha);
        if (m_UseClearColor) {
            clearColor.setX(m_ClearColor.x());
            clearColor.setY(m_ClearColor.y());
            clearColor.setZ(m_ClearColor.z());
        }
        // Maybe clear and reset to previous clear color after we leave.
        QDemonRenderContextScopedProperty<QVector4D> __clearColor(
            *inContext.GetRenderContext(), &QDemonRenderContext::GetClearColor,
                    &QDemonRenderContext::SetClearColor, clearColor);
        inContext.GetRenderContext()->Clear(QDemonRenderClearValues::Color);
    }
    if (m_FirstChild) {
        inContext.GetRenderer()->RenderLayer(*m_FirstChild, inViewportDimensions, m_UseClearColor,
                                            m_ClearColor, true, id);
    }
}
void SScene::RenderWithClear(const QVector2D &inViewportDimensions,
                             IQDemonRenderContext &inContext,
                             RenderClearCommand inClearColorBuffer,
                             QVector3D inClearColor,
                             const SRenderInstanceId id)
{
    // If this scene is not using clear color, we set the color
    // to background color from parent layer. This allows
    // fully transparent subpresentations (both scene and layer(s) transparent)
    // to inherit color from the layer that contains them.
    if (!m_UseClearColor) {
        m_ClearColor = inClearColor;
        m_UseClearColor = true;
    }
    Render(inViewportDimensions, inContext, inClearColorBuffer, id);
}

QT_END_NAMESPACE
