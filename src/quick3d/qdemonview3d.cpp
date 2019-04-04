#include "qdemonview3d.h"
#include "qdemonsceneenvironment.h"
#include "qdemonobject_p.h"
#include "qdemonscenemanager_p.h"
#include "qdemonimage.h"
#include "qdemonscenerenderer.h"
#include <QtDemonRuntimeRender/QDemonRenderLayer>

#include <qsgtextureprovider.h>
#include <QSGSimpleTextureNode>
#include <QSGRendererInterface>
#include <QQuickWindow>
#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

QDemonView3D::QDemonView3D(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    m_environment = new QDemonSceneEnvironment(this);
    m_camera = nullptr;
    m_sceneRoot = new QDemonNode();
    QDemonObjectPrivate::get(m_sceneRoot)->sceneRenderer = new QDemonSceneManager(m_sceneRoot);
    connect(QDemonObjectPrivate::get(m_sceneRoot)->sceneRenderer, &QDemonSceneManager::needsUpdate,
            this, &QQuickItem::update);
}

QDemonView3D::~QDemonView3D()
{

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

QDemonSceneRenderer *QDemonView3D::createRenderer() const
{
    return new QDemonSceneRenderer();
}

bool QDemonView3D::isTextureProvider() const
{
    return true;
}

QSGTextureProvider *QDemonView3D::textureProvider() const
{
    // When Item::layer::enabled == true, QQuickItem will be a texture
    // provider. In this case we should prefer to return the layer rather
    // than the fbo texture.
    if (QQuickItem::isTextureProvider())
        return QQuickItem::textureProvider();

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
    SGFramebufferObjectNode *n = static_cast<SGFramebufferObjectNode *>(node);

    // We only abort if we never had a node before. This is so that we
    // don't recreate the renderer object if the thing becomes tiny. In
    // terms of API it would be horrible if the renderer would go away
    // that easily so with this logic, the renderer only goes away when
    // the scenegraph is invalidated or it is removed from the scene.
    if (!n && (width() <= 0 || height() <= 0))
        return nullptr;

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
        connect(window(), SIGNAL(beforeRendering()), n, SLOT(render()));
        connect(window(), SIGNAL(screenChanged(QScreen*)), n, SLOT(handleScreenChange()));
    }
    QSize minFboSize = QQuickItemPrivate::get(this)->sceneGraphContext()->minimumFBOSize();
    QSize desiredFboSize(qMax<int>(minFboSize.width(), width()),
                         qMax<int>(minFboSize.height(), height()));

    n->devicePixelRatio = window()->effectiveDevicePixelRatio();
    desiredFboSize *= n->devicePixelRatio;

    n->renderer->synchronize(this, desiredFboSize);

    n->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
    n->setRect(0, 0, width(), height());

    n->scheduleRender();

    return n;
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
    emit environmentChanged(m_environment);
    update();
}

void QDemonView3D::setScene(QDemonNode *sceneRoot)
{
    // ### We may need consider the case where there is
    // already a scene tree here
    if (m_referencedScene) {
        // ### remove referenced tree
    }
    m_referencedScene = sceneRoot;
    // ### add referenced tree
}

void QDemonView3D::invalidateSceneGraph()
{
    m_node = nullptr;
}

QT_END_NAMESPACE
