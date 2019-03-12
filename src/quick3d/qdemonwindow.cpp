#include "qdemonwindow.h"
#include "qdemonwindow_p.h"
#include "qdemonobject_p.h"
#include "qdemonnode.h"
#include "qdemonlayer.h"
#include <private/qqmlglobal_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtQml/qqmlincubator.h>
#include "qdemonrenderloop_p.h"

#include <QtGui/QGuiApplication>

#include <QtDemonRuntimeRender/QDemonRenderLayer>

#include <qalgorithms.h>

QT_BEGIN_NAMESPACE

extern Q_GUI_EXPORT QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha);

class QDemonWindowIncubationController : public QObject, public QQmlIncubationController
{
    Q_OBJECT

public:
    QDemonWindowIncubationController(QDemonRenderLoop *loop) : m_renderLoop(loop), m_timer(0)
    {
        // Allow incubation for 1/3 of a frame.
        m_incubation_time = qMax(1, int(1000 / QGuiApplication::primaryScreen()->refreshRate()) / 3);

        QAnimationDriver *animationDriver = m_renderLoop->animationDriver();
        if (animationDriver) {
            connect(animationDriver, SIGNAL(stopped()), this, SLOT(animationStopped()));
            connect(m_renderLoop, SIGNAL(timeToIncubate()), this, SLOT(incubate()));
        }
    }

protected:
    void timerEvent(QTimerEvent *) override
    {
        killTimer(m_timer);
        m_timer = 0;
        incubate();
    }

    void incubateAgain()
    {
        if (m_timer == 0) {
            // Wait for a while before processing the next batch. Using a
            // timer to avoid starvation of system events.
            m_timer = startTimer(m_incubation_time);
        }
    }

public slots:
    void incubate()
    {
        if (incubatingObjectCount()) {
            if (m_renderLoop->interleaveIncubation()) {
                incubateFor(m_incubation_time);
            } else {
                incubateFor(m_incubation_time * 2);
                if (incubatingObjectCount())
                    incubateAgain();
            }
        }
    }

    void animationStopped() { incubate(); }

protected:
    void incubatingObjectCountChanged(int count) override
    {
        if (count && !m_renderLoop->interleaveIncubation())
            incubateAgain();
    }

private:
    QDemonRenderLoop *m_renderLoop;
    int m_incubation_time;
    int m_timer;
};

#include "qdemonwindow.moc"
//#include "moc_qdemonwindow_p.cpp"

QDemonWindow::QDemonWindow(QWindow *parent) : QDemonWindow(*new QDemonWindowPrivate, parent) {}

QDemonWindow::QDemonWindow(QDemonWindowPrivate &dd, QWindow *parent) : QWindow(dd, parent)
{
    Q_D(QDemonWindow);
    d->init(this);
}

QDemonWindow::~QDemonWindow()
{
    Q_D(QDemonWindow);

    if (d->windowManager) {
        d->windowManager->removeWindow(this);
        d->windowManager->windowDestroyed(this);
    }

    delete d->incubationController;
    d->incubationController = nullptr;
    QDemonObject *root = d->contentItem;
    d->contentItem = nullptr;
    delete root;

    d->renderJobMutex.lock();
    qDeleteAll(d->beforeSynchronizingJobs);
    d->beforeSynchronizingJobs.clear();
    qDeleteAll(d->afterSynchronizingJobs);
    d->afterSynchronizingJobs.clear();
    qDeleteAll(d->beforeRenderingJobs);
    d->beforeRenderingJobs.clear();
    qDeleteAll(d->afterRenderingJobs);
    d->afterRenderingJobs.clear();
    qDeleteAll(d->afterSwapJobs);
    d->afterSwapJobs.clear();
    d->renderJobMutex.unlock();
}

QDemonObject *QDemonWindow::contentItem() const
{
    Q_D(const QDemonWindow);

    return d->contentItem;
}

QDemonObject *QDemonWindow::activeFocusItem() const
{
    Q_D(const QDemonWindow);

    return d->activeFocusItem;
}

QObject *QDemonWindow::focusObject() const
{
    Q_D(const QDemonWindow);

    if (d->activeFocusItem)
        return d->activeFocusItem;
    return const_cast<QDemonWindow *>(this);
}

QImage QDemonWindow::grabWindow()
{
    //    Q_D(QDemonWindow);

    //    if (!isVisible()) {
    //        if (d->windowManager)
    //            return d->windowManager->grab(this);
    //    }

    //    if (!isVisible()) {
    //        auto openglRenderContext = static_cast<QSGDefaultRenderContext *>(d->context);
    //        if (!openglRenderContext->openglContext()) {
    //            if (!handle() || !size().isValid()) {
    //                qWarning("QQuickWindow::grabWindow: window must be created and have a valid size");
    //                return QImage();
    //            }

    //            QOpenGLContext context;
    //            context.setFormat(requestedFormat());
    //            context.setShareContext(qt_gl_global_share_context());
    //            context.create();
    //            context.makeCurrent(this);
    //            d->context->initialize(&context);

    //            d->polishItems();
    //            d->syncSceneGraph();
    //            d->renderSceneGraph(size());

    //            bool alpha = format().alphaBufferSize() > 0 && color().alpha() < 255;
    //            QImage image = qt_gl_read_framebuffer(size() * effectiveDevicePixelRatio(), alpha, alpha);
    //            image.setDevicePixelRatio(effectiveDevicePixelRatio());
    //            d->cleanupNodesOnShutdown();
    //            d->context->invalidate();
    //            context.doneCurrent();

    //            return image;
    //        }
    //    }

    //    if (d->windowManager)
    //        return d->windowManager->grab(this);
    return QImage();
}

