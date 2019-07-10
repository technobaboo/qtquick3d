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

#include <QtQuick3D/QQuick3DViewport>

QT_BEGIN_NAMESPACE


class QQuick3DSceneManager;
class QQuick3DViewport;
struct QDemonRenderLayer;

class QQuick3DSceneRenderer
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

    QQuick3DSceneRenderer(QWindow *window);
    ~QQuick3DSceneRenderer();
protected:
    GLuint render();
    void render(const QRect &viewport, bool clearFirst = false);
    void synchronize(QQuick3DViewport *item, const QSize &size, bool useFBO = true);
    void update();
    void invalidateFramebufferObject();
    QSize surfaceSize() const { return m_surfaceSize; }

private:
    void updateLayerNode(QQuick3DViewport *view3D);
    void addNodeToLayer(QDemonRenderNode *node);
    void removeNodeFromLayer(QDemonRenderNode *node);
    QQuick3DSceneManager *m_sceneManager = nullptr;
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
    friend class QQuick3DSGRenderNode;
    friend class QQuick3DSGDirectRenderer;
    friend class QQuick3DViewport;
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
    QQuick3DSceneRenderer *renderer;
    QQuick3DViewport *quickFbo;

    bool renderPending;
    bool invalidatePending;

    qreal devicePixelRatio;
};

class QQuick3DSGRenderNode : public QSGRenderNode
{
public:

    StateFlags changedStates() const override;
    void render(const RenderState *state) override;
    void releaseResources() override;
    RenderingFlags flags() const override;
public:
    QQuickWindow *window = nullptr;
    QQuick3DSceneRenderer *renderer = nullptr;
};

class QQuick3DSGDirectRenderer : public QObject
{
    Q_OBJECT
public:
    enum QQuick3DSGDirectRendererMode {
        Underlay,
        Overlay
    };
    QQuick3DSGDirectRenderer(QQuick3DSceneRenderer *renderer, QQuickWindow *window, QQuick3DSGDirectRendererMode mode = Underlay);
    ~QQuick3DSGDirectRenderer();

    QQuick3DSceneRenderer *renderer() { return m_renderer; }
    void setViewport(const QRectF &viewport);

    void requestRender();

private Q_SLOTS:
    void render();

private:
    QQuick3DSceneRenderer *m_renderer = nullptr;
    QQuickWindow *m_window = nullptr;
    QQuick3DSGDirectRendererMode m_mode;
    QRectF m_viewport;
};

QT_END_NAMESPACE

#endif // QDEMONSCENERENDERER_H
