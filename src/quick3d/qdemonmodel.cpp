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

QDemonModel::TessModeValues QDemonModel::tesselationMode() const
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

void QDemonModel::setSource(QString source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
}

void QDemonModel::setSkeletonRoot(int skeletonRoot)
{
    if (m_skeletonRoot == skeletonRoot)
        return;

    m_skeletonRoot = skeletonRoot;
    emit skeletonRootChanged(m_skeletonRoot);
}

void QDemonModel::setTesselationMode(QDemonModel::TessModeValues tesselationMode)
{
    if (m_tesselationMode == tesselationMode)
        return;

    m_tesselationMode = tesselationMode;
    emit tesselationModeChanged(m_tesselationMode);
}

void QDemonModel::setEdgeTess(float edgeTess)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_edgeTess, edgeTess))
        return;

    m_edgeTess = edgeTess;
    emit edgeTessChanged(m_edgeTess);
}

void QDemonModel::setInnerTess(float innerTess)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_innerTess, innerTess))
        return;

    m_innerTess = innerTess;
    emit innerTessChanged(m_innerTess);
}

void QDemonModel::setIsWireframeMode(bool isWireframeMode)
{
    if (m_isWireframeMode == isWireframeMode)
        return;

    m_isWireframeMode = isWireframeMode;
    emit isWireframeModeChanged(m_isWireframeMode);
}

QDemonGraphObject *QDemonModel::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new SModel();

    auto modelNode = static_cast<SModel *>(node);

    // TODO: Update model properties here

    return modelNode;
}

QT_END_NAMESPACE
