/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquick3dmaterial_p.h"
#include "qquick3dobject_p_p.h"
#include "qquick3dscenemanager_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Material
    \inqmlmodule QtQuick3D
    \brief Lets you define material for the 3D item.
*/

/*!
 * \qmlproperty Texture Material::lightmapIndirect
 *
 *
 */

/*!
 * \qmlproperty Texture Material::lightmapRadiosity
 *
 *
 */

/*!
 * \qmlproperty Texture Material::lightmapShadow
 *
 *
 */

/*!
 * \qmlproperty Texture Material::iblProbe
 *
 *
 */

/*!
 * \qmlproperty Texture Material::emissiveMap2
 *
 *
 */

/*!
 * \qmlproperty Texture Material::displacementMap
 *
 *
 */

/*!
 * \qmlproperty real Material::displacementAmount
 *
 *
 */


QQuick3DMaterial::QQuick3DMaterial() {}

QQuick3DMaterial::~QQuick3DMaterial()
{
    for (auto connection : m_connections)
        disconnect(connection);
}

static void updateProperyListener(QQuick3DObject *newO, QQuick3DObject *oldO, QQuick3DSceneManager *window, QHash<QObject*, QMetaObject::Connection> &connections, std::function<void(QQuick3DObject *o)> callFn) {
    // disconnect previous destruction listern
    if (oldO) {
        if (window)
            QQuick3DObjectPrivate::get(oldO)->derefSceneRenderer();

        auto connection = connections.find(oldO);
        if (connection != connections.end()) {
            QObject::disconnect(connection.value());
            connections.erase(connection);
        }
    }

    // listen for new map's destruction
    if (newO) {
        if (window)
            QQuick3DObjectPrivate::get(newO)->refSceneRenderer(window);
        auto connection = QObject::connect(newO, &QObject::destroyed, [callFn](){
            callFn(nullptr);
        });
        connections.insert(newO, connection);
    }
}

QQuick3DTexture *QQuick3DMaterial::lightmapIndirect() const
{
    return m_lightmapIndirect;
}

QQuick3DTexture *QQuick3DMaterial::lightmapRadiosity() const
{
    return m_lightmapRadiosity;
}

QQuick3DTexture *QQuick3DMaterial::lightmapShadow() const
{
    return m_lightmapShadow;
}

QQuick3DTexture *QQuick3DMaterial::iblProbe() const
{
    return m_iblProbe;
}

QQuick3DTexture *QQuick3DMaterial::emissiveMap2() const
{
    return m_emissiveMap2;
}

QQuick3DTexture *QQuick3DMaterial::displacementMap() const
{
    return m_displacementMap;
}

float QQuick3DMaterial::displacementAmount() const
{
    return m_displacementAmount;
}

void QQuick3DMaterial::setLightmapIndirect(QQuick3DTexture *lightmapIndirect)
{
    if (m_lightmapIndirect == lightmapIndirect)
        return;

    updateProperyListener(lightmapIndirect, m_lightmapIndirect, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setLightmapIndirect(qobject_cast<QQuick3DTexture *>(n));
    });

    m_lightmapIndirect = lightmapIndirect;
    emit lightmapIndirectChanged(m_lightmapIndirect);
    update();
}

void QQuick3DMaterial::setLightmapRadiosity(QQuick3DTexture *lightmapRadiosity)
{
    if (m_lightmapRadiosity == lightmapRadiosity)
        return;

    updateProperyListener(lightmapRadiosity, m_lightmapRadiosity, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setLightmapRadiosity(qobject_cast<QQuick3DTexture *>(n));
    });

    m_lightmapRadiosity = lightmapRadiosity;
    emit lightmapRadiosityChanged(m_lightmapRadiosity);
    update();
}

void QQuick3DMaterial::setLightmapShadow(QQuick3DTexture *lightmapShadow)
{
    if (m_lightmapShadow == lightmapShadow)
        return;

    updateProperyListener(lightmapShadow, m_lightmapShadow, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setLightmapShadow(qobject_cast<QQuick3DTexture *>(n));
    });

    m_lightmapShadow = lightmapShadow;
    emit lightmapShadowChanged(m_lightmapShadow);
    update();
}

