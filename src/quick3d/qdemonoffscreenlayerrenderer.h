#ifndef QDEMONOFFSCREENLAYERRENDERER_H
#define QDEMONOFFSCREENLAYERRENDERER_H

#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>

QT_BEGIN_NAMESPACE

class QDemonOffscreenLayerRenderer : public QDemonOffscreenRendererInterface
{
public:
    struct LayerPickQuery : public QDemonGraphObjectPickQueryInterface
    {
        QDemonOffscreenLayerRenderer &m_renderer;

        LayerPickQuery(QDemonOffscreenLayerRenderer &renderer)
            : m_renderer(renderer) {}
        QDemonRenderPickResult pick(const QVector2D &inMouseCoords,
                                    const QVector2D &inViewportDimensions,
                                    bool inPickEverything) override;
    };
    QDemonOffscreenLayerRenderer(QDemonRef<QDemonRenderContextInterface> inRenderContext, QDemonRenderLayer *layer, const QSize &size);
    ~QDemonOffscreenLayerRenderer();

    QString getOffscreenRendererType() override;
    QDemonOffscreenRendererEnvironment getDesiredEnvironment(QVector2D inPresentationScaleFactor) override;
    QDemonOffscreenRenderFlags needsRender(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                           QVector2D inPresentationScaleFactor,
                                           const QDemonRenderInstanceId instanceId) override;
    void render(const QDemonOffscreenRendererEnvironment &inEnvironment,
                QDemonRenderContext &inRenderContext,
                QVector2D inPresentationScaleFactor,
                QDemonRenderScene::RenderClearCommand inColorBufferNeedsClear,
                const QDemonRenderInstanceId instanceId) override;
    void renderWithClear(const QDemonOffscreenRendererEnvironment &inEnvironment,
                         QDemonRenderContext &inRenderContext,
                         QVector2D inPresentationScaleFactor,
                         QDemonRenderScene::RenderClearCommand inColorBufferNeedsClear,
                         QVector3D inclearColor,
                         const QDemonRenderInstanceId instanceId) override;
    QDemonGraphObjectPickQueryInterface *getGraphObjectPickQuery(const QDemonRenderInstanceId instanceId) override;
    bool pick(const QVector2D &inMouseCoords,
              const QVector2D &inViewportDimensions,
              const QDemonRenderInstanceId instanceId) override;

    QDemonRenderPickResult doGraphQueryPick(const QVector2D &inMouseCoords,
                                            const QVector2D &inViewportDimensions,
                                            bool inPickEverything);

private:
    QDemonRef<QDemonRenderContextInterface> m_renderContext;
    QDemonRenderLayer *m_layer;
    QSize m_size;
    QDemonOffscreenRendererEnvironment m_lastRenderedEnvironment;
    LayerPickQuery m_pickQuery;
    QString m_offscreenRendererType;
};

QT_END_NAMESPACE

#endif // QDEMONOFFSCREENLAYERRENDERER_H
