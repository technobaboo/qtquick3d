#include "renderwindow.h"
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>

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

void RenderWindow::initialize()
{
    m_contextCore = IQDemonRenderContextCore::Create();
    m_contextCore->SetTextRendererCore(ITextRendererCore::CreateQtTextRenderer());
    m_contextCore->SetOnscreenTextRendererCore(ITextRendererCore::CreateOnscreenTextRenderer());

    m_renderContext = QDemonRenderContext::CreateGL(format());

    m_context = m_contextCore->CreateRenderContext(m_renderContext, ".");
    m_context->SetSceneColor(QVector4D(1.0, 0.0, 0.0, 1.0));
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

    m_context->BeginFrame();

    // Render the first presentation (QDemonRenderPresentation)

    m_context->RunRenderTasks();

    m_context->EndFrame();
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