void QQuick3DMaterial::setIblProbe(QQuick3DTexture *iblProbe)
{
    if (m_iblProbe == iblProbe)
        return;

    updateProperyListener(iblProbe, m_iblProbe, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setIblProbe(qobject_cast<QQuick3DTexture *>(n));
    });

    m_iblProbe = iblProbe;
    emit iblProbeChanged(m_iblProbe);
    update();
}

void QQuick3DMaterial::setEmissiveMap2(QQuick3DTexture *emissiveMap2)
{
    if (m_emissiveMap2 == emissiveMap2)
        return;

    updateProperyListener(emissiveMap2, m_emissiveMap2, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setEmissiveMap2(qobject_cast<QQuick3DTexture *>(n));
    });

    m_emissiveMap2 = emissiveMap2;
    emit emissiveMap2Changed(m_emissiveMap2);
    update();
}

void QQuick3DMaterial::setDisplacementMap(QQuick3DTexture *displacementMap)
{
    if (m_displacementMap == displacementMap)
        return;

    updateProperyListener(displacementMap, m_displacementMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setDisplacementMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_displacementMap = displacementMap;
    emit displacementMapChanged(m_displacementMap);
    update();
}

void QQuick3DMaterial::setDisplacementAmount(float displacementAmount)
{
    if (qFuzzyCompare(m_displacementAmount, displacementAmount))
        return;

    m_displacementAmount = displacementAmount;
    emit displacementAmountChanged(m_displacementAmount);
    update();
}

QSSGRenderGraphObject *QQuick3DMaterial::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        return nullptr;

    // Set the common properties
    if (node->type == QSSGRenderGraphObject::Type::DefaultMaterial) {
        auto defaultMaterial = static_cast<QSSGRenderDefaultMaterial *>(node);
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

    } else if (node->type == QSSGRenderGraphObject::Type::CustomMaterial) {
        auto customMaterial = static_cast<QSSGRenderCustomMaterial *>(node);
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

void QQuick3DMaterial::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneRenderer(value.sceneRenderer);
}

void QQuick3DMaterial::setDynamicTextureMap(QQuick3DTexture *textureMap)
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

    updateProperyListener(textureMap, nullptr, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setDynamicTextureMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_dynamicTextureMaps.push_back(textureMap);
    update();
}

void QQuick3DMaterial::updateSceneRenderer(QQuick3DSceneManager *window)
{
    if (window) {
        if (m_lightmapIndirect) {
           QQuick3DObjectPrivate::get(m_lightmapIndirect)->refSceneRenderer(window);
        }
        if (m_lightmapRadiosity) {
           QQuick3DObjectPrivate::get(m_lightmapRadiosity)->refSceneRenderer(window);
        }
        if (m_lightmapShadow) {
           QQuick3DObjectPrivate::get(m_lightmapShadow)->refSceneRenderer(window);
        }
        if (m_iblProbe) {
           QQuick3DObjectPrivate::get(m_iblProbe)->refSceneRenderer(window);
        }
        if (m_emissiveMap2) {
           QQuick3DObjectPrivate::get(m_emissiveMap2)->refSceneRenderer(window);
        }
        if (m_displacementMap) {
           QQuick3DObjectPrivate::get(m_displacementMap)->refSceneRenderer(window);
        }
        for (auto it : m_dynamicTextureMaps)
            QQuick3DObjectPrivate::get(it)->refSceneRenderer(window);
    } else {
        if (m_lightmapIndirect) {
           QQuick3DObjectPrivate::get(m_lightmapIndirect)->derefSceneRenderer();
        }
        if (m_lightmapRadiosity) {
           QQuick3DObjectPrivate::get(m_lightmapRadiosity)->derefSceneRenderer();
        }
        if (m_lightmapShadow) {
           QQuick3DObjectPrivate::get(m_lightmapShadow)->derefSceneRenderer();
        }
        if (m_iblProbe) {
           QQuick3DObjectPrivate::get(m_iblProbe)->derefSceneRenderer();
        }
        if (m_emissiveMap2) {
           QQuick3DObjectPrivate::get(m_emissiveMap2)->derefSceneRenderer();
        }
        if (m_displacementMap) {
           QQuick3DObjectPrivate::get(m_displacementMap)->derefSceneRenderer();
        }
        for (auto it : m_dynamicTextureMaps)
            QQuick3DObjectPrivate::get(it)->derefSceneRenderer();
    }
}

QT_END_NAMESPACE