QQmlIncubationController *QDemonWindow::incubationController() const
{
    Q_D(const QDemonWindow);

    if (!d->windowManager)
        return nullptr; // TODO: make sure that this is safe

    if (!d->incubationController)
        d->incubationController = new QDemonWindowIncubationController(d->windowManager);
    return d->incubationController;
}

void QDemonWindow::setColor(const QColor &color)
{
    Q_D(QDemonWindow);
    if (color == d->clearColor)
        return;

    if (color.alpha() != d->clearColor.alpha()) {
        QSurfaceFormat fmt = requestedFormat();
        if (color.alpha() < 255)
            fmt.setAlphaBufferSize(8);
        else
            fmt.setAlphaBufferSize(-1);
        setFormat(fmt);
    }
    d->clearColor = color;
    emit colorChanged(color);
    update();
}

QColor QDemonWindow::color() const
{
    return d_func()->clearColor;
}

void QDemonWindow::scheduleRenderJob(QRunnable *job, QDemonWindow::RenderStage stage)
{
    Q_D(QDemonWindow);

    d->renderJobMutex.lock();
    if (stage == BeforeSynchronizingStage) {
        d->beforeSynchronizingJobs << job;
    } else if (stage == AfterSynchronizingStage) {
        d->afterSynchronizingJobs << job;
    } else if (stage == BeforeRenderingStage) {
        d->beforeRenderingJobs << job;
    } else if (stage == AfterRenderingStage) {
        d->afterRenderingJobs << job;
    } else if (stage == AfterSwapStage) {
        d->afterSwapJobs << job;
    } else if (stage == NoStage) {
        if (isExposed())
            d->windowManager->postJob(this, job);
        else
            delete job;
    }
    d->renderJobMutex.unlock();
}

qreal QDemonWindow::effectiveDevicePixelRatio() const
{
    return devicePixelRatio();
}

QOpenGLContext *QDemonWindow::graphicsContext()
{
    Q_D(QDemonWindow);
    return d->glContext;
}

void QDemonWindow::update()
{
    Q_D(QDemonWindow);
    if (d->windowManager)
        d->windowManager->update(this);
}

void QDemonWindow::releaseResources()
{
    Q_D(QDemonWindow);
    if (d->windowManager)
        d->windowManager->releaseResources(this);
}

void QDemonWindow::exposeEvent(QExposeEvent *)
{
    Q_D(QDemonWindow);
    if (d->windowManager)
        d->windowManager->exposureChanged(this);
}

void QDemonWindow::resizeEvent(QResizeEvent *ev)
{
    Q_D(QDemonWindow);
    // TODO: Determine if we need to set a new size to the "content" item
    //    if (d->contentItem)
    //        d->contentItem->setSize(ev->size());
    if (d->m_presentation) {
        d->m_presentation->presentationDimensions.setX(ev->size().width());
        d->m_presentation->presentationDimensions.setY(ev->size().height());
    }

    if (d->windowManager)
        d->windowManager->resize(this);
}

void QDemonWindow::showEvent(QShowEvent *)
{
    Q_D(QDemonWindow);
    if (d->windowManager)
        d->windowManager->show(this);
}

void QDemonWindow::hideEvent(QHideEvent *)
{
    Q_D(QDemonWindow);
    if (d->windowManager)
        d->windowManager->hide(this);
}

void QDemonWindow::focusInEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev)
    //    Q_D(QDemonWindow);
    //    if (d->contentItem)
    //        d->contentItem->setFocus(true, ev->reason());
    // d->updateFocusItemTransform();
}

void QDemonWindow::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev)
    //    Q_D(QDemonWindow);
    //    if (d->contentItem)
    //        d->contentItem->setFocus(false, ev->reason());
}

bool QDemonWindow::event(QEvent *e)
{
    Q_D(QDemonWindow);

    switch (e->type()) {
    case QEvent::Type(QDemonWindowPrivate::FullUpdateRequest):
        update();
        break;
    case QEvent::UpdateRequest:
        if (d->windowManager)
            d->windowManager->handleUpdateRequest(this);
        break;
    default:
        break;
    }
    return QWindow::event(e);
}

void QDemonWindow::keyPressEvent(QKeyEvent *) {}

void QDemonWindow::keyReleaseEvent(QKeyEvent *) {}

void QDemonWindow::mousePressEvent(QMouseEvent *) {}

void QDemonWindow::mouseReleaseEvent(QMouseEvent *) {}

void QDemonWindow::mouseDoubleClickEvent(QMouseEvent *) {}

void QDemonWindow::mouseMoveEvent(QMouseEvent *) {}

#if QT_CONFIG(wheelevent)
void QDemonWindow::wheelEvent(QWheelEvent *) {}
#endif

void QDemonWindow::maybeUpdate()
{
    Q_D(QDemonWindow);
    if (d->windowManager)
        d->windowManager->maybeUpdate(this);
}

