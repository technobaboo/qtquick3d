#include "qdemonscenerenderer.h"
#include "qdemonsceneenvironment.h"
#include "qdemonobject_p.h"
#include "qdemonnode.h"
#include "qdemonscenemanager_p.h"
#include "qdemonimage.h"
#include "qdemoncamera.h"

#include <private/qopenglvertexarrayobject_p.h>

#include <QtDemonRender/QDemonRenderFrameBuffer>
#include <QtDemonRuntimeRender/QDemonRenderLayer>
#include <QtDemonRuntimeRender/QDemonOffscreenRendererKey>
#include <QtQuick/QQuickWindow>

QT_BEGIN_NAMESPACE

SGFramebufferObjectNode::SGFramebufferObjectNode()
    : window(nullptr)
    , renderer(nullptr)
    , renderPending(true)
    , invalidatePending(false)
    , devicePixelRatio(1)
{
    qsgnode_set_description(this, QStringLiteral("fbonode"));
    setFlag(QSGNode::UsePreprocess, true);
}

SGFramebufferObjectNode::~SGFramebufferObjectNode()
{
    delete renderer;
    delete texture();
}

void SGFramebufferObjectNode::scheduleRender()
{
    renderPending = true;
    markDirty(DirtyMaterial);
}

QSGTexture *SGFramebufferObjectNode::texture() const
{
    return QSGSimpleTextureNode::texture();
}

void SGFramebufferObjectNode::preprocess()
{
    render();
}

void SGFramebufferObjectNode::render()
{
    if (renderPending) {
        renderPending = false;
        GLuint textureId = renderer->render();

        window->resetOpenGLState();

        if (texture() && (GLuint(texture()->textureId()) != textureId || texture()->textureSize() != renderer->surfaceSize())) {
            delete texture();
            setTexture(window->createTextureFromId(textureId, renderer->surfaceSize(), QQuickWindow::TextureHasAlphaChannel));
        }
        if (!texture())
            setTexture(window->createTextureFromId(textureId, renderer->surfaceSize(), QQuickWindow::TextureHasAlphaChannel));


        markDirty(QSGNode::DirtyMaterial);
        emit textureChanged();
    }
}

void SGFramebufferObjectNode::handleScreenChange()
{
    if (window->effectiveDevicePixelRatio() != devicePixelRatio) {
        renderer->invalidateFramebufferObject();
        quickFbo->update();
    }
}

QDemonSceneRenderer::QDemonSceneRenderer(QWindow *window)
    : m_window(window)
{
    QOpenGLContext *openGLContext = QOpenGLContext::currentContext();

    // There is only one Render context per window, so check if one exists for this window already
    auto renderContextInterface = QDemonRenderContextInterface::getRenderContextInterface(quintptr(window));
    if (!renderContextInterface.isNull()) {
        m_sgContext = renderContextInterface;
        m_renderContext = renderContextInterface->renderContext();
    }

    // If there was no render context, then set it up for this window
    if (m_renderContext.isNull())
        m_renderContext = QDemonRenderContext::createGl(openGLContext->format());
    if (m_sgContext.isNull())
        m_sgContext = QDemonRenderContextInterface::getRenderContextInterface(m_renderContext, QString::fromLatin1("./"), quintptr(window));
}

QDemonSceneRenderer::~QDemonSceneRenderer()
{
    delete m_layer;
}

GLuint QDemonSceneRenderer::render()
{
    if (!m_layer)
        return 0;

    m_sgContext->beginFrame();
    m_renderContext->setRenderTarget(m_fbo->fbo);
    m_sgContext->renderList()->setViewport(QRect(0, 0, m_surfaceSize.width(), m_surfaceSize.height()));
    m_sgContext->setWindowDimensions(m_surfaceSize);

    m_sgContext->renderer()->prepareLayerForRender(*m_layer, m_surfaceSize, false, nullptr, true);
    m_sgContext->runRenderTasks();
    m_sgContext->renderer()->renderLayer(*m_layer, m_surfaceSize, true, QVector3D(0, 0, 0), false);
    m_sgContext->endFrame();

    return HandleToID_cast(GLuint, size_t, m_fbo->color0->handle());
}

void QDemonSceneRenderer::render(const QRect &viewport, bool clearFirst)
{
    if (!m_layer)
        return;

    m_sgContext->beginFrame();

    // set render target to be current window (default)
    m_renderContext->setRenderTarget(nullptr);

    // set viewport
    m_sgContext->renderList()->setViewport(viewport);
    m_sgContext->renderList()->setScissorRect(viewport);
    m_sgContext->setWindowDimensions(m_surfaceSize);

    m_sgContext->renderer()->prepareLayerForRender(*m_layer, m_surfaceSize, false, nullptr, true);
    m_sgContext->runRenderTasks();
    m_sgContext->renderer()->renderLayer(*m_layer, m_surfaceSize, clearFirst, QVector3D(0, 0, 0), false);
    m_sgContext->endFrame();

}

