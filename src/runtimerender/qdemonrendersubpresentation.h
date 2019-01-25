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
#ifndef QDEMON_RENDER_SUBPRESENTATION_H
#define QDEMON_RENDER_SUBPRESENTATION_H

#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrendersubpresentationhelper.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

class CSubPresentationRenderer;

struct CSubPresentationPickQuery : public IGraphObjectPickQuery
{
    CSubPresentationRenderer &m_Renderer;

    CSubPresentationPickQuery(CSubPresentationRenderer &renderer)
        : m_Renderer(renderer)
    {
    }
    QDemonRenderPickResult Pick(const QVector2D &inMouseCoords,
                                const QVector2D &inViewportDimensions,
                                bool inPickEverything) override;
};

class CSubPresentationRenderer : public IOffscreenRenderer
{
public:
    IQDemonRenderContext *m_RenderContext;
    QSharedPointer<SPresentation> m_Presentation;
    SOffscreenRendererEnvironment m_LastRenderedEnvironment;
    CSubPresentationPickQuery m_PickQuery;
    QString m_OffscreenRendererType;

    CSubPresentationRenderer(IQDemonRenderContext *inRenderContext, QSharedPointer<SPresentation> inPresentation);

    SOffscreenRendererEnvironment GetDesiredEnvironment(QVector2D inPresScale) override;
    virtual SOffscreenRenderFlags
    NeedsRender(const SOffscreenRendererEnvironment &inEnvironment,
                QVector2D inPresScale,
                const SRenderInstanceId instanceId) override;
    void Render(const SOffscreenRendererEnvironment &inEnvironment,
                QDemonRenderContext & /*inRenderContext*/,
                QVector2D inPresScale,
                SScene::RenderClearCommand inClearBuffer,
                const SRenderInstanceId instanceId) override;
    void RenderWithClear(const SOffscreenRendererEnvironment &inEnvironment,
                         QDemonRenderContext &inRenderContext,
                         QVector2D inPresScale,
                         SScene::RenderClearCommand inClearBuffer,
                         QVector3D inClearColor,
                         const SRenderInstanceId instanceId) override;
    IGraphObjectPickQuery *GetGraphObjectPickQuery(const SRenderInstanceId) override { return &m_PickQuery; }
    bool Pick(const QVector2D & /*inMouseCoords*/,
              const QVector2D & /*inViewportDimensions*/,
              const SRenderInstanceId) override
    {
        return false;
    }
    void addCallback(IOffscreenRendererCallback *cb) override
    {
        Q_UNUSED(cb)
    }
    // Used for RTTI purposes so we can safely static-cast an offscreen renderer to a
    // CSubPresentationRenderer
    static const char *GetRendererName() { return "SubPresentation"; }
    QString GetOffscreenRendererType() override { return m_OffscreenRendererType; }

    QDemonRenderPickResult DoGraphQueryPick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, bool inPickEverything);
};
QT_END_NAMESPACE
#endif
