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

#include "qquick3ddefaultmaterial.h"
#include "qquick3dobject_p.h"

#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype DefualtMaterial
    \inqmlmodule QtQuick3D
    \brief Lets you define material for the 3D item
*/
QQuick3DDefaultMaterial::QQuick3DDefaultMaterial()
    : m_diffuseColor(Qt::white)
    , m_emissiveColor(Qt::white)
    , m_specularTint(Qt::white)
{}

QQuick3DDefaultMaterial::~QQuick3DDefaultMaterial()
{
    for(auto connection : m_connections.values())
        disconnect(connection);
}

static void updateProperyListener(QQuick3DObject *newO, QQuick3DObject *oldO, QQuick3DSceneManager *window, QQuick3DDefaultMaterial::ConnectionMap &connections, std::function<void(QQuick3DObject *o)> callFn) {
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

QQuick3DObject::Type QQuick3DDefaultMaterial::type() const
{
    return QQuick3DObject::DefaultMaterial;
}

QQuick3DDefaultMaterial::QDemonDefaultMaterialLighting QQuick3DDefaultMaterial::lighting() const
{
    return m_lighting;
}

QQuick3DDefaultMaterial::QDemonDefaultMaterialBlendMode QQuick3DDefaultMaterial::blendMode() const
{
    return m_blendMode;
}

QColor QQuick3DDefaultMaterial::diffuseColor() const
{
    return m_diffuseColor;
}

QQuick3DTexture *QQuick3DDefaultMaterial::diffuseMap() const
{
    return m_diffuseMap;
}

QQuick3DTexture *QQuick3DDefaultMaterial::diffuseMap2() const
{
    return m_diffuseMap2;
}

QQuick3DTexture *QQuick3DDefaultMaterial::diffuseMap3() const
{
    return m_diffuseMap3;
}

float QQuick3DDefaultMaterial::emissivePower() const
{
    return m_emissivePower;
}

QQuick3DTexture *QQuick3DDefaultMaterial::emissiveMap() const
{
    return m_emissiveMap;
}

QColor QQuick3DDefaultMaterial::emissiveColor() const
{
    return m_emissiveColor;
}

QQuick3DTexture *QQuick3DDefaultMaterial::specularReflectionMap() const
{
    return m_specularReflectionMap;
}

QQuick3DTexture *QQuick3DDefaultMaterial::specularMap() const
{
    return m_specularMap;
}

QQuick3DDefaultMaterial::QDemonDefaultMaterialSpecularModel QQuick3DDefaultMaterial::specularModel() const
{
    return m_specularModel;
}

QColor QQuick3DDefaultMaterial::specularTint() const
{
    return m_specularTint;
}

float QQuick3DDefaultMaterial::indexOfRefraction() const
{
    return m_indexOfRefraction;
}

float QQuick3DDefaultMaterial::fresnelPower() const
{
    return m_fresnelPower;
}

float QQuick3DDefaultMaterial::specularAmount() const
{
    return m_specularAmount;
}

float QQuick3DDefaultMaterial::specularRoughness() const
{
    return m_specularRoughness;
}

QQuick3DTexture *QQuick3DDefaultMaterial::roughnessMap() const
{
    return m_roughnessMap;
}

float QQuick3DDefaultMaterial::opacity() const
{
    return m_opacity;
}

QQuick3DTexture *QQuick3DDefaultMaterial::opacityMap() const
{
    return m_opacityMap;
}

QQuick3DTexture *QQuick3DDefaultMaterial::bumpMap() const
{
    return m_bumpMap;
}

float QQuick3DDefaultMaterial::bumpAmount() const
{
    return m_bumpAmount;
}

QQuick3DTexture *QQuick3DDefaultMaterial::normalMap() const
{
    return m_normalMap;
}

QQuick3DTexture *QQuick3DDefaultMaterial::translucencyMap() const
{
    return m_translucencyMap;
}

float QQuick3DDefaultMaterial::translucentFalloff() const
{
    return m_translucentFalloff;
}

float QQuick3DDefaultMaterial::diffuseLightWrap() const
{
    return m_diffuseLightWrap;
}

bool QQuick3DDefaultMaterial::vertexColors() const
{
    return m_vertexColors;
}

void QQuick3DDefaultMaterial::setLighting(QQuick3DDefaultMaterial::QDemonDefaultMaterialLighting lighting)
{
    if (m_lighting == lighting)
        return;

    m_lighting = lighting;
    emit lightingChanged(m_lighting);
    markDirty(LightingModeDirty);
}

void QQuick3DDefaultMaterial::setBlendMode(QQuick3DDefaultMaterial::QDemonDefaultMaterialBlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;

    m_blendMode = blendMode;
    emit blendModeChanged(m_blendMode);
    markDirty(BlendModeDirty);
}

void QQuick3DDefaultMaterial::setDiffuseColor(QColor diffuseColor)
{
    if (m_diffuseColor == diffuseColor)
        return;

    m_diffuseColor = diffuseColor;
    emit diffuseColorChanged(m_diffuseColor);
    markDirty(DiffuseDirty);
}

void QQuick3DDefaultMaterial::setDiffuseMap(QQuick3DTexture *diffuseMap)
{
    if (m_diffuseMap == diffuseMap)
        return;

    updateProperyListener(diffuseMap, m_diffuseMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setDiffuseMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_diffuseMap = diffuseMap;
    emit diffuseMapChanged(m_diffuseMap);
    markDirty(DiffuseDirty);
}

void QQuick3DDefaultMaterial::setDiffuseMap2(QQuick3DTexture *diffuseMap2)
{
    if (m_diffuseMap2 == diffuseMap2)
        return;

    updateProperyListener(diffuseMap2, m_diffuseMap2, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setDiffuseMap2(qobject_cast<QQuick3DTexture *>(n));
    });


    m_diffuseMap2 = diffuseMap2;
    emit diffuseMap2Changed(m_diffuseMap2);
    markDirty(DiffuseDirty);
}

void QQuick3DDefaultMaterial::setDiffuseMap3(QQuick3DTexture *diffuseMap3)
{
    if (m_diffuseMap3 == diffuseMap3)
        return;

    updateProperyListener(diffuseMap3, m_diffuseMap3, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setDiffuseMap3(qobject_cast<QQuick3DTexture *>(n));
    });


    m_diffuseMap3 = diffuseMap3;
    emit diffuseMap3Changed(m_diffuseMap3);
    markDirty(DiffuseDirty);
}

