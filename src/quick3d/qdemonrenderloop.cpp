#include "qdemonrenderloop_p.h"

#include "qdemonwindow.h"
#include "qdemonwindow_p.h"

#include <QtCore/QRunnable>
#include <QtCore/QCoreApplication>
#include <QtCore/QScopedPointer>
#include <QtCore/private/qabstractanimation_p.h>

#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/private/qopenglcontext_p.h>

QT_BEGIN_NAMESPACE

QDemonRenderLoop *QDemonRenderLoop::s_instance = nullptr;
//extern bool qsg_useConsistentTiming();

class QDemonGuiThreadRenderLoop : public QDemonRenderLoop
{
    Q_OBJECT
public:
    QDemonGuiThreadRenderLoop();
    ~QDemonGuiThreadRenderLoop() override;

    void show(QDemonWindow *window) override;
    void hide(QDemonWindow *window) override;

    void windowDestroyed(QDemonWindow *window) override;

    void renderWindow(QDemonWindow *window);
    void exposureChanged(QDemonWindow *window) override;
    QImage grab(QDemonWindow *window) override;

    void maybeUpdate(QDemonWindow *window) override;
    void update(QDemonWindow *window) override { maybeUpdate(window); } // identical for this implementation.
    void handleUpdateRequest(QDemonWindow *) override;

    void releaseResources(QDemonWindow *) override;

    QAnimationDriver *animationDriver() const override { return nullptr; }

    QSharedPointer<IQDemonRenderContext> sceneGraphContext() const override;
    QSharedPointer<QDemonRenderContext> renderContext() const override { return m_renderContext; }

    struct WindowData {
        bool updatePending : 1;
        bool grabOnly : 1;
    };

    QHash<QDemonWindow *, WindowData> m_windows;

    QOpenGLContext *gl;
    QSharedPointer<IQDemonRenderContextCore> m_contextCore;
    QSharedPointer<IQDemonRenderContext> m_sgContext;
    QSharedPointer<QDemonRenderContext> m_renderContext;

    QImage grabContent;
};

#include "qdemonrenderloop.moc"

QDemonRenderLoop::~QDemonRenderLoop()
{
}

void QDemonRenderLoop::postJob(QDemonWindow *window, QRunnable *job)
{
    Q_ASSERT(job);
    Q_ASSERT(window);
    if (window->graphicsContext()) {
        window->graphicsContext()->makeCurrent(window);
        job->run();
    }
    delete job;
}

QSurface::SurfaceType QDemonRenderLoop::windowSurfaceType() const
{
    return QSurface::OpenGLSurface;
}

void QDemonRenderLoop::cleanup()
{
    if (!s_instance)
        return;
    for (QDemonWindow *w : s_instance->windows()) {
        QDemonWindowPrivate *wd = QDemonWindowPrivate::get(w);
        if (wd->windowManager == s_instance) {
           s_instance->windowDestroyed(w);
           wd->windowManager = nullptr;
        }
    }
    delete s_instance;
    s_instance = nullptr;
}

void QDemonRenderLoop::handleContextCreationFailure(QDemonWindow *window, bool isEs)
{
    Q_UNUSED(window)
    Q_UNUSED(isEs)
}

QDemonRenderLoop *QDemonRenderLoop::instance()
{
    if (!s_instance) {
        s_instance = new QDemonGuiThreadRenderLoop();
        qAddPostRoutine(QDemonRenderLoop::cleanup);
    }

    return s_instance;
}

void QDemonRenderLoop::setInstance(QDemonRenderLoop *instance)
{
    Q_ASSERT(!s_instance);
    s_instance = instance;
}

QDemonGuiThreadRenderLoop::QDemonGuiThreadRenderLoop()
    : gl(nullptr)
{
//    if (qsg_useConsistentTiming()) {
//        QUnifiedTimer::instance(true)->setConsistentTiming(true);
//    }

    m_contextCore = IQDemonRenderContextCore::Create();
    m_contextCore->SetTextRendererCore(ITextRendererCore::CreateQtTextRenderer());
    m_contextCore->SetOnscreenTextRendererCore(ITextRendererCore::CreateOnscreenTextRenderer());
}

QDemonGuiThreadRenderLoop::~QDemonGuiThreadRenderLoop()
{
}

void QDemonGuiThreadRenderLoop::show(QDemonWindow *window)
{
    WindowData data;
    data.updatePending = false;
    data.grabOnly = false;
    m_windows[window] = data;

    maybeUpdate(window);
}

void QDemonGuiThreadRenderLoop::hide(QDemonWindow *window)
{
    QDemonWindowPrivate *cd = QDemonWindowPrivate::get(window);
    cd->fireAboutToStop();
    if (m_windows.contains(window))
        m_windows[window].updatePending = false;
}