void QDemonWindow::cleanupSceneGraph()
{
    Q_D(QDemonWindow);

    //    if (!d->renderer)
    //        return;

    //    delete d->renderer->rootNode();
    //    delete d->renderer;
    //    d->renderer = nullptr;

    d->runAndClearJobs(&d->beforeSynchronizingJobs);
    d->runAndClearJobs(&d->afterSynchronizingJobs);
    d->runAndClearJobs(&d->beforeRenderingJobs);
    d->runAndClearJobs(&d->afterRenderingJobs);
    d->runAndClearJobs(&d->afterSwapJobs);
}

static void updatePixelRatioHelper(QDemonObject *item, float pixelRatio)
{
    QDemonObjectPrivate *itemPrivate = QDemonObjectPrivate::get(item);
    itemPrivate->itemChange(QDemonObject::ItemDevicePixelRatioHasChanged, pixelRatio);

    QList<QDemonObject *> items = item->childItems();
    for (int i = 0; i < items.size(); ++i)
        updatePixelRatioHelper(items.at(i), pixelRatio);
}

void QDemonWindow::physicalDpiChanged()
{
    Q_D(QDemonWindow);
    const qreal newPixelRatio = screen()->devicePixelRatio();
    if (qFuzzyCompare(newPixelRatio, d->devicePixelRatio))
        return;
    d->devicePixelRatio = newPixelRatio;
    if (d->contentItem)
        updatePixelRatioHelper(d->contentItem, newPixelRatio);
}

void QDemonWindow::handleScreenChanged(QScreen *screen)
{
    Q_D(QDemonWindow);
    if (screen) {
        physicalDpiChanged();
        // When physical DPI changes on the same screen, either the resolution or the device pixel
        // ratio changed. We must check what it is. Device pixel ratio does not have its own
        // ...Changed() signal.
        d->physicalDpiChangedConnection = connect(screen, SIGNAL(physicalDotsPerInchChanged(qreal)), this, SLOT(physicalDpiChanged()));
    } else {
        disconnect(d->physicalDpiChangedConnection);
    }

    d->forcePolish();
}

void QDemonWindow::setTransientParent_helper(QDemonWindow *window)
{
    setTransientParent(window);
    disconnect(sender(), SIGNAL(windowChanged(QDemonWindow *)), this, SLOT(setTransientParent_helper(QDemonWindow *)));
}

void QDemonWindow::runJobsAfterSwap()
{
    Q_D(QDemonWindow);
    d->runAndClearJobs(&d->afterSwapJobs);
}

void QDemonWindow::handleApplicationStateChanged(Qt::ApplicationState state)
{
    //    Q_D(QDemonWindow);
    //    if (state != Qt::ApplicationActive && d->contentItem)
    //        d->contentItem->windowDeactivateEvent();
}

QDemonWindowPrivate::QDemonWindowPrivate()
    : contentItem(nullptr)
    , activeFocusItem(nullptr)
    , dirtySpatialNodeList(nullptr)
    , dirtyResourceList(nullptr)
    , dirtyImageList(nullptr)
    , devicePixelRatio(0)
    , clearColor(Qt::white)
    , clearBeforeRendering(true)
    , persistentGLContext(true)
    , persistentSceneGraph(true)
    , lastWheelEventAccepted(false)
    , componentCompleted(true)
    , allowChildEventFiltering(true)
    , allowDoubleClick(true)
    , incubationController(nullptr)
{
}

QDemonWindowPrivate::~QDemonWindowPrivate() {}

void QDemonWindowPrivate::init(QDemonWindow *c)
{
    q_ptr = c;

    Q_Q(QDemonWindow);

    contentItem = new QDemonNode();
    QQml_setParent_noEvent(contentItem, c);
    QQmlEngine::setObjectOwnership(contentItem, QQmlEngine::CppOwnership);
    QDemonObjectPrivate *contentItemPrivate = QDemonObjectPrivate::get(contentItem);
    contentItemPrivate->window = q;
    contentItemPrivate->windowRefCount = 1;
    // contentItemPrivate->flags |= QQuickItem::ItemIsFocusScope;
    // contentItem->setSize(q->size());

    windowManager = QDemonRenderLoop::instance();
    context = windowManager->sceneGraphContext().data();

    Q_ASSERT(windowManager);

    if (QScreen *screen = q->screen())
        devicePixelRatio = screen->devicePixelRatio();

    windowManager->addWindow(q);
    auto sg = windowManager->sceneGraphContext();
    auto renderContext = windowManager->renderContext();

    q->setSurfaceType(windowManager ? windowManager->windowSurfaceType() : QSurface::OpenGLSurface);
    q->setFormat(renderContext->format());

    m_presentation = new QDemonRenderPresentation();
    m_scene = new QDemonRenderScene();
    m_presentation->scene = m_scene;
    m_scene->presentation = m_presentation.data();
    m_presentation->presentationDimensions.setX(q->width());
    m_presentation->presentationDimensions.setY(q->height());

    //    animationController = new QQuickAnimatorController(q);

    //    QObject::connect(context, SIGNAL(initialized()), q, SIGNAL(sceneGraphInitialized()), Qt::DirectConnection);
    //    QObject::connect(context, SIGNAL(invalidated()), q, SIGNAL(sceneGraphInvalidated()), Qt::DirectConnection);
    //    QObject::connect(context, SIGNAL(invalidated()), q, SLOT(cleanupSceneGraph()), Qt::DirectConnection);

    QObject::connect(q, SIGNAL(focusObjectChanged(QObject *)), q, SIGNAL(activeFocusItemChanged()));
    QObject::connect(q, SIGNAL(screenChanged(QScreen *)), q, SLOT(handleScreenChanged(QScreen *)));
    QObject::connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), q, SLOT(handleApplicationStateChanged(Qt::ApplicationState)));
    QObject::connect(q, SIGNAL(frameSwapped()), q, SLOT(runJobsAfterSwap()), Qt::DirectConnection);
}

