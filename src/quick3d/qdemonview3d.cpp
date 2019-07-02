#include "qdemonview3d.h"
#include "qdemonsceneenvironment.h"
#include "qdemonobject_p.h"
#include "qdemonscenemanager_p.h"
#include "qdemonimage.h"
#include "qdemonscenerenderer.h"
#include "qdemoncamera.h"
#include <QtDemonRuntimeRender/QDemonRenderLayer>
#include <QOpenGLFunctions>

#include <qsgtextureprovider.h>
#include <QSGSimpleTextureNode>
#include <QSGRendererInterface>
#include <QQuickWindow>
#include <QtQuick/private/qquickitem_p.h>

#include <QtQml>

QT_BEGIN_NAMESPACE

//static void updateProperyListener(QDemonObject *newO, QDemonObject *oldO, QDemonSceneManager *manager, QHash<QObject*, QMetaObject::Connection> &connections, std::function<void(QDemonObject *o)> callFn) {
//    // disconnect previous destruction listern
//    if (oldO) {
//        if (manager)
//            QDemonObjectPrivate::get(oldO)->derefSceneRenderer();

//        auto connection = connections.find(oldO);
//        if (connection != connections.end()) {
//            QObject::disconnect(connection.value());
//            connections.erase(connection);
//        }
//    }

//    // listen for new map's destruction
//    if (newO) {
//        if (manager)
//            QDemonObjectPrivate::get(newO)->refSceneRenderer(manager);
//        auto connection = QObject::connect(newO, &QObject::destroyed, [callFn](){
//            callFn(nullptr);
//        });
//        connections.insert(newO, connection);
//    }
//}

QDemonView3D::QDemonView3D(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    m_camera = nullptr;
    m_sceneRoot = new QDemonNode();
    m_environment = new QDemonSceneEnvironment(m_sceneRoot);
    QDemonObjectPrivate::get(m_sceneRoot)->sceneManager = new QDemonSceneManager(m_sceneRoot);
    connect(QDemonObjectPrivate::get(m_sceneRoot)->sceneManager, &QDemonSceneManager::needsUpdate,
            this, &QQuickItem::update);
}

QDemonView3D::~QDemonView3D()
{
    for (const auto &connection : qAsConst(m_connections))
        disconnect(connection);
}

static void ssgn_append(QQmlListProperty<QObject> *property, QObject *obj)
{
    if (!obj)
        return;
    QDemonView3D *view3d = static_cast<QDemonView3D *>(property->object);
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(view3d->scene())->data();
    itemProperty.append(&itemProperty, obj);
}

static int ssgn_count(QQmlListProperty<QObject> *property)
{
    QDemonView3D *view3d = static_cast<QDemonView3D *>(property->object);
    if (!view3d || !view3d->scene() || !QDemonObjectPrivate::get(view3d->scene())->data().count)
        return 0;
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(view3d->scene())->data();
    return itemProperty.count(&itemProperty);
}

static QObject *ssgn_at(QQmlListProperty<QObject> *property, int i)
{
    QDemonView3D *view3d = static_cast<QDemonView3D *>(property->object);
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(view3d->scene())->data();
    return itemProperty.at(&itemProperty, i);
}

static void ssgn_clear(QQmlListProperty<QObject> *property)
{
    QDemonView3D *view3d = static_cast<QDemonView3D *>(property->object);
    QQmlListProperty<QObject> itemProperty = QDemonObjectPrivate::get(view3d->scene())->data();
    return itemProperty.clear(&itemProperty);
}


QQmlListProperty<QObject> QDemonView3D::data()
{
    return QQmlListProperty<QObject>(this,
                                     nullptr,
                                     ssgn_append,
                                     ssgn_count,
                                     ssgn_at,
                                     ssgn_clear);
}

QDemonCamera *QDemonView3D::camera() const
{
    return m_camera;
}

QDemonSceneEnvironment *QDemonView3D::environment() const
{
    return m_environment;
}

QDemonNode *QDemonView3D::scene() const
{
    return m_sceneRoot;
}

QDemonNode *QDemonView3D::referencedScene() const
{
    return m_referencedScene;
}

QDemonView3D::QDemonView3DRenderMode QDemonView3D::renderMode() const
{
    return m_renderMode;
}

QDemonSceneRenderer *QDemonView3D::createRenderer() const
{
    return new QDemonSceneRenderer(this->window());
}

bool QDemonView3D::isTextureProvider() const
{
    // We can only be a texture provider if we are rendering to a texture first
    if (m_renderMode == QDemonView3D::Texture)
        return true;

    return false;
}