void QQuick3DDefaultMaterial::setEmissivePower(float emissivePower)
{
    if (qFuzzyCompare(m_emissivePower, emissivePower))
        return;

    m_emissivePower = emissivePower;
    emit emissivePowerChanged(m_emissivePower);
    markDirty(EmissiveDirty);
}

void QQuick3DDefaultMaterial::setEmissiveMap(QQuick3DTexture *emissiveMap)
{
    if (m_emissiveMap == emissiveMap)
        return;


    updateProperyListener(emissiveMap, m_emissiveMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setEmissiveMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_emissiveMap = emissiveMap;
    emit emissiveMapChanged(m_emissiveMap);
    markDirty(EmissiveDirty);
}

void QQuick3DDefaultMaterial::setEmissiveColor(QColor emissiveColor)
{
    if (m_emissiveColor == emissiveColor)
        return;

    m_emissiveColor = emissiveColor;
    emit emissiveColorChanged(m_emissiveColor);
    markDirty(EmissiveDirty);
}

void QQuick3DDefaultMaterial::setSpecularReflectionMap(QQuick3DTexture *specularReflectionMap)
{
    if (m_specularReflectionMap == specularReflectionMap)
        return;

    updateProperyListener(specularReflectionMap, m_specularReflectionMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setSpecularReflectionMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_specularReflectionMap = specularReflectionMap;
    emit specularReflectionMapChanged(m_specularReflectionMap);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setSpecularMap(QQuick3DTexture *specularMap)
{
    if (m_specularMap == specularMap)
        return;

    updateProperyListener(specularMap, m_specularMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setSpecularMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_specularMap = specularMap;
    emit specularMapChanged(m_specularMap);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setSpecularModel(QQuick3DDefaultMaterial::QDemonDefaultMaterialSpecularModel specularModel)
{
    if (m_specularModel == specularModel)
        return;

    m_specularModel = specularModel;
    emit specularModelChanged(m_specularModel);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setSpecularTint(QColor specularTint)
{
    if (m_specularTint == specularTint)
        return;

    m_specularTint = specularTint;
    emit specularTintChanged(m_specularTint);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setIndexOfRefraction(float indexOfRefraction)
{
    if (qFuzzyCompare(m_indexOfRefraction, indexOfRefraction))
        return;

    m_indexOfRefraction = indexOfRefraction;
    emit indexOfRefractionChanged(m_indexOfRefraction);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setFresnelPower(float fresnelPower)
{
    if (qFuzzyCompare(m_fresnelPower, fresnelPower))
        return;

    m_fresnelPower = fresnelPower;
    emit fresnelPowerChanged(m_fresnelPower);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setSpecularAmount(float specularAmount)
{
    if (qFuzzyCompare(m_specularAmount, specularAmount))
        return;

    m_specularAmount = specularAmount;
    emit specularAmountChanged(m_specularAmount);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setSpecularRoughness(float specularRoughness)
{
    if (qFuzzyCompare(m_specularRoughness, specularRoughness))
        return;

    m_specularRoughness = specularRoughness;
    emit specularRoughnessChanged(m_specularRoughness);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setRoughnessMap(QQuick3DTexture *roughnessMap)
{
    if (m_roughnessMap == roughnessMap)
        return;

    updateProperyListener(roughnessMap, m_roughnessMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setRoughnessMap(qobject_cast<QQuick3DTexture *>(n));
    });


    m_roughnessMap = roughnessMap;
    emit roughnessMapChanged(m_roughnessMap);
    markDirty(SpecularDirty);
}

void QQuick3DDefaultMaterial::setOpacity(float opacity)
{
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    if (opacity > 1.0f)
        opacity = 1.0f;

    if (opacity < 0.0f)
        opacity = 0.0f;

    m_opacity = opacity;
    emit opacityChanged(m_opacity);
    markDirty(OpacityDirty);
}

void QQuick3DDefaultMaterial::setOpacityMap(QQuick3DTexture *opacityMap)
{
    if (m_opacityMap == opacityMap)
        return;

    updateProperyListener(opacityMap, m_opacityMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setOpacityMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_opacityMap = opacityMap;
    emit opacityMapChanged(m_opacityMap);
    markDirty(OpacityDirty);
}

void QQuick3DDefaultMaterial::setBumpMap(QQuick3DTexture *bumpMap)
{
    if (m_bumpMap == bumpMap)
        return;

    updateProperyListener(bumpMap, m_bumpMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setBumpMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_bumpMap = bumpMap;
    emit bumpMapChanged(m_bumpMap);
    markDirty(BumpDirty);
}

void QQuick3DDefaultMaterial::setBumpAmount(float bumpAmount)
{
    if (qFuzzyCompare(m_bumpAmount, bumpAmount))
        return;

    m_bumpAmount = bumpAmount;
    emit bumpAmountChanged(m_bumpAmount);
    markDirty(BumpDirty);
}

void QQuick3DDefaultMaterial::setNormalMap(QQuick3DTexture *normalMap)
{
    if (m_normalMap == normalMap)
        return;

    updateProperyListener(normalMap, m_normalMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setNormalMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_normalMap = normalMap;
    emit normalMapChanged(m_normalMap);
    markDirty(NormalDirty);
}

void QQuick3DDefaultMaterial::setTranslucencyMap(QQuick3DTexture *translucencyMap)
{
    if (m_translucencyMap == translucencyMap)
        return;

    updateProperyListener(translucencyMap, m_translucencyMap, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setTranslucencyMap(qobject_cast<QQuick3DTexture *>(n));
    });

    m_translucencyMap = translucencyMap;
    emit translucencyMapChanged(m_translucencyMap);
    markDirty(TranslucencyDirty);
}

void QQuick3DDefaultMaterial::setTranslucentFalloff(float translucentFalloff)
{
    if (qFuzzyCompare(m_translucentFalloff, translucentFalloff))
        return;

    m_translucentFalloff = translucentFalloff;
    emit translucentFalloffChanged(m_translucentFalloff);
    markDirty(TranslucencyDirty);
}

void QQuick3DDefaultMaterial::setDiffuseLightWrap(float diffuseLightWrap)
{
    if (qFuzzyCompare(m_diffuseLightWrap, diffuseLightWrap))
        return;

    m_diffuseLightWrap = diffuseLightWrap;
    emit diffuseLightWrapChanged(m_diffuseLightWrap);
    markDirty(DiffuseDirty);
}

void QQuick3DDefaultMaterial::setVertexColors(bool vertexColors)
{
    if (m_vertexColors == vertexColors)
        return;

    m_vertexColors = vertexColors;
    emit vertexColorsChanged(m_vertexColors);
    markDirty(VertexColorsDirty);
}

QDemonRenderGraphObject *QQuick3DDefaultMaterial::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderDefaultMaterial();

    // Set common material properties
    QQuick3DMaterial::updateSpatialNode(node);

    QDemonRenderDefaultMaterial *material = static_cast<QDemonRenderDefaultMaterial *>(node);

    if (m_dirtyAttributes & LightingModeDirty)
        material->lighting = QDemonRenderDefaultMaterial::MaterialLighting(m_lighting);

    if (m_dirtyAttributes & BlendModeDirty)
        material->blendMode = QDemonRenderDefaultMaterial::MaterialBlendMode(m_blendMode);

    if (m_dirtyAttributes & DiffuseDirty) {
        material->diffuseColor = QVector3D(m_diffuseColor.redF(), m_diffuseColor.greenF(), m_diffuseColor.blueF());
        if (!m_diffuseMap)
            material->diffuseMaps[0] = nullptr;
        else
            material->diffuseMaps[0] = m_diffuseMap->getRenderImage();

        if (!m_diffuseMap2)
            material->diffuseMaps[1] = nullptr;
        else
            material->diffuseMaps[1] = m_diffuseMap2->getRenderImage();

        if (!m_diffuseMap3)
            material->diffuseMaps[2] = nullptr;
        else
            material->diffuseMaps[2] = m_diffuseMap3->getRenderImage();

        material->diffuseLightWrap = m_diffuseLightWrap;
    }

    if (m_dirtyAttributes & EmissiveDirty) {
        material->emissivePower = m_emissivePower;
        if (!m_emissiveMap)
            material->emissiveMap = nullptr;
        else
            material->emissiveMap = m_emissiveMap->getRenderImage();
        material->emissiveColor = QVector3D(m_emissiveColor.redF(), m_emissiveColor.greenF(), m_emissiveColor.blueF());
    }

    if (m_dirtyAttributes & SpecularDirty) {
        if (!m_specularReflectionMap)
            material->specularReflection = nullptr;
        else
            material->specularReflection = m_specularReflectionMap->getRenderImage();

        if (!m_specularMap)
            material->specularMap = nullptr;
        else
            material->specularMap = m_specularMap->getRenderImage();

        material->specularModel = QDemonRenderDefaultMaterial::MaterialSpecularModel(m_specularModel);
        material->specularTint = QVector3D(m_specularTint.redF(), m_specularTint.greenF(), m_specularTint.blueF());
        material->ior = m_indexOfRefraction;
        material->fresnelPower = m_fresnelPower;
        material->specularAmount = m_specularAmount;
        material->specularRoughness = m_specularRoughness;

        if (!m_roughnessMap)
            material->roughnessMap = nullptr;
        else
            material->roughnessMap = m_roughnessMap->getRenderImage();
    }

    if (m_dirtyAttributes & OpacityDirty) {
        material->opacity = m_opacity;
        if (!m_opacityMap)
            material->opacityMap = nullptr;
        else
            material->opacityMap = m_opacityMap->getRenderImage();
    }

    if (m_dirtyAttributes & BumpDirty) {
        if (!m_bumpMap)
            material->bumpMap = nullptr;
        else
            material->bumpMap = m_bumpMap->getRenderImage();
        material->bumpAmount = m_bumpAmount;
    }

    if (m_dirtyAttributes & NormalDirty) {
        if (!m_normalMap)
            material->normalMap = nullptr;
        else
            material->normalMap = m_normalMap->getRenderImage();
    }

    if (m_dirtyAttributes & TranslucencyDirty) {
        if (!m_translucencyMap)
            material->translucencyMap = nullptr;
        else
            material->translucencyMap = m_translucencyMap->getRenderImage();
        material->translucentFalloff = m_translucentFalloff;
    }

    if (m_dirtyAttributes & VertexColorsDirty)
        material->vertexColors = m_vertexColors;

    m_dirtyAttributes = 0;

    return node;
}

void QQuick3DDefaultMaterial::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneRenderer(value.sceneRenderer);
}

void QQuick3DDefaultMaterial::updateSceneRenderer(QQuick3DSceneManager *window)
{
    // Check all the resource value's windows, and update as necessary
    if (window) {
        if (m_diffuseMap)
            QQuick3DObjectPrivate::get(m_diffuseMap)->refSceneRenderer(window);
        if (m_diffuseMap2)
            QQuick3DObjectPrivate::get(m_diffuseMap2)->refSceneRenderer(window);
        if (m_diffuseMap3)
            QQuick3DObjectPrivate::get(m_diffuseMap3)->refSceneRenderer(window);
        if (m_emissiveMap)
            QQuick3DObjectPrivate::get(m_emissiveMap)->refSceneRenderer(window);
        if (m_specularReflectionMap)
            QQuick3DObjectPrivate::get(m_specularReflectionMap)->refSceneRenderer(window);
        if (m_specularMap)
            QQuick3DObjectPrivate::get(m_specularMap)->refSceneRenderer(window);
        if (m_roughnessMap)
            QQuick3DObjectPrivate::get(m_roughnessMap)->refSceneRenderer(window);
        if (m_opacityMap)
            QQuick3DObjectPrivate::get(m_opacityMap)->refSceneRenderer(window);
        if (m_bumpMap)
            QQuick3DObjectPrivate::get(m_bumpMap)->refSceneRenderer(window);
        if (m_normalMap)
            QQuick3DObjectPrivate::get(m_normalMap)->refSceneRenderer(window);
        if (m_translucencyMap)
            QQuick3DObjectPrivate::get(m_translucencyMap)->refSceneRenderer(window);
    } else {
        if (m_diffuseMap)
            QQuick3DObjectPrivate::get(m_diffuseMap)->derefSceneRenderer();
        if (m_diffuseMap2)
            QQuick3DObjectPrivate::get(m_diffuseMap2)->derefSceneRenderer();
        if (m_diffuseMap3)
            QQuick3DObjectPrivate::get(m_diffuseMap3)->derefSceneRenderer();
        if (m_emissiveMap)
            QQuick3DObjectPrivate::get(m_emissiveMap)->derefSceneRenderer();
        if (m_specularReflectionMap)
            QQuick3DObjectPrivate::get(m_specularReflectionMap)->derefSceneRenderer();
        if (m_specularMap)
            QQuick3DObjectPrivate::get(m_specularMap)->derefSceneRenderer();
        if (m_roughnessMap)
            QQuick3DObjectPrivate::get(m_roughnessMap)->derefSceneRenderer();
        if (m_opacityMap)
            QQuick3DObjectPrivate::get(m_opacityMap)->derefSceneRenderer();
        if (m_bumpMap)
            QQuick3DObjectPrivate::get(m_bumpMap)->derefSceneRenderer();
        if (m_normalMap)
            QQuick3DObjectPrivate::get(m_normalMap)->derefSceneRenderer();
        if (m_translucencyMap)
            QQuick3DObjectPrivate::get(m_translucencyMap)->derefSceneRenderer();
    }
}

void QQuick3DDefaultMaterial::markDirty(QQuick3DDefaultMaterial::QDemonDefaultMaterialDirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

QT_END_NAMESPACE