QQmlListProperty<QObject> QDemonWindowPrivate::data()
{
    return QQmlListProperty<QObject>(q_func(),
                                     nullptr,
                                     QDemonWindowPrivate::data_append,
                                     QDemonWindowPrivate::data_count,
                                     QDemonWindowPrivate::data_at,
                                     QDemonWindowPrivate::data_clear);
}

void QDemonWindowPrivate::deliverKeyEvent(QKeyEvent *) {}

void QDemonWindowPrivate::dirtyItem(QDemonObject *)
{
    Q_Q(QDemonWindow);
    q->maybeUpdate();
}

void QDemonWindowPrivate::cleanup(QDemonRenderGraphObject *n)
{
    Q_Q(QDemonWindow);

    Q_ASSERT(!cleanupNodeList.contains(n));
    cleanupNodeList.append(n);
    q->maybeUpdate();
}

void QDemonWindowPrivate::polishItems()
{
    // An item can trigger polish on another item, or itself for that matter,
    // during its updatePolish() call. Because of this, we cannot simply
    // iterate through the set, we must continue pulling items out until it
    // is empty.
    // In the case where polish is called from updatePolish() either directly
    // or indirectly, we use a recursionSafeguard to print a warning to
    // the user.
    int recursionSafeguard = INT_MAX;
    //    while (!itemsToPolish.isEmpty() && --recursionSafeguard > 0) {
    //        QDemonObject *item = itemsToPolish.takeLast();
    //        QDemonObjectPrivate *itemPrivate = QDemonObjectPrivate::get(item);
    //        itemPrivate->polishScheduled = false;
    //        itemPrivate->updatePolish();
    //        item->updatePolish();
    //    }

    if (recursionSafeguard == 0)
        qWarning("QDemonWindow: possible QDemonObject::polish() loop");
}

void forcePolishHelper(QDemonObject *item)
{
    // item->polish();

    QList<QDemonObject *> items = item->childItems();
    for (int i = 0; i < items.size(); ++i)
        forcePolishHelper(items.at(i));
}

void QDemonWindowPrivate::forcePolish()
{
    Q_Q(QDemonWindow);
    if (!q->screen())
        return;
    forcePolishHelper(contentItem);
}

void QDemonWindowPrivate::syncSceneGraph()
{
    Q_Q(QDemonWindow);

    // animationController->beforeNodeSync();

    emit q->beforeSynchronizing();
    runAndClearJobs(&beforeSynchronizingJobs);
    //    if (!renderer) {
    //        forceUpdate(contentItem);

    //        QSGRootNode *rootNode = new QSGRootNode;
    //        rootNode->appendChildNode(QDemonObjectPrivate::get(contentItem)->itemNode());
    //        renderer = context->createRenderer();
    //        renderer->setRootNode(rootNode);
    //    }

    updateDirtyNodes();

    // animationController->afterNodeSync();

    // Copy the current state of clearing from window into renderer.
    context->setSceneColor(QVector4D(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF()));
    emit q->afterSynchronizing();
    runAndClearJobs(&afterSynchronizingJobs);
}

void QDemonWindowPrivate::renderSceneGraph(const QSize &size)
{
    Q_Q(QDemonWindow);

    // animationController->advance();
    emit q->beforeRendering();
    runAndClearJobs(&beforeRenderingJobs);

    context->setPresentationDimensions(size * q->effectiveDevicePixelRatio());
    context->beginFrame();
    windowManager->renderContext()->resetBlendState();

    auto lastRenderViewport = context->getRenderList()->getViewport();
    if (m_presentation && m_presentation->scene) {
        QRect theViewportSize(lastRenderViewport);
        m_presentation->scene->prepareForRender(QVector2D(theViewportSize.width(), theViewportSize.height()), context);
    }

    context->runRenderTasks();
    if (m_presentation && m_presentation->scene) {
        QRect theViewportSize(lastRenderViewport);
        m_presentation->scene->render(QVector2D(theViewportSize.width(), theViewportSize.height()), context, QDemonRenderScene::DoNotClear);
    }

    context->endFrame();

    //    if (!customRenderStage || !customRenderStage->render()) {
    //        int fboId = 0;
    //        const qreal devicePixelRatio = q->effectiveDevicePixelRatio();
    //        if (renderTargetId) {
    //            QRect rect(QPoint(0, 0), renderTargetSize);
    //            fboId = renderTargetId;
    //            renderer->setDeviceRect(rect);
    //            renderer->setViewportRect(rect);
    //            if (QQuickRenderControl::renderWindowFor(q)) {
    //                renderer->setProjectionMatrixToRect(QRect(QPoint(0, 0), size));
    //                renderer->setDevicePixelRatio(devicePixelRatio);
    //            } else {
    //                renderer->setProjectionMatrixToRect(QRect(QPoint(0, 0), rect.size()));
    //                renderer->setDevicePixelRatio(1);
    //            }
    //        } else {
    //            QRect rect(QPoint(0, 0), devicePixelRatio * size);
    //            renderer->setDeviceRect(rect);
    //            renderer->setViewportRect(rect);
    //            renderer->setProjectionMatrixToRect(QRect(QPoint(0, 0), size));
    //            renderer->setDevicePixelRatio(devicePixelRatio);
    //        }

    //        context->renderNextFrame(renderer, fboId);
    //    }
    emit q->afterRendering();
    runAndClearJobs(&afterRenderingJobs);
}

