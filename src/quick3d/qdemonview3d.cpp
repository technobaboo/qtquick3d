#include "qdemonview3d.h"
#include "qdemonview3d_p.h"
#include "qdemonsceneenvironment.h"
#include "qdemonobject_p.h"
#include "qdemonscenemanager_p.h"
#include "qdemonsgrendernode_p.h"
#include "qdemonimage.h"
#include <QtDemonRuntimeRender/QDemonRenderLayer>

QT_BEGIN_NAMESPACE

QDemonView3DPrivate::QDemonView3DPrivate()
{
    Q_Q(QDemonView3D);
    environment = new QDemonSceneEnvironment(q);
    camera = nullptr;
    sceneRoot = new QDemonNode();
    QDemonObjectPrivate::get(sceneRoot)->sceneRenderer = new QDemonSceneManager(sceneRoot);
    sceneRoot->update();
}

QDemonView3DPrivate::~QDemonView3DPrivate()
{

}

void QDemonView3DPrivate::createRenderer()
{

}

QSGNode *QDemonView3DPrivate::createNode()
{
    return nullptr;

}

void QDemonView3DPrivate::sync()
{

}

QDemonView3D::QDemonView3D(QQuickItem *parent)
    : QQuickItem(*(new QDemonView3DPrivate), parent)
{
    setFlag(ItemHasContents);
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
    const Q_D(QDemonView3D);
    return d->camera;
}

QDemonSceneEnvironment *QDemonView3D::environment() const
{
    const Q_D(QDemonView3D);
    return d->environment;
}

QDemonNode *QDemonView3D::scene() const
{
    const Q_D(QDemonView3D);
    return d->sceneRoot;
}

void QDemonView3D::setCamera(QDemonCamera *camera)
{
    Q_D(QDemonView3D);
    if (d->camera == camera)
        return;

    d->camera = camera;
    emit cameraChanged(d->camera);
}

void QDemonView3D::setEnvironment(QDemonSceneEnvironment *environment)
{
    Q_D(QDemonView3D);
    if (d->environment == environment)
        return;

    d->environment = environment;
    emit environmentChanged(d->environment);
}

void QDemonView3D::setScene(QDemonNode *sceneRoot)
{
    Q_D(QDemonView3D);
    // ### We may need consider the case where there is
    // already a scene tree here
    if (d->referencedScene) {
        // ### remove referenced tree
    }
    d->referencedScene = sceneRoot;
    // ### add referenced tree
}

QSGNode *QDemonView3D::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    Q_D(QDemonView3D);

    if (d->sceneRoot && d->sceneRoot->sceneRenderer()) {
        d->sceneRoot->sceneRenderer()->updateDirtyNodes();

        auto renderNode = static_cast<QDemonSGRenderNode*>(node);

        if (!renderNode)
            renderNode = new QDemonSGRenderNode(d->sceneRoot->sceneRenderer());

        // Create / Update Layer and set it on the render node
        if (!d->layerNode) {
            d->layerNode = new QDemonRenderLayer();
        }

        updateLayerNode();
        auto rootNode = static_cast<QDemonRenderNode*>(QDemonObjectPrivate::get(d->sceneRoot)->spatialNode);
        if (rootNode) {
            if (d->layerNode->firstChild != nullptr && d->layerNode->firstChild != rootNode)
                d->layerNode->removeChild(*d->layerNode->firstChild);
            else
                d->layerNode->addChild(*rootNode);
        }
        renderNode->setRenderLayer(d->layerNode);
        node = renderNode;
    }

    return node;
}

void QDemonView3D::updatePolish()
{

}

void QDemonView3D::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{

}

void QDemonView3D::componentComplete()
{
    QQuickItem::componentComplete();
}

void QDemonView3D::classBegin()
{
    QQuickItem::classBegin();
}

void QDemonView3D::updateLayerNode()
{
    Q_D(QDemonView3D);
    QDemonRenderLayer *layerNode = d->layerNode;
    layerNode->progressiveAAMode = QDemonRenderLayer::AAMode(d->environment->progressiveAAMode());
    layerNode->multisampleAAMode = QDemonRenderLayer::AAMode(d->environment->multisampleAAMode());
    layerNode->temporalAAEnabled = d->environment->temporalAAEnabled();

    layerNode->background = QDemonRenderLayer::Background(d->environment->backgroundMode());
    layerNode->clearColor = QVector3D(d->environment->clearColor().redF(), d->environment->clearColor().greenF(), d->environment->clearColor().blueF());
    layerNode->blendType = QDemonRenderLayer::BlendMode(d->environment->blendType());

    // Set all units to pixels
    layerNode->m_width = float(width());
    layerNode->m_height = float(height());
    layerNode->widthUnits = QDemonRenderLayer::UnitType::Pixels;
    layerNode->heightUnits = QDemonRenderLayer::UnitType::Pixels;

    layerNode->m_left = float(x());
    layerNode->m_top = float(y());
    layerNode->leftUnits = QDemonRenderLayer::UnitType::Pixels;
    layerNode->topUnits = QDemonRenderLayer::UnitType::Pixels;

    // ## Maybe the rest of anchors as well

    layerNode->aoStrength = d->environment->aoStrength();
    layerNode->aoDistance = d->environment->aoDistance();
    layerNode->aoSoftness = d->environment->aoSoftness();
    layerNode->aoBias = d->environment->aoBias();
    layerNode->aoSamplerate = d->environment->aoSampleRate();
    layerNode->aoDither = d->environment->aoDither();


    layerNode->shadowStrength = d->environment->shadowStrength();
    layerNode->shadowDist = d->environment->shadowDistance();
    layerNode->shadowSoftness = d->environment->shadowSoftness();
    layerNode->shadowBias = d->environment->shadowBias();

    // ### These images will not be registered anywhere
    if (d->environment->lightProbe())
        layerNode->lightProbe = d->environment->lightProbe()->getRenderImage();
    else
        layerNode->lightProbe = nullptr;

    layerNode->probeBright = d->environment->probeBrightness();
    layerNode->fastIbl = d->environment->fastIBL();
    layerNode->probeHorizon = d->environment->probeHorizon();
    layerNode->probeFov = d->environment->probeFieldOfView();


    if (d->environment->lightProbe2())
        layerNode->lightProbe2 = d->environment->lightProbe()->getRenderImage();
    else
        layerNode->lightProbe2 = nullptr;

    layerNode->probe2Fade = d->environment->probe2Fade();
    layerNode->probe2Window = d->environment->probe2Window();
    layerNode->probe2Pos = d->environment->probe2Postion();
}

QT_END_NAMESPACE
