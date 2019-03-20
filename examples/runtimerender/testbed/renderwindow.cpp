#include "renderwindow.h"
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRender/qdemonrendercontext.h>

#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
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
    m_context = new QDemonRenderContextInterface(m_renderContext, "./");
    m_context->setSceneColor(QVector4D(1.0, 0.0, 0.0, 1.0));

    buildTestScene();
}

void RenderWindow::drawFrame(qint64 delta)
{
//    Q3DStudio::BOOL RenderPresentation(Q3DStudio::IPresentation *inPresentation) override
//    {
//        Qt3DSRenderScene *theFirstScene = nullptr;
//        for (QT3DSU32 idx = 0, end = m_Scenes.size(); idx < end && theFirstScene == nullptr; ++idx)
//            if (m_Scenes[idx].second->m_RuntimePresentation == inPresentation)
//                theFirstScene = m_Scenes[idx].second;

//        if (theFirstScene && theFirstScene->m_Presentation) {
//            m_LastRenderedScene = theFirstScene;
//            if (theFirstScene->m_Presentation->m_Scene
//                && theFirstScene->m_Presentation->m_Scene->m_UseClearColor) {
//                m_Context->m_Context->SetSceneColor(
//                    QT3DSVec4(theFirstScene->m_Presentation->m_Scene->m_ClearColor, 1.0f));
//            } else
//                m_Context->m_Context->SetSceneColor(QT3DSVec4(0.0f, 0.0f, 0.0f, 0.0f));

//            // Setup the render rotation *before* rendering so that the magic can happen on begin
//            // render.
//            if (m_Context->m_RenderRotationsEnabled)
//                m_Context->m_Context->SetRenderRotation(
//                    theFirstScene->m_Presentation->m_PresentationRotation);
//            else
//                m_Context->m_Context->SetRenderRotation(RenderRotationValues::NoRotation);

//            m_Context->m_Context->SetPresentationDimensions(QSize(
//                (QT3DSU32)theFirstScene->m_Presentation->m_PresentationDimensions.x,
//                (QT3DSU32)theFirstScene->m_Presentation->m_PresentationDimensions.y));
//        }

//        m_Context->m_Context->BeginFrame();
//        m_Context->m_RenderContext->ResetBlendState();

//        // How exactly does this work, I have no idea.
//        // Should we only render the first scene and not every scene, perhaps?
//        bool wasDirty = false;
//        if (theFirstScene)
//            wasDirty = theFirstScene->PrepareForRender();
//        else {
//            m_Context->m_RenderContext->SetClearColor(QT3DSVec4(0, 0, 0, 0));
//            m_Context->m_RenderContext->Clear(qt3ds::render::NVRenderClearFlags(
//                NVRenderClearValues::Color | NVRenderClearValues::Depth));
//        }
//        m_Context->m_Context->RunRenderTasks();
//        if (theFirstScene)
//            theFirstScene->Render();

//        m_Context->m_Context->EndFrame();

//        return wasDirty;
//    }
//    bool PrepareForRender()
//    {
//        TransferDirtyProperties();
//        m_LastRenderViewport = m_Context->GetRenderList().GetViewport();
//        if (m_Presentation && m_Presentation->m_Scene) {
//            NVRenderRect theViewportSize(m_LastRenderViewport);
//            return m_Presentation->m_Scene->PrepareForRender(
//                QT3DSVec2((QT3DSF32)theViewportSize.m_Width, (QT3DSF32)theViewportSize.m_Height),
//                *m_Context);
//        }
//        return false;
//    }

//    void Render()
//    {
//        if (m_Presentation && m_Presentation->m_Scene) {
//            NVRenderRect theViewportSize(m_LastRenderViewport);
//            m_Presentation->m_Scene->Render(
//                QT3DSVec2((QT3DSF32)theViewportSize.m_Width, (QT3DSF32)theViewportSize.m_Height), *m_Context,
//                SScene::DoNotClear);
//        }
//    }
    updateAnimations();

    // Set Clear Color
    if (m_scene && m_scene->useClearColor)
        m_context->setSceneColor(QVector4D(m_scene->clearColor, 1.0f));
    else
        m_context->setSceneColor(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));

    m_context->setPresentationDimensions(QSize(m_presentation->presentationDimensions.x(),
                                               m_presentation->presentationDimensions.y()));

    m_context->beginFrame();
    m_renderContext->resetBlendState();

    // Render the first presentation (QDemonRenderPresentation)
    auto lastRenderViewport = m_context->renderList()->getViewport();
    if (m_presentation && m_presentation->scene) {
        QRect theViewportSize(lastRenderViewport);
        m_presentation->scene->prepareForRender(QVector2D(theViewportSize.width(), theViewportSize.height()), m_context.data());
    }

    m_context->runRenderTasks();
    if (m_presentation && m_presentation->scene) {
        QRect theViewportSize(lastRenderViewport);
        m_presentation->scene->render(QVector2D(theViewportSize.width(), theViewportSize.height()), m_context.data(), QDemonRenderScene::DoNotClear);
    }

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
    m_presentation = new QDemonRenderPresentation();
    m_scene = new QDemonRenderScene();
    m_scene->clearColor = QVector3D(0.0, 1.0, 0.0);
    m_presentation->scene = m_scene;
    m_scene->presentation = m_presentation.data();

    auto layer = new QDemonRenderLayer();
    layer->clearColor = QVector3D(0.0, 0.0, 1.0);
    layer->background = QDemonRenderLayer::Background::Color;

    m_scene->addChild(*layer);

    // Camera
    auto camera = new QDemonRenderCamera();
    layer->addChild(*camera);
    camera->lookAt(QVector3D(0.0, 0.0, -600.0),
                   QVector3D(0.0, 1.0, 0.0),
                   QVector3D(0.0, 0.0, 0.0));

    // Light
    auto light = new QDemonRenderLight();
    layer->addChild(*light);

    // Mesh (#Cube)
    m_cube = new QDemonRenderModel();
    m_cube->meshPath = QStringLiteral("#Cube");
    layer->addChild(*m_cube);

    // Default Material
    auto material = new QDemonRenderDefaultMaterial();
    m_cube->addMaterial(*material);
}