bool QDemonWindowPrivate::isRenderable() const
{
    Q_Q(const QDemonWindow);
    return ((q->isExposed() && q->isVisible())) && q->geometry().isValid();
}

void QDemonWindowPrivate::updateDirtyNodes()
{
    cleanupNodes();

    auto updateNodes = [this](QDemonObject *updateList) {
        if (updateList)
            QDemonObjectPrivate::get(updateList)->prevDirtyItem = &updateList;

        while (updateList) {
            QDemonObject *item = updateList;
            QDemonObjectPrivate *itemPriv = QDemonObjectPrivate::get(item);
            itemPriv->removeFromDirtyList();

            updateDirtyNode(item);
        }
    };

    updateNodes(dirtyImageList);
    updateNodes(dirtyResourceList);
    updateNodes(dirtySpatialNodeList);

    dirtyImageList = nullptr;
    dirtyResourceList = nullptr;
    dirtySpatialNodeList = nullptr;

}

void QDemonWindowPrivate::cleanupNodes()
{
    for (int ii = 0; ii < cleanupNodeList.count(); ++ii) {
        QDemonRenderGraphObject *node = cleanupNodeList.at(ii);
        // Different processing for resource nodes vs hierarchical nodes
        switch (node->type) {
        case QDemonRenderGraphObject::Type::Layer: {
            QDemonRenderLayer *layerNode = static_cast<QDemonRenderLayer *>(node);
            // remove layer from scene
            m_scene->removeChild(*layerNode);
        } break;
        case QDemonRenderGraphObject::Type::Node:
        case QDemonRenderGraphObject::Type::Light:
        case QDemonRenderGraphObject::Type::Camera:
        case QDemonRenderGraphObject::Type::Model:
        case QDemonRenderGraphObject::Type::Text:
        case QDemonRenderGraphObject::Type::Path: {
            // handle hierarchical nodes
            QDemonRenderNode *spatialNode = static_cast<QDemonRenderNode *>(node);
            spatialNode->removeFromGraph();
        } break;
        case QDemonRenderGraphObject::Type::Presentation:
        case QDemonRenderGraphObject::Type::Scene:
        case QDemonRenderGraphObject::Type::DefaultMaterial:
        case QDemonRenderGraphObject::Type::Image:
        case QDemonRenderGraphObject::Type::Effect:
        case QDemonRenderGraphObject::Type::CustomMaterial:
        case QDemonRenderGraphObject::Type::ReferencedMaterial:
        case QDemonRenderGraphObject::Type::PathSubPath:
        case QDemonRenderGraphObject::Type::Lightmaps:
            // handle resource nodes
            // ### Handle the case where we are referenced by another node
            break;
        default:
            // we dont need to do anything with the other nodes
            break;
        }

        delete node;
    }
    cleanupNodeList.clear();
}

void QDemonWindowPrivate::cleanupNodesOnShutdown()
{
    Q_Q(QDemonWindow);
    cleanupNodes();
    cleanupNodesOnShutdown(contentItem);
    for (QSet<QDemonObject *>::const_iterator it = parentlessItems.begin(), cend = parentlessItems.end(); it != cend; ++it)
        cleanupNodesOnShutdown(*it);
    // animationController->windowNodesDestroyed();
    q->cleanupSceneGraph();
}

void QDemonWindowPrivate::cleanupNodesOnShutdown(QDemonObject *item)
{
    //    QDemonObjectPrivate *p = QDemonObjectPrivate::get(item);
    //    if (p->itemNodeInstance) {
    //        delete p->itemNodeInstance;
    //        p->itemNodeInstance = nullptr;

    //        if (p->extra.isAllocated()) {
    //            p->extra->opacityNode = nullptr;
    //            p->extra->clipNode = nullptr;
    //            p->extra->rootNode = nullptr;
    //        }

    //        p->paintNode = nullptr;

    //        p->dirty(QDemonObjectPrivate::Window);
    //    }

    //    const QMetaObject *mo = item->metaObject();
    //    int index = mo->indexOfSlot("invalidateSceneGraph()");
    //    if (index >= 0) {
    //        const QMetaMethod &method = mo->method(index);
    //        // Skip functions named invalidateSceneGraph() in QML items.
    //        if (strstr(method.enclosingMetaObject()->className(), "_QML_") == nullptr)
    //            method.invoke(item, Qt::DirectConnection);
    //    }

    //    for (int ii = 0; ii < p->childItems.count(); ++ii)
    //        cleanupNodesOnShutdown(p->childItems.at(ii));
}

bool QDemonWindowPrivate::updateEffectiveOpacity(QDemonObject *)
{
    return false;
}

void QDemonWindowPrivate::updateEffectiveOpacityRoot(QDemonObject *, qreal) {}