void QDemonSceneRenderer::synchronize(QDemonView3D *item, const QSize &size, bool useFBO)
{
    if (!item)
        return;

    if (m_surfaceSize != size) {
        m_layerSizeIsDirty = true;
        m_surfaceSize = size;
    }

    auto view3D = static_cast<QDemonView3D*>(item);
    m_sceneManager = QDemonObjectPrivate::get(view3D->scene())->sceneManager;
    m_sceneManager->updateDirtyNodes();

    if (view3D->referencedScene()) {
        QDemonObjectPrivate::get(view3D->referencedScene())->sceneManager->updateDirtyNodes();
    }

    // Generate layer node
    if (!m_layer)
        m_layer = new QDemonRenderLayer();

    // Update the layer node properties
    updateLayerNode(view3D);

    // Set the root item for the scene to the layer
    auto rootNode = static_cast<QDemonRenderNode*>(QDemonObjectPrivate::get(view3D->scene())->spatialNode);
    if (rootNode != m_sceneRootNode) {
        if (m_sceneRootNode)
            removeNodeFromLayer(m_sceneRootNode);

        if (rootNode)
            addNodeToLayer(rootNode);

        m_sceneRootNode = rootNode;
    }

    // Add the referenced scene root node to the layer as well if available
    QDemonRenderNode* referencedRootNode = nullptr;
    if (view3D->referencedScene())
        referencedRootNode = static_cast<QDemonRenderNode*>(QDemonObjectPrivate::get(view3D->referencedScene())->spatialNode);
    if (referencedRootNode != m_referencedRootNode) {
        if (m_referencedRootNode)
            removeNodeFromLayer(m_referencedRootNode);

        if (referencedRootNode)
            addNodeToLayer(referencedRootNode);

        m_referencedRootNode = referencedRootNode;
    }

    if (useFBO) {
        if (!m_fbo || m_layerSizeIsDirty) {
            if (m_fbo)
                delete m_fbo;

            m_fbo = new FramebufferObject(m_surfaceSize, m_renderContext);
            m_layerSizeIsDirty = false;
        }
    }

}

void QDemonSceneRenderer::update()
{
    if (data)
        static_cast<SGFramebufferObjectNode *>(data)->scheduleRender();
}

void QDemonSceneRenderer::invalidateFramebufferObject()
{
    if (data)
        static_cast<SGFramebufferObjectNode *>(data)->invalidatePending = true;
}

void QDemonSceneRenderer::updateLayerNode(QDemonView3D *view3D)
{
    QDemonRenderLayer *layerNode = m_layer;
    layerNode->progressiveAAMode = QDemonRenderLayer::AAMode(view3D->environment()->progressiveAAMode());
    layerNode->multisampleAAMode = QDemonRenderLayer::AAMode(view3D->environment()->multisampleAAMode());
    layerNode->temporalAAEnabled = view3D->environment()->temporalAAEnabled();

    layerNode->background = QDemonRenderLayer::Background(view3D->environment()->backgroundMode());
    layerNode->clearColor = QVector3D(float(view3D->environment()->clearColor().redF()),
                                      float(view3D->environment()->clearColor().greenF()),
                                      float(view3D->environment()->clearColor().blueF()));
    layerNode->blendType = QDemonRenderLayer::BlendMode(view3D->environment()->blendType());

    layerNode->m_width = 100.f;
    layerNode->m_height = 100.f;
    layerNode->widthUnits = QDemonRenderLayer::UnitType::Percent;
    layerNode->heightUnits = QDemonRenderLayer::UnitType::Percent;

    layerNode->aoStrength = view3D->environment()->aoStrength();
    layerNode->aoDistance = view3D->environment()->aoDistance();
    layerNode->aoSoftness = view3D->environment()->aoSoftness();
    layerNode->aoBias = view3D->environment()->aoBias();
    layerNode->aoSamplerate = view3D->environment()->aoSampleRate();
    layerNode->aoDither = view3D->environment()->aoDither();


    layerNode->shadowStrength = view3D->environment()->shadowStrength();
    layerNode->shadowDist = view3D->environment()->shadowDistance();
    layerNode->shadowSoftness = view3D->environment()->shadowSoftness();
    layerNode->shadowBias = view3D->environment()->shadowBias();

    // ### These images will not be registered anywhere
    if (view3D->environment()->lightProbe())
        layerNode->lightProbe = view3D->environment()->lightProbe()->getRenderImage();
    else
        layerNode->lightProbe = nullptr;

    layerNode->probeBright = view3D->environment()->probeBrightness();
    layerNode->fastIbl = view3D->environment()->fastIBL();
    layerNode->probeHorizon = view3D->environment()->probeHorizon();
    layerNode->probeFov = view3D->environment()->probeFieldOfView();


    if (view3D->environment()->lightProbe2())
        layerNode->lightProbe2 = view3D->environment()->lightProbe()->getRenderImage();
    else
        layerNode->lightProbe2 = nullptr;

    if (view3D->camera()) {
        layerNode->activeCamera = view3D->camera()->getCameraNode();
    }


    layerNode->probe2Fade = view3D->environment()->probe2Fade();
    layerNode->probe2Window = view3D->environment()->probe2Window();
    layerNode->probe2Pos = view3D->environment()->probe2Postion();

    if (view3D->environment()->isDepthTestDisabled())
        layerNode->flags.setFlag(QDemonRenderNode::Flag::LayerEnableDepthTest, false);
    else
        layerNode->flags.setFlag(QDemonRenderNode::Flag::LayerEnableDepthTest, true);

    if (view3D->environment()->isDepthPrePassDisabled())
        layerNode->flags.setFlag(QDemonRenderNode::Flag::LayerEnableDepthPrePass, false);
    else
        layerNode->flags.setFlag(QDemonRenderNode::Flag::LayerEnableDepthPrePass, true);

    layerNode->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformNotDirty);
}