QSGTextureProvider *QDemonView3D::textureProvider() const
{
    // When Item::layer::enabled == true, QQuickItem will be a texture
    // provider. In this case we should prefer to return the layer rather
    // than the fbo texture.
    if (QQuickItem::isTextureProvider())
        return QQuickItem::textureProvider();

    // We can only be a texture provider if we are rendering to a texture first
    if (m_renderMode != QDemonView3D::Texture)
        return nullptr;

    QQuickWindow *w = window();
    if (!w || !w->openglContext() || QThread::currentThread() != w->openglContext()->thread()) {
        qWarning("QDemonView3D::textureProvider: can only be queried on the rendering thread of an exposed window");
        return nullptr;
    }
    if (!m_node)
        m_node = new SGFramebufferObjectNode;
    return m_node;
}

void QDemonView3D::releaseResources()
{
    m_node = nullptr;
}

void QDemonView3D::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    if (newGeometry.size() != oldGeometry.size())
        update();
}

QSGNode *QDemonView3D::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    // When changing render modes
    if (m_renderModeDirty) {
        if (node) {
            delete node;
            node = nullptr;
        }
        if (m_directRenderer) {
            delete m_directRenderer;
            m_directRenderer = nullptr;
        }
    }


    m_renderModeDirty = false;

    if (m_renderMode == Texture) {
        SGFramebufferObjectNode *n = static_cast<SGFramebufferObjectNode *>(node);

        if (!n) {
            if (!m_node)
                m_node = new SGFramebufferObjectNode;
            n = m_node;
        }

        if (!n->renderer) {
            n->window = window();
            n->renderer = createRenderer();
            n->renderer->data = n;
            n->quickFbo = this;
            connect(window(), SIGNAL(screenChanged(QScreen*)), n, SLOT(handleScreenChange()));
        }
        QSize minFboSize = QQuickItemPrivate::get(this)->sceneGraphContext()->minimumFBOSize();
        QSize desiredFboSize(qMax<int>(minFboSize.width(), width()),
                             qMax<int>(minFboSize.height(), height()));

        n->devicePixelRatio = window()->effectiveDevicePixelRatio();
        desiredFboSize *= n->devicePixelRatio;

        n->renderer->synchronize(this, desiredFboSize);

        n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
        n->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
        n->setRect(0, 0, width(), height());

        n->scheduleRender();

        return n;
    } else if (m_renderMode == Underlay) {
        if (!m_directRenderer)
            m_directRenderer = new QDemonSGDirectRenderer(createRenderer(), window(), QDemonSGDirectRenderer::Underlay);
        const QSizeF targetSize = window()->effectiveDevicePixelRatio() * QSizeF(width(), height());
        m_directRenderer->renderer()->synchronize(this, targetSize.toSize(), false);
        m_directRenderer->setViewport(QRectF(window()->effectiveDevicePixelRatio() * mapToScene(QPointF(0, 0)), targetSize));
        m_directRenderer->requestRender();
        if (window()->clearBeforeRendering())
            window()->setClearBeforeRendering(false);
        window()->update();
        return node; // node should be nullptr
    } else if (m_renderMode == Overlay) {
        if (!m_directRenderer)
            m_directRenderer = new QDemonSGDirectRenderer(createRenderer(), window(), QDemonSGDirectRenderer::Overlay);
        const QSizeF targetSize = window()->effectiveDevicePixelRatio() * QSizeF(width(), height());
        m_directRenderer->renderer()->synchronize(this, targetSize.toSize(), false);
        m_directRenderer->setViewport(QRectF(window()->effectiveDevicePixelRatio() * mapToScene(QPointF(0, 0)), targetSize));
        m_directRenderer->requestRender();
        return node; // node should be nullptr
    } else {
        // Render Node
        QDemonSGRenderNode *n = static_cast<QDemonSGRenderNode *>(node);
        if (!n) {
            if (!m_renderNode)
                m_renderNode = new QDemonSGRenderNode();
            n = m_renderNode;
        }

        if (!n->renderer) {
            n->window = window();
            n->renderer = createRenderer();
            n->renderer->data = n;
        }

        const QSize targetSize = window()->effectiveDevicePixelRatio() * QSize(width(), height());

        n->renderer->synchronize(this, targetSize, false);
        n->markDirty(QSGNode::DirtyMaterial);

        return n;
    }
}

void QDemonView3D::setCamera(QDemonCamera *camera)
{
    if (m_camera == camera)
        return;

    m_camera = camera;
    emit cameraChanged(m_camera);
    update();
}

