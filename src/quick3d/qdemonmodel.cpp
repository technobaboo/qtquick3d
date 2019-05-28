#include "qdemonmodel.h"
#include "qdemonobject_p.h"

#include <QtDemonRuntimeRender/QDemonRenderGraphObject>
#include <QtDemonRuntimeRender/QDemonRenderCustomMaterial>
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtQml/QQmlFile>

QT_BEGIN_NAMESPACE

QDemonModel::QDemonModel() {}

QDemonModel::~QDemonModel() {}

QDemonObject::Type QDemonModel::type() const
{
    return QDemonObject::Model;
}

QUrl QDemonModel::source() const
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
    return QQmlListProperty<QDemonMaterial>(this,
                                            nullptr,
                                            QDemonModel::qmlAppendMaterial,
                                            QDemonModel::qmlMaterialsCount,
                                            QDemonModel::qmlMaterialAt,
                                            QDemonModel::qmlClearMaterials);
}

void QDemonModel::setSource(const QUrl &source)
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

static QDemonRenderGraphObject *getMaterialNodeFromQDemonMaterial(QDemonMaterial *material)
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(material);
    return p->spatialNode;
}

QDemonRenderGraphObject *QDemonModel::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderModel();

    QDemonNode::updateSpatialNode(node);

    auto modelNode = static_cast<QDemonRenderModel *>(node);
    // TODO: Don't call translateSource() unless the source is dirty!
    modelNode->meshPath = translateSource();
    modelNode->skeletonRoot = m_skeletonRoot;
    modelNode->tessellationMode = TessModeValues(m_tesselationMode);
    modelNode->edgeTess = m_edgeTess;
    modelNode->innerTess = m_innerTess;
    modelNode->wireframeMode = m_isWireframeMode;

    // ### TODO: Make sure materials are setup
    if (!m_materials.isEmpty()) {
        if (modelNode->firstMaterial == nullptr) {
            // Easy mode, just add each material
            for (auto material : m_materials) {
                QDemonRenderGraphObject *graphObject = getMaterialNodeFromQDemonMaterial(material);
                if (graphObject)
                    modelNode->addMaterial(*graphObject);
            }
        } else {
            // Hard mode, go through each material and see if they match
            QDemonRenderGraphObject *material = modelNode->firstMaterial;
            QDemonRenderGraphObject *previousMaterial = nullptr;
            int index = 0;
            while (material) {
                if (index > m_materials.count()) {
                    // Materials have been removed!!
                    previousMaterial->setNextMaterialSibling(nullptr);
                    break;
                }

                auto newMaterial = getMaterialNodeFromQDemonMaterial(m_materials.at(index));

                if (material != newMaterial) {
                    // materials are not the same
                    if (index == 0) {
                        // new first
                        modelNode->firstMaterial = newMaterial;
                    } else {
                        previousMaterial->setNextMaterialSibling(newMaterial);
                    }
                    previousMaterial = newMaterial;
                } else {
                    previousMaterial = material;
                }

                material = material->nextMaterialSibling();
                index++;
            }
        }
    } else {
        // No materials
        modelNode->firstMaterial = nullptr;
    }

    return modelNode;
}

// Source URL's need a bit of translation for the engine because of the
// use of fragment syntax for specifiying primitives and sub-meshes
// So we need to check for the fragment before translating to a qmlfile

QString QDemonModel::translateSource()
{
    QString fragment;
    if (m_source.hasFragment()) {
        // Check if this is an index, or primitive
        bool isNumber = false;
        m_source.fragment().toInt(&isNumber);
        fragment = QStringLiteral("#") + m_source.fragment();
        // If it wasn't an index, then it was a primitive
        if (!isNumber)
            return fragment;
    }

    return QQmlFile::urlToLocalFileOrQrc(m_source) + fragment;
}

void QDemonModel::qmlAppendMaterial(QQmlListProperty<QDemonMaterial> *list, QDemonMaterial *material)
{
    if (material == nullptr)
        return;
    QDemonModel *self = static_cast<QDemonModel *>(list->object);
    self->m_materials.push_back(material);
    self->update();

    if(material->parentItem() == nullptr)
        material->setParentItem(self);
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
    self->update();
}

QT_END_NAMESPACE
