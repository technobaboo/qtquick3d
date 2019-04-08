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
    QDemonSceneRenderer(QWindow *window);
    ~QDemonSceneRenderer();
protected:
    GLuint render();
    void synchronize(QDemonView3D *item, const QSize &size);
    void update();
    void invalidateFramebufferObject();
    QSize surfaceSize() const { return m_surfaceSize; }

private:
    void updateLayerNode(QDemonView3D *view3D);
    void addNodeToLayer(QDemonRenderNode *node);
    void removeNodeFromLayer(QDemonRenderNode *node);
    QDemonSceneManager *m_sceneManager = nullptr;
    QDemonRenderLayer *m_layer = nullptr;
    QDemonRef<QDemonRenderContextInterface> m_sgContext;
    QDemonRef<QDemonRenderContext> m_renderContext;
    QSize m_surfaceSize;
    void *data = nullptr;
    bool m_layerSizeIsDirty = true;
    QOpenGLContext *m_openGLContext = nullptr;
    QWindow *m_window;

    QDemonRenderNode *m_sceneRootNode = nullptr;
    QDemonRenderNode *m_referencedRootNode = nullptr;

    friend class SGFramebufferObjectNode;
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

private:
    void resetOpenGLState();
    QOpenGLVertexArrayObjectHelper *m_vaoHelper = nullptr;
};

QT_END_NAMESPACE

#endif // QDEMONSCENERENDERER_H