void QDemonView3D::setEnvironment(QDemonSceneEnvironment *environment)
{
    if (m_environment == environment)
        return;

    m_environment = environment;
    if (!m_environment->parentItem())
        m_environment->setParentItem(m_sceneRoot);
    emit environmentChanged(m_environment);
    update();
}

void QDemonView3D::setScene(QDemonNode *sceneRoot)
{
    // ### We may need consider the case where there is
    // already a scene tree here
    if (m_referencedScene) {
        // if there was previously a reference scene, disconnect
        if (!QDemonObjectPrivate::get(m_referencedScene)->sceneManager)
            disconnect(QDemonObjectPrivate::get(m_referencedScene)->sceneManager, &QDemonSceneManager::needsUpdate, this, &QQuickItem::update);
    }
    m_referencedScene = sceneRoot;
    if (m_referencedScene) {
        // If the referenced scene doesn't have a manager, add one (scenes defined outside of an view3d)
        auto privateObject = QDemonObjectPrivate::get(m_referencedScene);
        // ### BUG: This will probably leak, need to think harder about this
        if (!privateObject->sceneManager)
            privateObject->refSceneRenderer(new QDemonSceneManager(m_referencedScene));
        connect(QDemonObjectPrivate::get(m_referencedScene)->sceneManager, &QDemonSceneManager::needsUpdate, this, &QQuickItem::update);
    }

}

void QDemonView3D::setRenderMode(QDemonView3D::QDemonView3DRenderMode renderMode)
{
    if (m_renderMode == renderMode)
        return;

    m_renderMode = renderMode;
    m_renderModeDirty = true;
    emit renderModeChanged(m_renderMode);
    update();
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

static bool isBlackListedES3Driver(QOpenGLContext &ctx) {
    auto glFunctions = ctx.functions();
    static bool hasBeenTested = false;
    static bool result = false;
    if (!hasBeenTested) {
        QOffscreenSurface offscreenSurface;
        offscreenSurface.create();
        ctx.makeCurrent(&offscreenSurface);
        QString vendorString = QString::fromLatin1(reinterpret_cast<const char *>(glFunctions->glGetString(GL_RENDERER)));
        ctx.doneCurrent();
        if (vendorString == QStringLiteral("PowerVR Rogue GE8300"))
            result = true;
        hasBeenTested = true;
    }
    return result;
}


static QSurfaceFormat findIdealGLESVersion()
{
    QSurfaceFormat fmt;

    // Advanced: Try 3.1 (so we get compute shaders for instance)
    fmt.setVersion(3, 1);
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);
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
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 0) && !isBlackListedES3Driver(ctx)) {
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

QSurfaceFormat QDemonView3D::idealSurfaceFormat()
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

/*!
 * Transforms \a worldPos from world space into view space. The returned z value
 * will contain the distance from \l clipNear to \a worldPos in world units. If
 * the position is not visible in the viewport, a position of [-1, -1, -1] is
 * returned. This function requires that a camera is assigned to the view.
 *
 * \sa QDemonCamera::worldToViewport QDemonView3D::viewToWorld
 */
QVector3D QDemonView3D::worldToView(const QVector3D &worldPos) const
{
    if (!m_camera) {
        qmlWarning(this) << "Cannot resolve view position without a camera assigned!";
        return QVector3D(-1, -1, -1);
    }

    const QVector3D normalizedPos = m_camera->worldToViewport(worldPos);
    if (normalizedPos.x() < 0)
        return normalizedPos;
    return normalizedPos * QVector3D(float(width()), float(height()), 1);
}

/*!
 * Transforms \a viewPos from view space into world space. \a The x-, and y
 * values of \l viewPos should be within the width and height of the view.
 * The z value should be the distance from the camera into the world in world units. If
 * \a viewPos cannot be mapped to a position, a position of [-1, -1, -1] is returned.
 *
 * \sa QDemonCamera::viewportToWorld QDemonView3D::worldToView
 */
QVector3D QDemonView3D::viewToWorld(const QVector3D &viewPos) const
{
    if (!m_camera) {
        qmlWarning(this) << "Cannot resolve world position without a camera assigned!";
        return QVector3D(-1, -1, -1);
    }

    const QVector3D normalizedPos = viewPos / QVector3D(float(width()), float(height()), 1);
    return m_camera->viewportToWorld(normalizedPos);
}

void QDemonView3D::invalidateSceneGraph()
{
    m_node = nullptr;
}

QT_END_NAMESPACE
