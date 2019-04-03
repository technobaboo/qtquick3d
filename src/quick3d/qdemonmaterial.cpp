#include "qdemonmaterial.h"
#include "qdemonobject_p.h"
#include "qdemonscenemanager_p.h"
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>

QT_BEGIN_NAMESPACE

QDemonMaterial::QDemonMaterial() {}

QDemonMaterial::~QDemonMaterial()
{
    for (auto connection : m_connections)
        disconnect(connection);
}

static void updateProperyListener(QDemonObject *newO, QDemonObject *oldO, QDemonSceneManager *window, QHash<QObject*, QMetaObject::Connection> &connections, std::function<void(QDemonObject *o)> callFn) {
    // disconnect previous destruction listern
    if (oldO) {
        if (window)
            QDemonObjectPrivate::get(oldO)->derefSceneRenderer();

        auto connection = connections.find(oldO);
        if (connection != connections.end()) {
            QObject::disconnect(connection.value());
            connections.erase(connection);
        }
    }

    // listen for new map's destruction
    if (newO) {
        if (window)
            QDemonObjectPrivate::get(newO)->refSceneRenderer(window);
        auto connection = QObject::connect(newO, &QObject::destroyed, [callFn](){
            callFn(nullptr);
        });
        connections.insert(newO, connection);
    }
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

    updateProperyListener(lightmapIndirect, m_lightmapIndirect, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setLightmapIndirect(qobject_cast<QDemonImage *>(n));
    });

    m_lightmapIndirect = lightmapIndirect;
    emit lightmapIndirectChanged(m_lightmapIndirect);
    update();
}

void QDemonMaterial::setLightmapRadiosity(QDemonImage *lightmapRadiosity)
{
    if (m_lightmapRadiosity == lightmapRadiosity)
        return;

    updateProperyListener(lightmapRadiosity, m_lightmapRadiosity, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setLightmapRadiosity(qobject_cast<QDemonImage *>(n));
    });

    m_lightmapRadiosity = lightmapRadiosity;
    emit lightmapRadiosityChanged(m_lightmapRadiosity);
    update();
}

void QDemonMaterial::setLightmapShadow(QDemonImage *lightmapShadow)
{
    if (m_lightmapShadow == lightmapShadow)
        return;

    updateProperyListener(lightmapShadow, m_lightmapShadow, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setLightmapShadow(qobject_cast<QDemonImage *>(n));
    });

    m_lightmapShadow = lightmapShadow;
    emit lightmapShadowChanged(m_lightmapShadow);
    update();
}

void QDemonMaterial::setIblProbe(QDemonImage *iblProbe)
{
    if (m_iblProbe == iblProbe)
        return;

    updateProperyListener(iblProbe, m_iblProbe, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setIblProbe(qobject_cast<QDemonImage *>(n));
    });

    m_iblProbe = iblProbe;
    emit iblProbeChanged(m_iblProbe);
    update();
}

void QDemonMaterial::setEmissiveMap2(QDemonImage *emissiveMap2)
{
    if (m_emissiveMap2 == emissiveMap2)
        return;

    updateProperyListener(emissiveMap2, m_emissiveMap2, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setEmissiveMap2(qobject_cast<QDemonImage *>(n));
    });

    m_emissiveMap2 = emissiveMap2;
    emit emissiveMap2Changed(m_emissiveMap2);
    update();
}

void QDemonMaterial::setDisplacementMap(QDemonImage *displacementMap)
{
    if (m_displacementMap == displacementMap)
        return;

    updateProperyListener(displacementMap, m_displacementMap, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setDisplacementMap(qobject_cast<QDemonImage *>(n));
    });

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

QDemonRenderGraphObject *QDemonMaterial::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        return nullptr;

    // Set the common properties
    if (node->type == QDemonRenderGraphObject::Type::DefaultMaterial) {
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

    } else if (node->type == QDemonRenderGraphObject::Type::CustomMaterial) {
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

void QDemonMaterial::itemChange(QDemonObject::ItemChange change, const QDemonObject::ItemChangeData &value)
{
    if (change == QDemonObject::ItemSceneChange)
        updateSceneRenderer(value.sceneRenderer);
}

void QDemonMaterial::setDynamicTextureMap(QDemonImage *textureMap)
{
    if (!textureMap)
        return;

    auto it = m_dynamicTextureMaps.begin();
    const auto end = m_dynamicTextureMaps.end();
    for (; it != end; ++it) {
        if (*it == textureMap)
            break;
    }

    if (it != end)
        return;

    updateProperyListener(textureMap, nullptr, sceneRenderer(), m_connections, [this](QDemonObject *n) {
        setDynamicTextureMap(qobject_cast<QDemonImage *>(n));
    });

    m_dynamicTextureMaps.push_back(textureMap);
    update();
}

void QDemonMaterial::updateSceneRenderer(QDemonSceneManager *window)
{
    if (window) {
        if (m_lightmapIndirect) {
           QDemonObjectPrivate::get(m_lightmapIndirect)->refSceneRenderer(window);
        }
        if (m_lightmapRadiosity) {
           QDemonObjectPrivate::get(m_lightmapRadiosity)->refSceneRenderer(window);
        }
        if (m_lightmapShadow) {
           QDemonObjectPrivate::get(m_lightmapShadow)->refSceneRenderer(window);
        }
        if (m_iblProbe) {
           QDemonObjectPrivate::get(m_iblProbe)->refSceneRenderer(window);
        }
        if (m_emissiveMap2) {
           QDemonObjectPrivate::get(m_emissiveMap2)->refSceneRenderer(window);
        }
        if (m_displacementMap) {
           QDemonObjectPrivate::get(m_displacementMap)->refSceneRenderer(window);
        }
        for (auto it : m_dynamicTextureMaps)
            QDemonObjectPrivate::get(it)->refSceneRenderer(window);
    } else {
        if (m_lightmapIndirect) {
           QDemonObjectPrivate::get(m_lightmapIndirect)->derefSceneRenderer();
        }
        if (m_lightmapRadiosity) {
           QDemonObjectPrivate::get(m_lightmapRadiosity)->derefSceneRenderer();
        }
        if (m_lightmapShadow) {
           QDemonObjectPrivate::get(m_lightmapShadow)->derefSceneRenderer();
        }
        if (m_iblProbe) {
           QDemonObjectPrivate::get(m_iblProbe)->derefSceneRenderer();
        }
        if (m_emissiveMap2) {
           QDemonObjectPrivate::get(m_emissiveMap2)->derefSceneRenderer();
        }
        if (m_displacementMap) {
           QDemonObjectPrivate::get(m_displacementMap)->derefSceneRenderer();
        }
        for (auto it : m_dynamicTextureMaps)
            QDemonObjectPrivate::get(it)->derefSceneRenderer();
    }
}

QT_END_NAMESPACE
