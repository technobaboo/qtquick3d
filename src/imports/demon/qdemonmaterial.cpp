#include "qdemonmaterial.h"

QDemonMaterial::QDemonMaterial()
{

}

QDemonMaterial::~QDemonMaterial()
{

}

QDemonImage *QDemonMaterial::lightmapIndirect() const
{
    return m_lightmapIndirect;
}

QDemonImage *QDemonMaterial::lightmapRadiosity() const
{
    return m_lightmapRadiosity;
}

QDemonImage *QDemonMaterial::lightmapShadow() const
{
    return m_lightmapShadow;
}

QDemonImage *QDemonMaterial::iblProbe() const
{
    return m_iblProbe;
}

QDemonImage *QDemonMaterial::emissiveMap2() const
{
    return m_emissiveMap2;
}

QDemonImage *QDemonMaterial::displacementMap() const
{
    return m_displacementMap;
}

float QDemonMaterial::displacementAmount() const
{
    return m_displacementAmount;
}

void QDemonMaterial::setLightmapIndirect(QDemonImage *lightmapIndirect)
{
    if (m_lightmapIndirect == lightmapIndirect)
        return;

    m_lightmapIndirect = lightmapIndirect;
    emit lightmapIndirectChanged(m_lightmapIndirect);
}

void QDemonMaterial::setLightmapRadiosity(QDemonImage *lightmapRadiosity)
{
    if (m_lightmapRadiosity == lightmapRadiosity)
        return;

    m_lightmapRadiosity = lightmapRadiosity;
    emit lightmapRadiosityChanged(m_lightmapRadiosity);
}

void QDemonMaterial::setLightmapShadow(QDemonImage *lightmapShadow)
{
    if (m_lightmapShadow == lightmapShadow)
        return;

    m_lightmapShadow = lightmapShadow;
    emit lightmapShadowChanged(m_lightmapShadow);
}

void QDemonMaterial::setIblProbe(QDemonImage *iblProbe)
{
    if (m_iblProbe == iblProbe)
        return;

    m_iblProbe = iblProbe;
    emit iblProbeChanged(m_iblProbe);
}

void QDemonMaterial::setEmissiveMap2(QDemonImage *emissiveMap2)
{
    if (m_emissiveMap2 == emissiveMap2)
        return;

    m_emissiveMap2 = emissiveMap2;
    emit emissiveMap2Changed(m_emissiveMap2);
}


void QDemonMaterial::setDisplacementMap(QDemonImage *displacementMap)
{
    if (m_displacementMap == displacementMap)
        return;

    m_displacementMap = displacementMap;
    emit displacementMapChanged(m_displacementMap);
}

void QDemonMaterial::setDisplacementAmount(float displacementAmount)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_displacementAmount, displacementAmount))
        return;

    m_displacementAmount = displacementAmount;
    emit displacementAmountChanged(m_displacementAmount);
}

SGraphObject *QDemonMaterial::updateSpatialNode(SGraphObject *node)
{
    // TODO update material node properties

    return node;
}

