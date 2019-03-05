#include "qdemonmaterial.h"
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>

QT_BEGIN_NAMESPACE

QDemonMaterial::QDemonMaterial() {}

QDemonMaterial::~QDemonMaterial() {}

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
    update();
}

void QDemonMaterial::setLightmapRadiosity(QDemonImage *lightmapRadiosity)
{
    if (m_lightmapRadiosity == lightmapRadiosity)
        return;

    m_lightmapRadiosity = lightmapRadiosity;
    emit lightmapRadiosityChanged(m_lightmapRadiosity);
    update();
}

void QDemonMaterial::setLightmapShadow(QDemonImage *lightmapShadow)
{
    if (m_lightmapShadow == lightmapShadow)
        return;

    m_lightmapShadow = lightmapShadow;
    emit lightmapShadowChanged(m_lightmapShadow);
    update();
}

void QDemonMaterial::setIblProbe(QDemonImage *iblProbe)
{
    if (m_iblProbe == iblProbe)
        return;

    m_iblProbe = iblProbe;
    emit iblProbeChanged(m_iblProbe);
    update();
}

void QDemonMaterial::setEmissiveMap2(QDemonImage *emissiveMap2)
{
    if (m_emissiveMap2 == emissiveMap2)
        return;

    m_emissiveMap2 = emissiveMap2;
    emit emissiveMap2Changed(m_emissiveMap2);
    update();
}

void QDemonMaterial::setDisplacementMap(QDemonImage *displacementMap)
{
    if (m_displacementMap == displacementMap)
        return;

    m_displacementMap = displacementMap;
    emit displacementMapChanged(m_displacementMap);
    update();
}

void QDemonMaterial::setDisplacementAmount(float displacementAmount)
{
    if (qFuzzyCompare(m_displacementAmount, displacementAmount))
        return;

    m_displacementAmount = displacementAmount;
    emit displacementAmountChanged(m_displacementAmount);
    update();
}

QDemonGraphObject *QDemonMaterial::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        return nullptr;

    // Set the common properties
    if (node->type == QDemonGraphObjectTypes::Enum::DefaultMaterial) {
        auto defaultMaterial = static_cast<QDemonRenderDefaultMaterial *>(node);
        if (!m_lightmapIndirect)
            defaultMaterial->lightmaps.m_lightmapIndirect = nullptr;
        else
            defaultMaterial->lightmaps.m_lightmapIndirect = m_lightmapIndirect->getRenderImage();

        if (!m_lightmapRadiosity)
            defaultMaterial->lightmaps.m_lightmapRadiosity = nullptr;
        else
            defaultMaterial->lightmaps.m_lightmapRadiosity = m_lightmapRadiosity->getRenderImage();

        if (!m_lightmapShadow)
            defaultMaterial->lightmaps.m_lightmapShadow = nullptr;
        else
            defaultMaterial->lightmaps.m_lightmapShadow = m_lightmapShadow->getRenderImage();

        if (!m_iblProbe)
            defaultMaterial->iblProbe = nullptr;
        else
            defaultMaterial->iblProbe = m_iblProbe->getRenderImage();

        if (!m_emissiveMap2)
            defaultMaterial->emissiveMap2 = nullptr;
        else
            defaultMaterial->emissiveMap2 = m_emissiveMap2->getRenderImage();

        if (!m_displacementMap)
            defaultMaterial->displacementMap = nullptr;
        else
            defaultMaterial->displacementMap = m_displacementMap->getRenderImage();

        defaultMaterial->displaceAmount = m_displacementAmount;
        node = defaultMaterial;

    } else if (node->type == QDemonGraphObjectTypes::Enum::CustomMaterial) {
        auto customMaterial = static_cast<QDemonRenderCustomMaterial *>(node);
        if (!m_lightmapIndirect)
            customMaterial->m_lightmaps.m_lightmapIndirect = nullptr;
        else
            customMaterial->m_lightmaps.m_lightmapIndirect = m_lightmapIndirect->getRenderImage();

        if (!m_lightmapRadiosity)
            customMaterial->m_lightmaps.m_lightmapRadiosity = nullptr;
        else
            customMaterial->m_lightmaps.m_lightmapRadiosity = m_lightmapRadiosity->getRenderImage();

        if (!m_lightmapShadow)
            customMaterial->m_lightmaps.m_lightmapShadow = nullptr;
        else
            customMaterial->m_lightmaps.m_lightmapShadow = m_lightmapShadow->getRenderImage();

        if (!m_iblProbe)
            customMaterial->m_iblProbe = nullptr;
        else
            customMaterial->m_iblProbe = m_iblProbe->getRenderImage();

        if (!m_emissiveMap2)
            customMaterial->m_emissiveMap2 = nullptr;
        else
            customMaterial->m_emissiveMap2 = m_emissiveMap2->getRenderImage();

        if (!m_displacementMap)
            customMaterial->m_displacementMap = nullptr;
        else
            customMaterial->m_displacementMap = m_displacementMap->getRenderImage();

        customMaterial->m_displaceAmount = m_displacementAmount;
        node = customMaterial;
    }

    return node;
}

QT_END_NAMESPACE
