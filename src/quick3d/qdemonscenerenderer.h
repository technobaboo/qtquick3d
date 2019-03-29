#ifndef QDEMONSCENERENDERER_H
#define QDEMONSCENERENDERER_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

#include <qsgtextureprovider.h>
#include <QSGSimpleTextureNode>

#include "qdemonview3d.h"

QT_BEGIN_NAMESPACE


class QDemonSceneManager;
class QDemonView3D;
struct QDemonRenderLayer;

class QDemonSceneRenderer
{
public:
    struct FrambufferObject {
        FrambufferObject(const QSize &s, QDemonRef<QDemonRenderContext> context);
        ~FrambufferObject();
        QSize size;
        QDemonRef<QDemonRenderContext> renderContext;
        QDemonRef<QDemonRenderFrameBuffer> fbo;
        QDemonRef<QDemonRenderTexture2D> color0;
        QDemonRef<QDemonRenderTexture2D> depthStencil;
    };

    QDemonSceneRenderer();
protected:
    void render();
    QDemonSceneRenderer::FrambufferObject *createFramebufferObject(const QSize &size);
    void synchronize(QDemonView3D *item);
    void update();
    void invalidateFramebufferObject();

private:
    void updateLayerNode(QDemonView3D *view3D);
    QDemonSceneManager *m_sceneManager = nullptr;
    QDemonRenderLayer *m_layer = nullptr;
    QDemonRef<QDemonRenderContextInterface> m_sgContext;
    QDemonRef<QDemonRenderContext> m_renderContext;
    QSize m_surfaceSize;
    void *data = nullptr;
    friend class SGFramebufferObjectNode;
    friend class QDemonView3D;
};

class SGFramebufferObjectNode : public QSGTextureProvider, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    SGFramebufferObjectNode();
    ~SGFramebufferObjectNode() override;

    void scheduleRender();

    QSGTexture *texture() const override;

public Q_SLOTS:
    void render();

    void handleScreenChange();

public:
    QQuickWindow *window;
    QDemonSceneRenderer::FrambufferObject *fbo;
    QDemonSceneRenderer *renderer;
    QDemonView3D *quickFbo;

    bool renderPending;
    bool invalidatePending;

    qreal devicePixelRatio;
};

QT_END_NAMESPACE

#endif // QDEMONSCENERENDERER_H