// ### this is where all the magic happens
void QDemonWindowPrivate::updateDirtyNode(QDemonObject *item)
{
    // Different processing for resource nodes vs hierarchical nodes
    switch (item->type()) {
    case QDemonObject::Layer: {
        QDemonLayer *layerNode = qobject_cast<QDemonLayer *>(item);
        if (layerNode)
            updateDirtyLayer(layerNode);
    } break;
    case QDemonObject::Node:
    case QDemonObject::Light:
    case QDemonObject::Camera:
    case QDemonObject::Model:
    case QDemonObject::Text:
    case QDemonObject::Path: {
        // handle hierarchical nodes
        QDemonNode *spatialNode = qobject_cast<QDemonNode *>(item);
        if (spatialNode)
            updateDirtySpatialNode(spatialNode);
    } break;
    case QDemonObject::Presentation:
    case QDemonObject::Scene:
    case QDemonObject::DefaultMaterial:
    case QDemonObject::Image:
    case QDemonObject::Effect:
    case QDemonObject::CustomMaterial:
    case QDemonObject::ReferencedMaterial:
    case QDemonObject::PathSubPath:
    case QDemonObject::Lightmaps:
        // handle resource nodes
        updateDirtyResource(item);
        break;
    default:
        // we dont need to do anything with the other nodes
        break;
    }

    //    if ((dirty & QDemonObjectPrivate::TransformUpdateMask) ||
    //        (dirty & QDemonObjectPrivate::Size && itemPriv->origin() != QQuickItem::TopLeft &&
    //         (itemPriv->scale() != 1. || itemPriv->rotation() != 0.))) {

    //        QMatrix4x4 matrix;

    //        if (itemPriv->x != 0. || itemPriv->y != 0.)
    //            matrix.translate(itemPriv->x, itemPriv->y);

    //        for (int ii = itemPriv->transforms.count() - 1; ii >= 0; --ii)
    //            itemPriv->transforms.at(ii)->applyTo(&matrix);

    //        if (itemPriv->scale() != 1. || itemPriv->rotation() != 0.) {
    //            QPointF origin = item->transformOriginPoint();
    //            matrix.translate(origin.x(), origin.y());
    //            if (itemPriv->scale() != 1.)
    //                matrix.scale(itemPriv->scale(), itemPriv->scale());
    //            if (itemPriv->rotation() != 0.)
    //                matrix.rotate(itemPriv->rotation(), 0, 0, 1);
    //            matrix.translate(-origin.x(), -origin.y());
    //        }

    //        itemPriv->itemNode()->setMatrix(matrix);
    //    }

    //    bool clipEffectivelyChanged = (dirty & (QDemonObjectPrivate::Clip | QDemonObjectPrivate::Window)) &&
    //                                  ((item->clip() == false) != (itemPriv->clipNode() == nullptr));
    //    int effectRefCount = itemPriv->extra.isAllocated()?itemPriv->extra->effectRefCount:0;
    //    bool effectRefEffectivelyChanged = (dirty & (QDemonObjectPrivate::EffectReference | QDemonObjectPrivate::Window)) &&
    //                                  ((effectRefCount == 0) != (itemPriv->rootNode() == nullptr));

    //    if (clipEffectivelyChanged) {
    //        QSGNode *parent = itemPriv->opacityNode() ? (QSGNode *) itemPriv->opacityNode() :
    //                                                    (QSGNode *) itemPriv->itemNode();
    //        QSGNode *child = itemPriv->rootNode();

    //        if (item->clip()) {
    //            Q_ASSERT(itemPriv->clipNode() == nullptr);
    //            QQuickDefaultClipNode *clip = new QQuickDefaultClipNode(item->clipRect());
    //            itemPriv->extra.value().clipNode = clip;
    //            clip->update();

    //            if (!child) {
    //                parent->reparentChildNodesTo(clip);
    //                parent->appendChildNode(clip);
    //            } else {
    //                parent->removeChildNode(child);
    //                clip->appendChildNode(child);
    //                parent->appendChildNode(clip);
    //            }

    //        } else {
    //            QQuickDefaultClipNode *clip = itemPriv->clipNode();
    //            Q_ASSERT(clip);
    //            parent->removeChildNode(clip);
    //            if (child) {
    //                clip->removeChildNode(child);
    //                parent->appendChildNode(child);
    //            } else {
    //                clip->reparentChildNodesTo(parent);
    //            }

    //            delete itemPriv->clipNode();
    //            itemPriv->extra->clipNode = nullptr;
    //        }
    //    }

    //    if (effectRefEffectivelyChanged) {
    //        if (dirty & QDemonObjectPrivate::ChildrenUpdateMask)
    //            itemPriv->childContainerNode()->removeAllChildNodes();

    //        QSGNode *parent = itemPriv->clipNode();
    //        if (!parent)
    //            parent = itemPriv->opacityNode();
    //        if (!parent)
    //            parent = itemPriv->itemNode();

    //        if (itemPriv->extra.isAllocated() && itemPriv->extra->effectRefCount) {
    //            Q_ASSERT(itemPriv->rootNode() == nullptr);
    //            QSGRootNode *root = new QSGRootNode();
    //            itemPriv->extra->rootNode = root;
    //            parent->reparentChildNodesTo(root);
    //            parent->appendChildNode(root);
    //        } else {
    //            Q_ASSERT(itemPriv->rootNode() != nullptr);
    //            QSGRootNode *root = itemPriv->rootNode();
    //            parent->removeChildNode(root);
    //            root->reparentChildNodesTo(parent);
    //            delete itemPriv->rootNode();
    //            itemPriv->extra->rootNode = nullptr;
    //        }
    //    }

    //    if (dirty & QDemonObjectPrivate::ChildrenUpdateMask) {
    //        int ii = 0;
    //        bool fetchedPaintNode = false;
    //        QList<QQuickItem *> orderedChildren = itemPriv->paintOrderChildItems();
    //        int desiredNodesSize = orderedChildren.size() + (itemPriv->paintNode ? 1 : 0);

    //        // now start making current state match the promised land of
    //        // desiredNodes. in the case of our current state matching desiredNodes
    //        // (though why would we get ChildrenUpdateMask with no changes?) then we
    //        // should make no changes at all.

    //        // how many nodes did we process, when examining changes
    //        int desiredNodesProcessed = 0;

    //        // currentNode is how far, in our present tree, we have processed. we
    //        // make use of this later on to trim the current child list if the
    //        // desired list is shorter.
    //        QSGNode *groupNode = itemPriv->childContainerNode();
    //        QSGNode *currentNode = groupNode->firstChild();
    //        int added = 0;
    //        int removed = 0;
    //        int replaced = 0;
    //        QSGNode *desiredNode = nullptr;

    //        while (currentNode && (desiredNode = fetchNextNode(itemPriv, ii, fetchedPaintNode))) {
    //            // uh oh... reality and our utopic paradise are diverging!
    //            // we need to reconcile this...
    //            if (currentNode != desiredNode) {
    //                // for now, we're just removing the node from the children -
    //                // and replacing it with the new node.
    //                if (desiredNode->parent())
    //                    desiredNode->parent()->removeChildNode(desiredNode);
    //                groupNode->insertChildNodeAfter(desiredNode, currentNode);
    //                groupNode->removeChildNode(currentNode);
    //                replaced++;

    //                // since we just replaced currentNode, we also need to reset
    //                // the pointer.
    //                currentNode = desiredNode;
    //            }

    //            currentNode = currentNode->nextSibling();
    //            desiredNodesProcessed++;
    //        }

    //        // if we didn't process as many nodes as in the new list, then we have
    //        // more nodes at the end of desiredNodes to append to our list.
    //        // this will be the case when adding new nodes, for instance.
    //        if (desiredNodesProcessed < desiredNodesSize) {
    //            while ((desiredNode = fetchNextNode(itemPriv, ii, fetchedPaintNode))) {
    //                if (desiredNode->parent())
    //                    desiredNode->parent()->removeChildNode(desiredNode);
    //                groupNode->appendChildNode(desiredNode);
    //                added++;
    //            }
    //        } else if (currentNode) {
    //            // on the other hand, if we processed less than our current node
    //            // tree, then nodes have been _removed_ from the scene, and we need
    //            // to take care of that here.
    //            while (currentNode) {
    //                QSGNode *node = currentNode->nextSibling();
    //                groupNode->removeChildNode(currentNode);
    //                currentNode = node;
    //                removed++;
    //            }
    //        }
    //    }

    //    if ((dirty & QDemonObjectPrivate::Size) && itemPriv->clipNode()) {
    //        itemPriv->clipNode()->setRect(item->clipRect());
    //        itemPriv->clipNode()->update();
    //    }

    //    if (dirty & (QDemonObjectPrivate::OpacityValue | QDemonObjectPrivate::Visible
    //                 | QDemonObjectPrivate::HideReference | QDemonObjectPrivate::Window))
    //    {
    //        qreal opacity = itemPriv->explicitVisible && (!itemPriv->extra.isAllocated() || itemPriv->extra->hideRefCount == 0)
    //                      ? itemPriv->opacity() : qreal(0);

    //        if (opacity != 1 && !itemPriv->opacityNode()) {
    //            QSGOpacityNode *node = new QSGOpacityNode;
    //            itemPriv->extra.value().opacityNode = node;

    //            QSGNode *parent = itemPriv->itemNode();
    //            QSGNode *child = itemPriv->clipNode();
    //            if (!child)
    //                child = itemPriv->rootNode();

    //            if (child) {
    //                parent->removeChildNode(child);
    //                node->appendChildNode(child);
    //                parent->appendChildNode(node);
    //            } else {
    //                parent->reparentChildNodesTo(node);
    //                parent->appendChildNode(node);
    //            }
    //        }
    //        if (itemPriv->opacityNode())
    //            itemPriv->opacityNode()->setOpacity(opacity);
    //    }

    //    if (dirty & QDemonObjectPrivate::ContentUpdateMask) {

    ////        if (itemPriv->flags & QQuickItem::ItemHasContents) {
    ////            updatePaintNodeData.transformNode = itemPriv->itemNode();
    ////            itemPriv->spatialNode = item->updateSpatialNode(itemPriv->spatialNode);

    ////            Q_ASSERT(itemPriv->paintNode == nullptr ||
    ////                     itemPriv->paintNode->parent() == nullptr ||
    ////                     itemPriv->paintNode->parent() == itemPriv->childContainerNode());

    //            if (itemPriv->spatialNode && itemPriv->spatialNode->parent() == nullptr) {
    //                QSGNode *before = qquickitem_before_paintNode(itemPriv);
    //                if (before && before->parent()) {
    //                    Q_ASSERT(before->parent() == itemPriv->childContainerNode());
    //                    itemPriv->childContainerNode()->insertChildNodeAfter(itemPriv->paintNode, before);
    //                } else {
    //                    itemPriv->childContainerNode()->prependChildNode(itemPriv->paintNode);
    //                }
    //            }
    //        }
    //    }

    //#ifndef QT_NO_DEBUG
    //    // Check consistency.

    //    QList<QSGNode *> nodes;
    //    nodes << itemPriv->itemNodeInstance
    //          << itemPriv->opacityNode()
    //          << itemPriv->clipNode()
    //          << itemPriv->rootNode()
    //          << itemPriv->paintNode;
    //    nodes.removeAll(0);

    //    Q_ASSERT(nodes.constFirst() == itemPriv->itemNodeInstance);
    //    for (int i=1; i<nodes.size(); ++i) {
    //        QSGNode *n = nodes.at(i);
    //        // Failing this means we messed up reparenting
    //        Q_ASSERT(n->parent() == nodes.at(i-1));
    //        // Only the paintNode and the one who is childContainer may have more than one child.
    //        Q_ASSERT(n == itemPriv->paintNode || n == itemPriv->childContainerNode() || n->childCount() == 1);
    //    }
    //#endif
}

