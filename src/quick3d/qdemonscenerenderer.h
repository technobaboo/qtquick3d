/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#ifndef QDEMONSCENERENDERER_H
#define QDEMONSCENERENDERER_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

#include <qsgtextureprovider.h>
#include <qsgrendernode.h>
#include <QSGSimpleTextureNode>

#include <QtQuick3d/qdemonview3d.h>

QT_BEGIN_NAMESPACE


class QDemonSceneManager;
class QDemonView3D;
struct QDemonRenderLayer;

class QDemonSceneRenderer
{
public:
    struct FramebufferObject {
        FramebufferObject(const QSize &s, const QDemonRef<QDemonRenderContext> &context);
        ~FramebufferObject();
        QSize size;
        QDemonRef<QDemonRenderContext> renderContext;
        QDemonRef<QDemonRenderFrameBuffer> fbo;
        QDemonRef<QDemonRenderTexture2D> color0;
        QDemonRef<QDemonRenderTexture2D> depthStencil;
    };

    QDemonSceneRenderer(QWindow *window);
    ~QDemonSceneRenderer();
protected:
    GLuint render();
    void render(const QRect &viewport, bool clearFirst = false);
    void synchronize(QDemonView3D *item, const QSize &size, bool useFBO = true);
    void update();
    void invalidateFramebufferObject();
    QSize surfaceSize() const { return m_surfaceSize; }

private:
    void updateLayerNode(QDemonView3D *view3D);
    void addNodeToLayer(QDemonRenderNode *node);
    void removeNodeFromLayer(QDemonRenderNode *node);
    QDemonSceneManager *m_sceneManager = nullptr;
    QDemonRenderLayer *m_layer = nullptr;
    QDemonRenderContextInterface::QDemonRenderContextInterfacePtr m_sgContext;
    QDemonRef<QDemonRenderContext> m_renderContext;
    QSize m_surfaceSize;
    void *data = nullptr;
    bool m_layerSizeIsDirty = true;
    QWindow *m_window = nullptr;
    FramebufferObject *m_fbo = nullptr;

    QDemonRenderNode *m_sceneRootNode = nullptr;
    QDemonRenderNode *m_referencedRootNode = nullptr;

    friend class SGFramebufferObjectNode;
    friend class QDemonSGRenderNode;
    friend class QDemonSGDirectRenderer;
    friend class QDemonView3D;
};

class QOpenGLVertexArrayObjectHelper;

class SGFramebufferObjectNode : public QSGTextureProvider, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    SGFramebufferObjectNode();
    ~SGFramebufferObjectNode() override;

    void scheduleRender();

    QSGTexture *texture() const override;

    void preprocess() override;

public Q_SLOTS:
    void render();

    void handleScreenChange();

public:
    QQuickWindow *window;
    QDemonSceneRenderer *renderer;
    QDemonView3D *quickFbo;

    bool renderPending;
    bool invalidatePending;

    qreal devicePixelRatio;
};

class QDemonSGRenderNode : public QSGRenderNode
{
public:

    StateFlags changedStates() const override;
    void render(const RenderState *state) override;
    void releaseResources() override;
    RenderingFlags flags() const override;
public:
    QQuickWindow *window = nullptr;
    QDemonSceneRenderer *renderer = nullptr;
};

class QDemonSGDirectRenderer : public QObject
{
    Q_OBJECT
public:
    enum QDemonSGDirectRendererMode {
        Underlay,
        Overlay
    };
    QDemonSGDirectRenderer(QDemonSceneRenderer *renderer, QQuickWindow *window, QDemonSGDirectRendererMode mode = Underlay);
    ~QDemonSGDirectRenderer();

    QDemonSceneRenderer *renderer() { return m_renderer; }
    void setViewport(const QRectF &viewport);

    void requestRender();

private Q_SLOTS:
    void render();

private:
    QDemonSceneRenderer *m_renderer = nullptr;
    QQuickWindow *m_window = nullptr;
    QDemonSGDirectRendererMode m_mode;
    QRectF m_viewport;
};

QT_END_NAMESPACE

#endif // QDEMONSCENERENDERER_H
