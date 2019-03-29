#include "qdemonscenerenderer.h"
#include "qdemonsceneenvironment.h"
#include "qdemonobject_p.h"
#include "qdemonnode.h"
#include "qdemonscenemanager_p.h"
#include "qdemonimage.h"

#include <QtDemonRuntimeRender/QDemonRenderLayer>
#include <QtQuick/QQuickWindow>

QT_BEGIN_NAMESPACE

SGFramebufferObjectNode::SGFramebufferObjectNode()
    : window(nullptr)
    , fbo(nullptr)
    , renderer(nullptr)
    , renderPending(true)
    , invalidatePending(false)
    , devicePixelRatio(1)
{
    qsgnode_set_description(this, QStringLiteral("fbonode"));
}

SGFramebufferObjectNode::~SGFramebufferObjectNode()
{
    delete renderer;
    delete texture();
    delete fbo;
}

void SGFramebufferObjectNode::scheduleRender()
{
    renderPending = true;
    window->update();
}

QSGTexture *SGFramebufferObjectNode::texture() const
{
    return QSGSimpleTextureNode::texture();
}

void SGFramebufferObjectNode::render()
{
    if (renderPending) {
        renderPending = false;
        fbo->renderContext->setRenderTarget(fbo->fbo);
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, fbo->size.width(), fbo->size.height());
        renderer->render();
        //fbo->renderContext->setRenderTarget(nullptr);
        window->resetOpenGLState();

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


QDemonSceneRenderer::QDemonSceneRenderer()
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (m_renderContext.isNull())
        m_renderContext = QDemonRenderContext::createGl(context->format());
    if (m_sgContext.isNull())
        m_sgContext = new QDemonRenderContextInterface(m_renderContext, QString::fromLatin1("./"));
}

void QDemonSceneRenderer::render()
{
    if (!m_layer)
        return;
    m_sgContext->setSceneColor(QVector4D(1.0f, 0.0f, 0.0f, 1.0f));
    m_sgContext->renderList()->setViewport(QRect(0, 0, m_surfaceSize.width(), m_surfaceSize.height()));
    m_sgContext->setWindowDimensions(m_surfaceSize);
    //m_sgContext->setViewport(QRect(viewport[0], viewport[1], viewport[2], viewport[3]));
    // Render
    m_sgContext->setPresentationDimensions(m_surfaceSize);
    m_sgContext->beginFrame();
    m_sgContext->renderContext()->resetBlendState();

    m_sgContext->renderer()->prepareLayerForRender(*m_layer, m_surfaceSize, false, nullptr);
    m_sgContext->runRenderTasks();
    // Set the default render target (which in our case should be the FBO)
    m_sgContext->renderer()->renderLayer(*m_layer, m_surfaceSize, false, QVector3D(), false, nullptr);

    m_sgContext->endFrame();
}

QDemonSceneRenderer::FrambufferObject *QDemonSceneRenderer::createFramebufferObject(const QSize &size)
{
    m_surfaceSize = size;
    return new QDemonSceneRenderer::FrambufferObject(size, m_renderContext);
}

void QDemonSceneRenderer::synchronize(QDemonView3D *item)
{
    if (!item)
        return;

    auto view3D = static_cast<QDemonView3D*>(item);
    m_sceneManager = QDemonObjectPrivate::get(view3D->scene())->sceneRenderer;
    m_sceneManager->updateDirtyNodes();

    if (view3D->referencedScene()) {
        QDemonObjectPrivate::get(view3D->referencedScene())->sceneRenderer->updateDirtyNodes();
    }


    // Generate layer node
    if (!m_layer) {
        m_layer = new QDemonRenderLayer();
    }

    // Update the layer node properties
    updateLayerNode(view3D);

    // Set the root item for the scene to the layer
    auto rootNode = static_cast<QDemonRenderNode*>(QDemonObjectPrivate::get(view3D->scene())->spatialNode);
    if (rootNode) {
        if (m_layer->firstChild != nullptr && m_layer->firstChild != rootNode)
            m_layer->removeChild(*m_layer->firstChild);
        else
            m_layer->addChild(*rootNode);
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

    // The layer is the full size of the surface
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

    layerNode->probe2Fade = view3D->environment()->probe2Fade();
    layerNode->probe2Window = view3D->environment()->probe2Window();
    layerNode->probe2Pos = view3D->environment()->probe2Postion();
    layerNode->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformNotDirty);
}

QDemonSceneRenderer::FrambufferObject::FrambufferObject(const QSize &s, QDemonRef<QDemonRenderContext> context)
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

QDemonSceneRenderer::FrambufferObject::~FrambufferObject()
{
}

QT_END_NAMESPACE