void QDemonWindowPrivate::updateDirtyResource(QDemonObject *resourceObject)
{
    QDemonObjectPrivate *itemPriv = QDemonObjectPrivate::get(resourceObject);
    quint32 dirty = itemPriv->dirtyAttributes;
    itemPriv->dirtyAttributes = 0;
    itemPriv->spatialNode = resourceObject->updateSpatialNode(itemPriv->spatialNode);

    // resource nodes dont go in the tree, so we dont need to parent them
}

void QDemonWindowPrivate::updateDirtySpatialNode(QDemonNode *spatialNode)
{
    QDemonObjectPrivate *itemPriv = QDemonObjectPrivate::get(spatialNode);
    quint32 dirty = itemPriv->dirtyAttributes;
    itemPriv->dirtyAttributes = 0;
    itemPriv->spatialNode = spatialNode->updateSpatialNode(itemPriv->spatialNode);

    QDemonRenderNode *graphNode = static_cast<QDemonRenderNode *>(itemPriv->spatialNode);

    if (graphNode && graphNode->parent == nullptr) {
        QDemonNode *nodeParent = qobject_cast<QDemonNode *>(spatialNode->parent());
        if (nodeParent) {
            QDemonRenderNode *parentGraphNode = static_cast<QDemonRenderNode *>(QDemonObjectPrivate::get(nodeParent)->spatialNode);
            parentGraphNode->addChild(*graphNode);
        } else if (graphNode && spatialNode != contentItem) {
            Q_ASSERT(false);
        }
    }
}