void QDemonSceneRenderer::removeNodeFromLayer(QDemonRenderNode *node)
{
    if (!m_layer)
        return;

    m_layer->removeChild(*node);
}

void QDemonSceneRenderer::addNodeToLayer(QDemonRenderNode *node)
{
    if (!m_layer)
        return;

    m_layer->addChild(*node);
}

QDemonSceneRenderer::FramebufferObject::FramebufferObject(const QSize &s, QDemonRef<QDemonRenderContext> context)
{
    size = s;
    renderContext = context;

    depthStencil = new QDemonRenderTexture2D(renderContext);
    depthStencil->setTextureData(QDemonByteView(), 0, size.width(), size.height(), QDemonRenderTextureFormat::Depth24Stencil8);
    color0 = new QDemonRenderTexture2D(renderContext);
    color0->setTextureData(QDemonByteView(), 0, size.width(), size.height(), QDemonRenderTextureFormat::RGBA8);
    fbo = new QDemonRenderFrameBuffer(renderContext);
    fbo->attach(QDemonRenderFrameBufferAttachment::Color0, color0);
    fbo->attach(QDemonRenderFrameBufferAttachment::DepthStencil, depthStencil);
}

QDemonSceneRenderer::FramebufferObject::~FramebufferObject()
{

}

QSGRenderNode::StateFlags QDemonSGRenderNode::changedStates() const
{
    return BlendState | StencilState | DepthState | ScissorState | ColorState | CullState | ViewportState | RenderTargetState;
}

void QDemonSGRenderNode::render(const QSGRenderNode::RenderState *state)
{
    // calculate viewport
    QRect viewport = matrix()->mapRect(QRect(QPoint(0, 0), renderer->surfaceSize()));

    // render
    renderer->render(viewport);
    markDirty(QSGNode::DirtyMaterial);
}

void QDemonSGRenderNode::releaseResources()
{
}

QSGRenderNode::RenderingFlags QDemonSGRenderNode::flags() const
{
    return QSGRenderNode::RenderingFlags();
}

QDemonSGDirectRenderer::QDemonSGDirectRenderer(QDemonSceneRenderer *renderer, QQuickWindow *window, QDemonSGDirectRenderer::QDemonSGDirectRendererMode mode)
    : m_renderer(renderer)
    , m_window(window)
    , m_mode(mode)
{
    if (mode == Underlay)
        connect(window, &QQuickWindow::beforeRendering, this, &QDemonSGDirectRenderer::render, Qt::DirectConnection);
    else
        connect(window, &QQuickWindow::afterRendering, this, &QDemonSGDirectRenderer::render, Qt::DirectConnection);
}

QDemonSGDirectRenderer::~QDemonSGDirectRenderer()
{
    delete m_renderer;
}

void QDemonSGDirectRenderer::setViewport(const QRectF &viewport)
{
    m_viewport = viewport;
}

void QDemonSGDirectRenderer::requestRender()
{
    m_window->update();
}

namespace {
QRect convertQtRectToGLViewport(const QRectF &rect, const QSize surfaceSize) {
    //
    const int x = int(rect.x());
    const int y = surfaceSize.height() - (int(rect.y()) + int(rect.height()));
    const int width = int(rect.width());
    const int height = int(rect.height());
    return QRect(x, y, width, height);
}
}

void QDemonSGDirectRenderer::render()
{
    const QRect glViewport = convertQtRectToGLViewport(m_viewport, m_window->size() * m_window->devicePixelRatio());
    m_renderer->render(glViewport, false);
    m_window->resetOpenGLState();
}

QT_END_NAMESPACE
