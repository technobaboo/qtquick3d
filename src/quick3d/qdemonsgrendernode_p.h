#ifndef QDEMONRENDERNODE_P_H
#define QDEMONRENDERNODE_P_H

#include <QtQuick/QSGRenderNode>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

QT_BEGIN_NAMESPACE

class QDemonSceneManager;
struct QDemonRenderLayer;
class QDemonSGRenderNode : public QSGRenderNode
{
public:
    QDemonSGRenderNode(QDemonSceneManager *sceneRenderer);

    StateFlags changedStates() const override;
    void render(const RenderState *state) override;
    RenderingFlags flags() const override;

    void setRenderLayer(QDemonRenderLayer *layer);

private:
    QDemonSceneManager *m_sceneRenderer;
    QDemonRenderLayer *m_layer;
    QDemonRef<QDemonRenderContextInterface> m_sgContext;
    QDemonRef<QDemonRenderContext> m_renderContext;
};

QT_END_NAMESPACE

#endif // QDEMONRENDERNODE_P_H
