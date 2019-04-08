#include "qdemonscenerenderer.h"
#include "qdemonsceneenvironment.h"
#include "qdemonobject_p.h"
#include "qdemonnode.h"
#include "qdemonscenemanager_p.h"
#include "qdemonimage.h"
#include "qdemonoffscreenlayerrenderer.h"

#include <private/qopenglvertexarrayobject_p.h>

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
}

SGFramebufferObjectNode::~SGFramebufferObjectNode()
{
    delete renderer;
    delete texture();
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

void SGFramebufferObjectNode::resetOpenGLState() {
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions *gl = ctx->functions();

    if (!m_vaoHelper)
        m_vaoHelper = new QOpenGLVertexArrayObjectHelper(ctx);
    if (m_vaoHelper->isValid())
        m_vaoHelper->glBindVertexArray(0);

    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (ctx->isOpenGLES() || (gl->openGLFeatures() & QOpenGLFunctions::FixedFunctionPipeline)) {
        int maxAttribs;
        gl->glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
        for (int i=0; i<maxAttribs; ++i) {
            gl->glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
            gl->glDisableVertexAttribArray(i);
        }
    }

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, 0);

    gl->glDisable(GL_DEPTH_TEST);
    gl->glDisable(GL_STENCIL_TEST);
    gl->glDisable(GL_SCISSOR_TEST);

    gl->glColorMask(true, true, true, true);
    gl->glClearColor(0, 0, 0, 0);

    gl->glDepthMask(true);
    gl->glDepthFunc(GL_LESS);
    gl->glClearDepthf(1);

    gl->glStencilMask(0xff);
    gl->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    gl->glStencilFunc(GL_ALWAYS, 0, 0xff);

    gl->glDisable(GL_BLEND);
    gl->glBlendFunc(GL_ONE, GL_ZERO);

    gl->glUseProgram(0);

    QOpenGLFramebufferObject::bindDefault();
}

void SGFramebufferObjectNode::render()
{
    if (renderPending) {
        renderPending = false;
        GLuint textureId = renderer->render();

        //resetOpenGLState();

        if (texture() && (texture()->textureId() != textureId || texture()->textureSize() != renderer->surfaceSize())) {
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
    QOpenGLContext *oldContext = QOpenGLContext::currentContext();

    m_openGLContext = new QOpenGLContext();
    m_openGLContext->setFormat(oldContext->format());
    m_openGLContext->setShareContext(oldContext);
    m_openGLContext->create();

    m_openGLContext->makeCurrent(m_window);

    if (m_renderContext.isNull())
        m_renderContext = QDemonRenderContext::createGl(oldContext->format());
    if (m_sgContext.isNull())
        m_sgContext = new QDemonRenderContextInterface(m_renderContext, QString::fromLatin1("./"));
    m_openGLContext->doneCurrent();
    oldContext->makeCurrent(window);
}

QDemonSceneRenderer::~QDemonSceneRenderer()
{
    delete m_openGLContext;
}

GLuint QDemonSceneRenderer::render()
{
    if (!m_layer)
        return 0;

    QOpenGLContext *oldContext = QOpenGLContext::currentContext();

    m_openGLContext->makeCurrent(m_window);
    m_sgContext->beginFrame();
    m_sgContext->offscreenRenderManager()->beginFrame();
    auto result = m_sgContext->offscreenRenderManager()->getRenderedItem(QDemonOffscreenRendererKey(m_layer));
    m_sgContext->runRenderTasks();
    m_sgContext->offscreenRenderManager()->endFrame();
    m_sgContext->endFrame();
    m_openGLContext->doneCurrent();
    oldContext->makeCurrent(m_window);

    if (!result.texture.isNull())
        return HandleToID_cast(GLuint, size_t, result.texture->handle());

    return 0;
}

void QDemonSceneRenderer::synchronize(QDemonView3D *item, const QSize &size)
{
    if (!item)
        return;

    if (m_surfaceSize != size) {
        m_layerSizeIsDirty = true;
        m_surfaceSize = size;
    }

    auto view3D = static_cast<QDemonView3D*>(item);
    m_sceneManager = QDemonObjectPrivate::get(view3D->scene())->sceneRenderer;
    m_sceneManager->updateDirtyNodes();

    if (view3D->referencedScene()) {
        QDemonObjectPrivate::get(view3D->referencedScene())->sceneRenderer->updateDirtyNodes();
    }

    // Generate layer node
    if (!m_layer)
        m_layer = new QDemonRenderLayer();

    // Update the layer node properties
    updateLayerNode(view3D);

    // Set the root item for the scene to the layer
    auto rootNode = static_cast<QDemonRenderNode*>(QDemonObjectPrivate::get(view3D->scene())->spatialNode);
    if (rootNode) {
        if (m_layer->firstChild != nullptr && m_layer->firstChild != rootNode)
            m_layer->removeChild(*m_layer->firstChild);

        if (m_layer->firstChild == nullptr)
            m_layer->addChild(*rootNode);
    }

    auto offscreenRenderer = m_sgContext->offscreenRenderManager()->getOffscreenRenderer(QDemonOffscreenRendererKey(m_layer));
    if (offscreenRenderer.isNull() || m_layerSizeIsDirty) {
        if (!offscreenRenderer.isNull())
            m_sgContext->offscreenRenderManager()->releaseOffscreenRenderer(QDemonOffscreenRendererKey(m_layer));
        offscreenRenderer = new QDemonOffscreenLayerRenderer(m_sgContext, m_layer, m_surfaceSize);
        m_sgContext->offscreenRenderManager()->registerOffscreenRenderer(QDemonOffscreenRendererKey(m_layer), offscreenRenderer);
        m_layerSizeIsDirty = false;
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

    layerNode->probe2Fade = view3D->environment()->probe2Fade();
    layerNode->probe2Window = view3D->environment()->probe2Window();
    layerNode->probe2Pos = view3D->environment()->probe2Postion();
    layerNode->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformNotDirty);
}

QT_END_NAMESPACE



