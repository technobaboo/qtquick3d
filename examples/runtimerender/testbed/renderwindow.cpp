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
    m_context->BeginFrame();

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