void QDemonGuiThreadRenderLoop::windowDestroyed(QDemonWindow *window)
{
    m_windows.remove(window);
    hide(window);
    QDemonWindowPrivate *d = QDemonWindowPrivate::get(window);

    bool current = false;
    QScopedPointer<QOffscreenSurface> offscreenSurface;
    if (gl) {
        QSurface *surface = window;
        // There may be no platform window if the window got closed.
        if (!window->handle()) {
            offscreenSurface.reset(new QOffscreenSurface);
            offscreenSurface->setFormat(gl->format());
            offscreenSurface->create();
            surface = offscreenSurface.data();
        }
        current = gl->makeCurrent(surface);
    }

    d->cleanupNodesOnShutdown();
    if (m_windows.size() == 0) {
        //rc->invalidate();
        delete gl;
        gl = nullptr;
    } else if (gl && window == gl->surface() && current) {
        gl->doneCurrent();
    }

    delete d->animationController;
}

void QDemonGuiThreadRenderLoop::renderWindow(QDemonWindow *window)
{
    if (!m_windows.contains(window))
        return;

    WindowData &data = const_cast<WindowData &>(m_windows[window]);
    bool alsoSwap = data.updatePending;
    data.updatePending = false;

    QDemonWindowPrivate *cd = QDemonWindowPrivate::get(window);
    if (!cd->isRenderable())
        return;

    bool current = false;

    if (!gl) {
        gl = new QOpenGLContext();
        gl->setFormat(window->requestedFormat());
        gl->setScreen(window->screen());
        if (qt_gl_global_share_context())
            gl->setShareContext(qt_gl_global_share_context());
        if (!gl->create()) {
            const bool isEs = gl->isOpenGLES();
            delete gl;
            gl = nullptr;
            handleContextCreationFailure(window, isEs);
        } else {
            //cd->fireOpenGLContextCreated(gl);
            current = gl->makeCurrent(window);
        }
        if (current) {
            m_renderContext = QDemonRenderContext::CreateGL(gl->format());
            m_sgContext = m_contextCore->CreateRenderContext(m_renderContext, "./");
        }
    } else {
        current = gl->makeCurrent(window);
    }

    bool lastDirtyWindow = true;
    auto i = m_windows.constBegin();
    while (i != m_windows.constEnd()) {
        if (i.value().updatePending) {
            lastDirtyWindow = false;
            break;
        }
        i++;
    }

    if (!current)
        return;

    if (!data.grabOnly) {
        //cd->flushFrameSynchronousEvents();
        // Event delivery/processing triggered the window to be deleted or stop rendering.
        if (!m_windows.contains(window))
            return;
    }

    cd->polishItems();

    emit window->afterAnimating();

    cd->syncSceneGraph();
//    if (lastDirtyWindow)
//        rc->endSync();

    cd->renderSceneGraph(window->size());

    if (data.grabOnly) {
//        bool alpha = window->format().alphaBufferSize() > 0 && window->color().alpha() != 255;
//        grabContent = qt_gl_read_framebuffer(window->size() * window->effectiveDevicePixelRatio(), alpha, alpha);
//        grabContent.setDevicePixelRatio(window->effectiveDevicePixelRatio());
        data.grabOnly = false;
    }

    if (alsoSwap && window->isVisible()) {
        gl->swapBuffers(window);
        cd->fireFrameSwapped();
    }

    // Might have been set during syncSceneGraph()
    if (data.updatePending)
        maybeUpdate(window);
}

void QDemonGuiThreadRenderLoop::exposureChanged(QDemonWindow *window)
{
    if (window->isExposed()) {
        m_windows[window].updatePending = true;
        renderWindow(window);
    }
}

QImage QDemonGuiThreadRenderLoop::grab(QDemonWindow *window)
{
    if (!m_windows.contains(window))
        return QImage();

    m_windows[window].grabOnly = true;

    renderWindow(window);

    QImage grabbed = grabContent;
    grabContent = QImage();
    return grabbed;
}

void QDemonGuiThreadRenderLoop::maybeUpdate(QDemonWindow *window)
{
    QDemonWindowPrivate *cd = QDemonWindowPrivate::get(window);
    if (!cd->isRenderable() || !m_windows.contains(window))
        return;

    m_windows[window].updatePending = true;
    window->requestUpdate();
}

QSharedPointer<IQDemonRenderContext> QDemonGuiThreadRenderLoop::sceneGraphContext() const
{
    return m_sgContext;
}

void QDemonGuiThreadRenderLoop::releaseResources(QDemonWindow *w)
{
    // No full invalidation of the rendercontext, just clear some caches.
    //QDemonWindowPrivate *d = QDemonWindowPrivate::get(w);
//    if (d->renderer)
//        d->renderer->releaseCachedResources();
}

void QDemonGuiThreadRenderLoop::handleUpdateRequest(QDemonWindow *window)
{
    renderWindow(window);
}


QT_END_NAMESPACE
