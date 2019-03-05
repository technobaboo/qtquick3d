#include "qdemoncustommaterial.h"
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>

QT_BEGIN_NAMESPACE

QDemonCustomMaterial::QDemonCustomMaterial() {}

QDemonCustomMaterial::~QDemonCustomMaterial() {}

QDemonObject::Type QDemonCustomMaterial::type() const
{
    return QDemonObject::CustomMaterial;
}

bool QDemonCustomMaterial::hasTransparency() const
{
    return m_hasTransparency;
}

bool QDemonCustomMaterial::hasRefraction() const
{
    return m_hasRefraction;
}

bool QDemonCustomMaterial::hasVolumetricDF() const
{
    return m_hasVolumetricDF;
}

void QDemonCustomMaterial::setHasTransparency(bool hasTransparency)
{
    if (m_hasTransparency == hasTransparency)
        return;

    m_hasTransparency = hasTransparency;
    emit hasTransparencyChanged(m_hasTransparency);
}

void QDemonCustomMaterial::setHasRefraction(bool hasRefraction)
{
    if (m_hasRefraction == hasRefraction)
        return;

    m_hasRefraction = hasRefraction;
    emit hasRefractionChanged(m_hasRefraction);
}

void QDemonCustomMaterial::setHasVolumetricDF(bool hasVolumetricDF)
{
    if (m_hasVolumetricDF == hasVolumetricDF)
        return;

    m_hasVolumetricDF = hasVolumetricDF;
    emit hasVolumetricDFChanged(m_hasVolumetricDF);
}

QDemonGraphObject *QDemonCustomMaterial::updateSpatialNode(QDemonGraphObject *node)
{
    // TODO: Create and update customer material (special)

    return node;
}

QT_END_NAMESPACE
