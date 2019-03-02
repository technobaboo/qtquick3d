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

class QDemonSubPresentationRenderer;

struct CSubPresentationPickQuery : public QDemonGraphObjectPickQueryInterface
{
    QDemonSubPresentationRenderer &m_renderer;

    CSubPresentationPickQuery(QDemonSubPresentationRenderer &renderer)
        : m_renderer(renderer)
    {
    }
    QDemonRenderPickResult pick(const QVector2D &inMouseCoords,
                                const QVector2D &inViewportDimensions,
                                bool inPickEverything) override;
};

class QDemonSubPresentationRenderer : public QDemonOffscreenRendererInterface
{
public:
    QDemonRenderContextInterface *m_renderContext;
    QDemonRef<QDemonPresentation> m_presentation;
    QDemonOffscreenRendererEnvironment m_lastRenderedEnvironment;
    CSubPresentationPickQuery m_pickQuery;
    QString m_offscreenRendererType;

    QDemonSubPresentationRenderer(QDemonRenderContextInterface *inRenderContext, QDemonRef<QDemonPresentation> inPresentation);

    QDemonOffscreenRendererEnvironment getDesiredEnvironment(QVector2D inPresScale) override;
    virtual QDemonOffscreenRenderFlags needsRender(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                                   QVector2D inPresScale,
                                                   const QDemonRenderInstanceId instanceId) override;
    void render(const QDemonOffscreenRendererEnvironment &inEnvironment,
                QDemonRenderContext & /*inRenderContext*/,
                QVector2D inPresScale,
                QDemonRenderScene::RenderClearCommand inClearBuffer,
                const QDemonRenderInstanceId instanceId) override;
    void renderWithClear(const QDemonOffscreenRendererEnvironment &inEnvironment,
                         QDemonRenderContext &inRenderContext,
                         QVector2D inPresScale,
                         QDemonRenderScene::RenderClearCommand inClearBuffer,
                         QVector3D inClearColor,
                         const QDemonRenderInstanceId instanceId) override;
    QDemonGraphObjectPickQueryInterface *getGraphObjectPickQuery(const QDemonRenderInstanceId) override { return &m_pickQuery; }
    bool pick(const QVector2D & /*inMouseCoords*/,
              const QVector2D & /*inViewportDimensions*/,
              const QDemonRenderInstanceId) override
    {
        return false;
    }
    void addCallback(QDemonOffscreenRendererCallbackInterface *cb) override
    {
        Q_UNUSED(cb)
    }
    // Used for RTTI purposes so we can safely static-cast an offscreen renderer to a
    // CSubPresentationRenderer
    static const char *getRendererName() { return "SubPresentation"; }
    QString getOffscreenRendererType() override { return m_offscreenRendererType; }

    QDemonRenderPickResult doGraphQueryPick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, bool inPickEverything);
};
QT_END_NAMESPACE
#endif
