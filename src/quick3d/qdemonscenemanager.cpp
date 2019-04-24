#include "qdemonscenemanager_p.h"
#include "qdemonobject_p.h"
#include "qdemonview3d.h"

#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
QT_BEGIN_NAMESPACE

QDemonSceneManager::QDemonSceneManager(QObject *parent)
    : QObject(parent)
    , dirtySpatialNodeList(nullptr)
    , dirtyResourceList(nullptr)
    , dirtyImageList(nullptr)
{
}

void QDemonSceneManager::dirtyItem(QDemonObject *item)
{
    Q_UNUSED(item)
    emit needsUpdate();
}

void QDemonSceneManager::cleanup(QDemonRenderGraphObject *item)
{
    Q_ASSERT(!cleanupNodeList.contains(item));
    cleanupNodeList.append(item);
}

void QDemonSceneManager::polishItems()
{

}

void QDemonSceneManager::forcePolish()
{

}

void QDemonSceneManager::sync()
{

}

void QDemonSceneManager::updateDirtyNodes()
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

void QDemonSceneManager::updateDirtyNode(QDemonObject *object)
{
    // Different processing for resource nodes vs hierarchical nodes
    switch (object->type()) {
//    case QDemonObject::Layer: {
//        QDemonLayer *layerNode = qobject_cast<QDemonLayer *>(object);
//        if (layerNode)
//            updateDirtyLayer(layerNode);
//    } break;
    case QDemonObject::Node:
    case QDemonObject::Light:
    case QDemonObject::Camera:
    case QDemonObject::Model:
    case QDemonObject::Text:
    case QDemonObject::Path: {
        // handle hierarchical nodes
        QDemonNode *spatialNode = qobject_cast<QDemonNode *>(object);
        if (spatialNode)
            updateDirtySpatialNode(spatialNode);
    } break;
    case QDemonObject::SceneEnvironment:
    case QDemonObject::DefaultMaterial:
    case QDemonObject::Image:
    case QDemonObject::Effect:
    case QDemonObject::CustomMaterial:
    case QDemonObject::ReferencedMaterial:
    case QDemonObject::PathSubPath:
    case QDemonObject::Lightmaps:
        // handle resource nodes
        updateDirtyResource(object);
        break;
    default:
        // we dont need to do anything with the other nodes
        break;
    }
}

void QDemonSceneManager::updateDirtyResource(QDemonObject *resourceObject)
{
    QDemonObjectPrivate *itemPriv = QDemonObjectPrivate::get(resourceObject);
    quint32 dirty = itemPriv->dirtyAttributes;
    itemPriv->dirtyAttributes = 0;
    itemPriv->spatialNode = resourceObject->updateSpatialNode(itemPriv->spatialNode);

    // resource nodes dont go in the tree, so we dont need to parent them
}

void QDemonSceneManager::updateDirtySpatialNode(QDemonNode *spatialNode)
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
        } else {
            QDemonView3D *viewParent = qobject_cast<QDemonView3D *>(spatialNode->parent());
            if (viewParent) {
                auto sceneRoot = QDemonObjectPrivate::get(viewParent->scene());
                if (!sceneRoot->spatialNode) {
                    // must have a sceen root spatial node first
                    sceneRoot->spatialNode = viewParent->scene()->updateSpatialNode(sceneRoot->spatialNode);
                }
                static_cast<QDemonRenderNode *>(sceneRoot->spatialNode)->addChild(*graphNode);
            }
        }
    }
}

//void QDemonSceneRenderer::updateDirtyLayer(QDemonLayer *layerNode)
//{
//    QDemonObjectPrivate *itemPriv = QDemonObjectPrivate::get(layerNode);
//    quint32 dirty = itemPriv->dirtyAttributes;
//    itemPriv->dirtyAttributes = 0;

//    itemPriv->spatialNode = layerNode->updateSpatialNode(itemPriv->spatialNode);

//    QDemonRenderLayer *layer = static_cast<QDemonRenderLayer *>(itemPriv->spatialNode);

//    if (!layer->scene)
//        m_scene->addChild(*layer);
//}

void QDemonSceneManager::cleanupNodes()
{
    for (int ii = 0; ii < cleanupNodeList.count(); ++ii) {
        QDemonRenderGraphObject *node = cleanupNodeList.at(ii);
        // Different processing for resource nodes vs hierarchical nodes
        switch (node->type) {
//        case QDemonRenderGraphObject::Type::Layer: {
//            QDemonRenderLayer *layerNode = static_cast<QDemonRenderLayer *>(node);
//            // remove layer from scene
//            m_scene->removeChild(*layerNode);
//        } break;
        case QDemonRenderGraphObject::Type::Node:
        case QDemonRenderGraphObject::Type::Light:
        case QDemonRenderGraphObject::Type::Camera:
        case QDemonRenderGraphObject::Type::Model:
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

QT_END_NAMESPACE
