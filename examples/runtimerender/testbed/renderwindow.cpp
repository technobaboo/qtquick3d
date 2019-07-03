#include "renderwindow.h"
#include <QtDemonRender/qdemonrendercontext.h>

#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

RenderWindow::RenderWindow(QWindow *parent)
    : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setWidth(1280);
    setHeight(720);
    m_frameTimer.start();
}

RenderWindow::~RenderWindow()
{
    delete m_glContext;
}



static QSurfaceFormat findIdealGLVersion()
{
    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Advanced: Try 4.3 core (so we get compute shaders for instance)
    fmt.setVersion(4, 3);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (ctx.create() && ctx.format().version() >= qMakePair(4, 3)) {
        qDebug("Requesting OpenGL 4.3 core context succeeded");
        return ctx.format();
    }

    // Basic: Stick with 3.3 for now to keep less fortunate, Mesa-based systems happy
    fmt.setVersion(3, 3);
    ctx.setFormat(fmt);
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 3)) {
        qDebug("Requesting OpenGL 3.3 core context succeeded");
        return ctx.format();
    }

    qDebug("Impending doom");
    return fmt;
}

static QSurfaceFormat findIdealGLESVersion()
{
    QSurfaceFormat fmt;

    // Advanced: Try 3.1 (so we get compute shaders for instance)
    fmt.setVersion(3, 1);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);

    // Now, it's important to check the format with the actual version (parsed
    // back from GL_VERSION) since some implementations, ANGLE for instance,
    // are broken and succeed the 3.1 context request even though they only
    // support and return a 3.0 context. This is against the spec since 3.0 is
    // obviously not backwards compatible with 3.1, but hey...
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 1)) {
        qDebug("Requesting OpenGL ES 3.1 context succeeded");
        return ctx.format();
    }

    // Basic: OpenGL ES 3.0 is a hard requirement at the moment since we can
    // only generate 300 es shaders, uniform buffers are mandatory.
    fmt.setVersion(3, 0);
    ctx.setFormat(fmt);
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 0)) {
        qDebug("Requesting OpenGL ES 3.0 context succeeded");
        return ctx.format();
    }

    fmt.setVersion(2, 0);
    ctx.setFormat(fmt);
    if (ctx.create()) {
        qDebug("Requesting OpenGL ES 2.0 context succeeded");
        return fmt;
    }

    qDebug("Impending doom");
    return fmt;
}

static QSurfaceFormat idealSurfaceFormat()
{
    static const QSurfaceFormat f = [] {
        QSurfaceFormat fmt;
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) { // works in dynamic gl builds too because there's a qguiapp already
            fmt = findIdealGLVersion();
        } else {
            fmt = findIdealGLESVersion();
        }
        fmt.setDepthBufferSize(24);
        fmt.setStencilBufferSize(8);
        // Ignore MSAA here as that is a per-layer setting.
        return fmt;
    }();
    return f;
}
void RenderWindow::initialize()
{
    m_renderContext = QDemonRenderContext::createGl(idealSurfaceFormat());
    m_context = QDemonRenderContextInterface::getRenderContextInterface(m_renderContext, "./", quintptr(this));
    m_context->setSceneColor(QVector4D(1.0, 0.0, 0.0, 1.0));

    buildTestScene();
}

void RenderWindow::drawFrame(qint64 delta)
{
    updateAnimations();

    QSize renderTargetSize = size() * devicePixelRatio();

    m_context->setPresentationDimensions(renderTargetSize);

    m_context->beginFrame();
    m_renderContext->resetBlendState();
    m_renderContext->setViewport(QRect(0, 0, renderTargetSize.width(), renderTargetSize.height()));

    m_context->renderer()->prepareLayerForRender(*m_layer, renderTargetSize, true);
    m_context->runRenderTasks();
    m_context->renderer()->renderLayer(*m_layer, renderTargetSize, false, QVector3D(0, 0, 0), true);

    m_context->endFrame();
}

void RenderWindow::renderLater()
{
    requestUpdate();
}

void RenderWindow::renderNow()
{
    if (!m_isIntialized) {
        preInit();
        initialize();
        m_isIntialized = true;
    }
    m_glContext->makeCurrent(this);
    drawFrame(m_frameTimer.elapsed());
    m_frameTimer.restart();
    m_glContext->swapBuffers(this);
    m_glContext->doneCurrent();
    if (m_autoUpdate)
        renderLater();
}

void RenderWindow::updateAnimations()
{
    m_cube->rotation = QVector3D(0.785398f, m_cube->rotation.y() + 0.01f, 0.785398f);
    m_cube->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformIsDirty);
}

bool RenderWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void RenderWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void RenderWindow::preInit()
{
    m_glContext = new QOpenGLContext();
    m_glContext->setFormat(requestedFormat());
    m_glContext->create();

    if (!m_glContext->makeCurrent(this))
        qDebug("fail");
}

void RenderWindow::buildTestScene()
{
    m_layer = new QDemonRenderLayer();
    m_layer->clearColor = QVector3D(0.0, 0.0, 1.0);
    m_layer->background = QDemonRenderLayer::Background::Color;
    m_layer->m_height = 100.f;
    m_layer->m_width = 100.f;
    m_layer->widthUnits = QDemonRenderLayer::UnitType::Percent;
    m_layer->heightUnits = QDemonRenderLayer::UnitType::Percent;

    // Camera
    auto camera = new QDemonRenderCamera();
    m_layer->addChild(*camera);
    camera->lookAt(QVector3D(0.0, 0.0, -600.0),
                   QVector3D(0.0, 1.0, 0.0),
                   QVector3D(0.0, 0.0, 0.0));

    // Light
    auto light = new QDemonRenderLight();
    m_layer->addChild(*light);

    // Mesh (#Cube)
    m_cube = new QDemonRenderModel();
    m_cube->meshPath = QDemonRenderMeshPath::create(QStringLiteral("#Cube"));
    m_layer->addChild(*m_cube);

    // Default Material
    auto material = new QDemonRenderDefaultMaterial();
    m_cube->materials.append(material);
}
