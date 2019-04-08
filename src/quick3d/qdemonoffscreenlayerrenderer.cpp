#include "qdemonoffscreenlayerrenderer.h"
#include <qdemonrenderrenderlist.h>

QT_BEGIN_NAMESPACE

QDemonRenderPickResult QDemonOffscreenLayerRenderer::LayerPickQuery::pick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, bool inPickEverything)
{
    return m_renderer.doGraphQueryPick(inMouseCoords, inViewportDimensions, inPickEverything);
}

QDemonOffscreenLayerRenderer::QDemonOffscreenLayerRenderer(QDemonRef<QDemonRenderContextInterface> inRenderContext, QDemonRenderLayer *layer, const QSize &size)
    : m_renderContext(inRenderContext)
    , m_layer(layer)
    , m_size(size)
    , m_pickQuery(*this)
    , m_offscreenRendererType(QStringLiteral("OffscreenLayerRenderer"))
{

}

QDemonOffscreenLayerRenderer::~QDemonOffscreenLayerRenderer()
{

}

QString QDemonOffscreenLayerRenderer::getOffscreenRendererType()
{
    return m_offscreenRendererType;
}

QDemonOffscreenRendererEnvironment QDemonOffscreenLayerRenderer::getDesiredEnvironment(QVector2D inPresentationScaleFactor)
{
    bool hasTransparency = false;
    if (m_layer->background == QDemonRenderLayer::Background::Transparent)
        hasTransparency = true;
    QDemonRenderTextureFormat format = hasTransparency ? QDemonRenderTextureFormat::RGBA8
                                                              : QDemonRenderTextureFormat::RGB8;
    return QDemonOffscreenRendererEnvironment(m_size.width(),
                                              m_size.height(),
                                              format,
                                              QDemonOffscreenRendererDepthValues::Depth24Stencil8,
                                              false,
                                              QDemonRenderLayer::AAMode::NoAA);
}

QDemonOffscreenRenderFlags QDemonOffscreenLayerRenderer::needsRender(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                                                     QVector2D inPresentationScaleFactor,
                                                                     const QDemonRenderInstanceId instanceId)
{
    Q_UNUSED(inPresentationScaleFactor)
    bool hasTransparency = m_layer->background == QDemonRenderLayer::Background::Transparent;
    bool wasDirty = m_renderContext->renderer()->prepareLayerForRender(*m_layer, QSize(m_layer->m_width, m_layer->m_height), false, instanceId, true);
    return QDemonOffscreenRenderFlags(hasTransparency, wasDirty);
}

void QDemonOffscreenLayerRenderer::render(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                          QDemonRenderContext &inRenderContext,
                                          QVector2D inPresentationScaleFactor,
                                          QDemonRenderScene::RenderClearCommand inColorBufferNeedsClear,
                                          const QDemonRenderInstanceId instanceId)
{
    Q_UNUSED(inRenderContext)
    Q_UNUSED(inPresentationScaleFactor)
    Q_UNUSED(inColorBufferNeedsClear)
    m_renderContext->setInSubPresentation(true);
    m_renderContext->setPresentationDimensions(QSize(inEnvironment.width, inEnvironment.height));
    m_renderContext->renderer()->renderLayer(*m_layer, QSize(inEnvironment.width, inEnvironment.height),
                                             true, QVector3D(), false, instanceId);

}

void QDemonOffscreenLayerRenderer::renderWithClear(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                                   QDemonRenderContext &inRenderContext,
                                                   QVector2D inPresentationScaleFactor,
                                                   QDemonRenderScene::RenderClearCommand inColorBufferNeedsClear,
                                                   QVector3D inclearColor,
                                                   const QDemonRenderInstanceId instanceId)
{
    Q_UNUSED(inRenderContext)
    Q_UNUSED(inPresentationScaleFactor)
    m_renderContext->setInSubPresentation(true);
    m_renderContext->setPresentationDimensions(QSize(inEnvironment.width, inEnvironment.height));
    bool clear = false;
    if (inColorBufferNeedsClear == QDemonRenderScene::RenderClearCommand::AlwaysClear)
        clear = true;
    m_renderContext->renderer()->renderLayer(*m_layer, QSize(inEnvironment.width, inEnvironment.height),
                                             clear, inclearColor, false, instanceId);
}

QDemonGraphObjectPickQueryInterface *QDemonOffscreenLayerRenderer::getGraphObjectPickQuery(const QDemonRenderInstanceId instanceId)
{
    Q_UNUSED(instanceId)
    return &m_pickQuery;
}

bool QDemonOffscreenLayerRenderer::pick(const QVector2D &inMouseCoords,
                                        const QVector2D &inViewportDimensions,
                                        const QDemonRenderInstanceId instanceId)
{
    Q_UNUSED(inMouseCoords)
    Q_UNUSED(inViewportDimensions)
    Q_UNUSED(instanceId)
    // ### why do we return false here...
    return false;
}

QDemonRenderPickResult QDemonOffscreenLayerRenderer::doGraphQueryPick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, bool inPickEverything)
{
    QDemonRenderPickResult thePickResult;

    thePickResult = m_renderContext->renderer()->pick(*m_layer, inViewportDimensions, inMouseCoords, true, inPickEverything);

    return thePickResult;
}

QT_END_NAMESPACE
