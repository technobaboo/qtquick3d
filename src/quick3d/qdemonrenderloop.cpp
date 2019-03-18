#include "qdemonrenderloop_p.h"

#include "qdemonwindow.h"
#include "qdemonwindow_p.h"

#include <QtCore/QRunnable>
#include <QtCore/QCoreApplication>
#include <QtCore/private/qabstractanimation_p.h>

#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/private/qopenglcontext_p.h>

QT_BEGIN_NAMESPACE

static bool dumpTimingInfo = false;
static int frameCount = 0;

QDemonRenderLoop *QDemonRenderLoop::s_instance = nullptr;
// extern bool qsg_useConsistentTiming();

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

    QDemonRef<QDemonRenderContextInterface> sceneGraphContext() const override;
    QDemonRef<QDemonRenderContext> renderContext() const override { return m_renderContext; }

    struct WindowData
    {
        bool updatePending : 1;
        bool grabOnly : 1;
    };

    QHash<QDemonWindow *, WindowData> m_windows;

    QOpenGLContext *gl;
    QSharedPointer<QOffscreenSurface> m_offscreenSurface;
    QDemonRef<QDemonRenderContextCoreInterface> m_contextCore;
    QDemonRef<QDemonRenderContextInterface> m_sgContext;
    QDemonRef<QDemonRenderContext> m_renderContext;

    QImage grabContent;
};

#include "qdemonrenderloop.moc"

QDemonRenderLoop::~QDemonRenderLoop()
{
    dumpTimingInfo = !qgetenv("QUICK3D_PERFTIMERS").isEmpty();
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

void QDemonRenderLoop::handleContextCreationFailure(QDemonWindow *window, bool isEs){ Q_UNUSED(window) Q_UNUSED(isEs) }

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

QDemonGuiThreadRenderLoop::QDemonGuiThreadRenderLoop() : gl(nullptr)
{
    m_contextCore = QDemonRenderContextCoreInterface::create();

    // To create the Render Context, we have to have a valid OpenGL Context
    // to resolve the functions, so do that now (before we have any windows)
    QSurfaceFormat format = idealSurfaceFormat();

    m_offscreenSurface.reset(new QOffscreenSurface);
    m_offscreenSurface->setFormat(format);
    m_offscreenSurface->create();

    gl = new QOpenGLContext();
    gl->setFormat(format);
    if (qt_gl_global_share_context())
        gl->setShareContext(qt_gl_global_share_context());
    if (!gl->create()) {
        delete gl;
        gl = nullptr;
    } else {
        gl->makeCurrent(m_offscreenSurface.data());
        m_renderContext = QDemonRenderContext::createGl(format);
        m_sgContext = m_contextCore->createRenderContext(m_renderContext, "./");
        gl->doneCurrent();
    }
}

QDemonGuiThreadRenderLoop::~QDemonGuiThreadRenderLoop() {}

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
    if (gl) {
        QSurface *surface = window;
        // There may be no platform window if the window got closed.
        if (!window->handle()) {
            surface = m_offscreenSurface.data();
        }
        current = gl->makeCurrent(surface);
    }

    d->cleanupNodesOnShutdown();
    if (m_windows.size() == 0) {
        // rc->invalidate();
        delete gl;
        gl = nullptr;
    } else if (gl && window == gl->surface() && current) {
        gl->doneCurrent();
    }
    // delete d->animationController;
}

void QDemonGuiThreadRenderLoop::renderWindow(QDemonWindow *window)
{
    if (!m_windows.contains(window))
        return;

    m_sgContext->setWindowDimensions(window->size() * window->effectiveDevicePixelRatio());

    WindowData &data = const_cast<WindowData &>(m_windows[window]);
    bool alsoSwap = data.updatePending;
    data.updatePending = false;

    QDemonWindowPrivate *cd = QDemonWindowPrivate::get(window);
    if (!cd->isRenderable())
        return;

    bool current = false;

    if (gl)
        current = gl->makeCurrent(window);

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
        // cd->flushFrameSynchronousEvents();
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
        //        grabContent = qt_gl_read_framebuffer(window->size() * window->effectiveDevicePixelRatio(), alpha,
        //        alpha); grabContent.setDevicePixelRatio(window->effectiveDevicePixelRatio());
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

QDemonRef<QDemonRenderContextInterface> QDemonGuiThreadRenderLoop::sceneGraphContext() const
{
    return m_sgContext;
}

void QDemonGuiThreadRenderLoop::releaseResources(QDemonWindow *w)
{
    // No full invalidation of the rendercontext, just clear some caches.
    // QDemonWindowPrivate *d = QDemonWindowPrivate::get(w);
    //    if (d->renderer)
    //        d->renderer->releaseCachedResources();
}

void QDemonGuiThreadRenderLoop::handleUpdateRequest(QDemonWindow *window)
{
    renderWindow(window);

    if (++frameCount == 60  ) {
        m_contextCore->getPerfTimer()->dump(frameCount);
        frameCount = 0;
    }
}

QT_END_NAMESPACE