void QDemonWindowPrivate::updateDirtyLayer(QDemonLayer *layerNode)
{
    QDemonObjectPrivate *itemPriv = QDemonObjectPrivate::get(layerNode);
    quint32 dirty = itemPriv->dirtyAttributes;
    itemPriv->dirtyAttributes = 0;

    itemPriv->spatialNode = layerNode->updateSpatialNode(itemPriv->spatialNode);

    QDemonRenderLayer *layer = static_cast<QDemonRenderLayer *>(itemPriv->spatialNode);

    if (!layer->scene)
        m_scene->addChild(*layer);
}

void QDemonWindowPrivate::data_append(QQmlListProperty<QObject> *property, QObject *o)
{
    if (!o)
        return;
    QDemonWindow *that = static_cast<QDemonWindow *>(property->object);
    if (QDemonWindow *window = qmlobject_cast<QDemonWindow *>(o)) {
        window->setTransientParent(that);
    }
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(that->contentItem())->data();
    itemProperty.append(&itemProperty, o);
}

int QDemonWindowPrivate::data_count(QQmlListProperty<QObject> *property)
{
    QDemonWindow *win = static_cast<QDemonWindow *>(property->object);
    if (!win || !win->contentItem() || !QDemonObjectPrivate::get(win->contentItem())->data().count)
        return 0;
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(win->contentItem())->data();
    return itemProperty.count(&itemProperty);
}

QObject *QDemonWindowPrivate::data_at(QQmlListProperty<QObject> *property, int i)
{
    QDemonWindow *win = static_cast<QDemonWindow *>(property->object);
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(win->contentItem())->data();
    return itemProperty.at(&itemProperty, i);
}

void QDemonWindowPrivate::data_clear(QQmlListProperty<QObject> *property)
{
    QDemonWindow *win = static_cast<QDemonWindow *>(property->object);
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(win->contentItem())->data();
    itemProperty.clear(&itemProperty);
}

void QDemonWindowPrivate::runAndClearJobs(QList<QRunnable *> *jobs)
{
    renderJobMutex.lock();
    QList<QRunnable *> jobList = *jobs;
    jobs->clear();
    renderJobMutex.unlock();

    for (QRunnable *r : qAsConst(jobList)) {
        r->run();
        delete r;
    }
}

QT_END_NAMESPACE

#include <moc_qdemonwindow.cpp>
