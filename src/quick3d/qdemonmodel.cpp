#include "qdemonmodel.h"

#include <QtDemonRuntimeRender/qdemonrendermodel.h>

QT_BEGIN_NAMESPACE

QDemonModel::QDemonModel()
{

}

QDemonModel::~QDemonModel()
{

}

QDemonObject::Type QDemonModel::type() const
{
    return QDemonObject::Model;
}

QString QDemonModel::source() const
{
    return m_source;
}

int QDemonModel::skeletonRoot() const
{
    return m_skeletonRoot;
}

QDemonModel::QDemonTessModeValues QDemonModel::tesselationMode() const
{
    return m_tesselationMode;
}

float QDemonModel::edgeTess() const
{
    return m_edgeTess;
}

float QDemonModel::innerTess() const
{
    return m_innerTess;
}

bool QDemonModel::isWireframeMode() const
{
    return m_isWireframeMode;
}

QQmlListProperty<QDemonMaterial> QDemonModel::materials()
{
    return QQmlListProperty<QDemonMaterial>(this, nullptr,
                                            QDemonModel::qmlAppendMaterial,
                                            QDemonModel::qmlMaterialsCount,
                                            QDemonModel::qmlMaterialAt,
                                            QDemonModel::qmlClearMaterials);
}

void QDemonModel::setSource(QString source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
    update();
}

void QDemonModel::setSkeletonRoot(int skeletonRoot)
{
    if (m_skeletonRoot == skeletonRoot)
        return;

    m_skeletonRoot = skeletonRoot;
    emit skeletonRootChanged(m_skeletonRoot);
    update();
}

void QDemonModel::setTesselationMode(QDemonModel::QDemonTessModeValues tesselationMode)
{
    if (m_tesselationMode == tesselationMode)
        return;

    m_tesselationMode = tesselationMode;
    emit tesselationModeChanged(m_tesselationMode);
    update();
}

void QDemonModel::setEdgeTess(float edgeTess)
{
    if (qFuzzyCompare(m_edgeTess, edgeTess))
        return;

    m_edgeTess = edgeTess;
    emit edgeTessChanged(m_edgeTess);
    update();
}

void QDemonModel::setInnerTess(float innerTess)
{
    if (qFuzzyCompare(m_innerTess, innerTess))
        return;

    m_innerTess = innerTess;
    emit innerTessChanged(m_innerTess);
    update();
}

void QDemonModel::setIsWireframeMode(bool isWireframeMode)
{
    if (m_isWireframeMode == isWireframeMode)
        return;

    m_isWireframeMode = isWireframeMode;
    emit isWireframeModeChanged(m_isWireframeMode);
    update();
}

QDemonGraphObject *QDemonModel::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonRenderModel();

    QDemonNode::updateSpatialNode(node);

    auto modelNode = static_cast<QDemonRenderModel *>(node);

    modelNode->meshPath = m_source;
    modelNode->skeletonRoot = m_skeletonRoot;
    modelNode->tessellationMode = TessModeValues::Enum(m_tesselationMode);
    modelNode->edgeTess = m_edgeTess;
    modelNode->innerTess = m_innerTess;
    modelNode->wireframeMode = m_isWireframeMode;

    // ### TODO: Make sure materials are setup

    return modelNode;
}

void QDemonModel::qmlAppendMaterial(QQmlListProperty<QDemonMaterial> *list, QDemonMaterial *material)
{
    if (material == nullptr)
        return;
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    self->m_materials.push_back(material);
}

QDemonMaterial *QDemonModel::qmlMaterialAt(QQmlListProperty<QDemonMaterial> *list, int index)
{
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    return self->m_materials.at(index);
}

int QDemonModel::qmlMaterialsCount(QQmlListProperty<QDemonMaterial> *list)
{
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    return self->m_materials.count();
}

void QDemonModel::qmlClearMaterials(QQmlListProperty<QDemonMaterial> *list)
{
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    self->m_materials.clear();
}

QT_END_NAMESPACE
