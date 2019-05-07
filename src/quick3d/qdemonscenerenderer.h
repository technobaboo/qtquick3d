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
        FramebufferObject(const QSize &s, QDemonRef<QDemonRenderContext> context);
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
