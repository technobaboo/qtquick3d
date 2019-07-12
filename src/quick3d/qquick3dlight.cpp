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

#include "qquick3dlight.h"
#include "qquick3dobject_p.h"

#include <QtDemonRuntimeRender/qdemonrenderlight.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Light
    \inqmlmodule QtQuick3D
    \brief Lets you define the lighting for a 3D item.
*/

/*!
 * \qmlproperty enumeration Light::lightType
 *
 *
 */

/*!
 * \qmlproperty color Light::diffuseColor
 *
 *
 */

/*!
 * \qmlproperty color Light::specularColor
 *
 *
 */

/*!
 * \qmlproperty color Light::ambientColor
 *
 *
 */

/*!
 * \qmlproperty real Light::brightness
 *
 *
 */

/*!
 * \qmlproperty real Light::linearFade
 *
 *
 */

/*!
 * \qmlproperty real Light::exponentialFade
 *
 *
 */

/*!
 * \qmlproperty real Light::areaWidth
 *
 *
 */

/*!
 * \qmlproperty real Light::areaHeight
 *
 *
 */

/*!
 * \qmlproperty bool Light::castShadow
 *
 *
 */

/*!
 * \qmlproperty real Light::shadowBias
 *
 *
 */

/*!
 * \qmlproperty real Light::shadowFactor
 *
 *
 */

/*!
 * \qmlproperty int Light::shadowMapResolution
 *
 *
 */

/*!
 * \qmlproperty real Light::shadowMapFar
 *
 *
 */

/*!
 * \qmlproperty real Light::shadowMapFieldOFView
 *
 *
 */

/*!
 * \qmlproperty real Light::shadowFilter
 *
 *
 */

/*!
 * \qmlproperty Node Light::scope
 *
 *
 */

QQuick3DLight::QQuick3DLight() : m_diffuseColor(Qt::white), m_specularColor(Qt::white), m_ambientColor(Qt::black) {}

QQuick3DLight::~QQuick3DLight() {}

QQuick3DObject::Type QQuick3DLight::type() const
{
    return QQuick3DObject::Light;
}

QQuick3DLight::QDemonRenderLightTypes QQuick3DLight::lightType() const
{
    return m_lightType;
}

QColor QQuick3DLight::diffuseColor() const
{
    return m_diffuseColor;
}

QColor QQuick3DLight::specularColor() const
{
    return m_specularColor;
}

QColor QQuick3DLight::ambientColor() const
{
    return m_ambientColor;
}

float QQuick3DLight::brightness() const
{
    return m_brightness;
}

float QQuick3DLight::linearFade() const
{
    return m_linearFade;
}

float QQuick3DLight::exponentialFade() const
{
    return m_exponentialFade;
}

float QQuick3DLight::areaWidth() const
{
    return m_areaWidth;
}

float QQuick3DLight::areaHeight() const
{
    return m_areaHeight;
}

bool QQuick3DLight::castShadow() const
{
    return m_castShadow;
}

float QQuick3DLight::shadowBias() const
{
    return m_shadowBias;
}

float QQuick3DLight::shadowFactor() const
{
    return m_shadowFactor;
}

int QQuick3DLight::shadowMapResolution() const
{
    return m_shadowMapResolution;
}

float QQuick3DLight::shadowMapFar() const
{
    return m_shadowMapFar;
}

float QQuick3DLight::shadowMapFieldOfView() const
{
    return m_shadowMapFieldOfView;
}

float QQuick3DLight::shadowFilter() const
{
    return m_shadowFilter;
}

QQuick3DNode *QQuick3DLight::scope() const
{
    return m_scope;
}

void QQuick3DLight::setLightType(QQuick3DLight::QDemonRenderLightTypes lightType)
{
    if (m_lightType == lightType)
        return;

    m_lightType = lightType;
    emit lightTypeChanged(m_lightType);
    update();
}

void QQuick3DLight::setDiffuseColor(QColor diffuseColor)
{
    if (m_diffuseColor == diffuseColor)
        return;

    m_diffuseColor = diffuseColor;
    m_dirtyFlags.setFlag(DirtyFlag::ColorDirty);
    emit diffuseColorChanged(m_diffuseColor);
    update();
}

void QQuick3DLight::setSpecularColor(QColor specularColor)
{
    if (m_specularColor == specularColor)
        return;

    m_specularColor = specularColor;
    m_dirtyFlags.setFlag(DirtyFlag::ColorDirty);
    emit specularColorChanged(m_specularColor);
    update();
}

void QQuick3DLight::setAmbientColor(QColor ambientColor)
{
    if (m_ambientColor == ambientColor)
        return;

    m_ambientColor = ambientColor;
    m_dirtyFlags.setFlag(DirtyFlag::ColorDirty);
    emit ambientColorChanged(m_ambientColor);
    update();
}

void QQuick3DLight::setBrightness(float brightness)
{
    if (qFuzzyCompare(m_brightness, brightness))
        return;

    m_brightness = brightness;
    m_dirtyFlags.setFlag(DirtyFlag::BrightnessDirty);
    emit brightnessChanged(m_brightness);
    update();
}

void QQuick3DLight::setLinearFade(float linearFade)
{
    if (qFuzzyCompare(m_linearFade, linearFade))
        return;

    m_linearFade = linearFade;
    m_dirtyFlags.setFlag(DirtyFlag::BrightnessDirty);
    emit linearFadeChanged(m_linearFade);
    update();
}

void QQuick3DLight::setExponentialFade(float exponentialFade)
{
    if (qFuzzyCompare(m_exponentialFade, exponentialFade))
        return;

    m_exponentialFade = exponentialFade;
    m_dirtyFlags.setFlag(DirtyFlag::BrightnessDirty);
    emit exponentialFadeChanged(m_exponentialFade);
    update();
}

void QQuick3DLight::setAreaWidth(float areaWidth)
{
    if (qFuzzyCompare(m_areaWidth, areaWidth))
        return;

    m_areaWidth = areaWidth;
    m_dirtyFlags.setFlag(DirtyFlag::AreaDirty);
    emit areaWidthChanged(m_areaWidth);
    update();
}

void QQuick3DLight::setAreaHeight(float areaHeight)
{
    if (qFuzzyCompare(m_areaHeight, areaHeight))
        return;

    m_areaHeight = areaHeight;
    m_dirtyFlags.setFlag(DirtyFlag::AreaDirty);
    emit areaHeightChanged(m_areaHeight);
    update();
}

void QQuick3DLight::setCastShadow(bool castShadow)
{
    if (m_castShadow == castShadow)
        return;

    m_castShadow = castShadow;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit castShadowChanged(m_castShadow);
    update();
}

void QQuick3DLight::setShadowBias(float shadowBias)
{
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowBiasChanged(m_shadowBias);
    update();
}

void QQuick3DLight::setShadowFactor(float shadowFactor)
{
    if (qFuzzyCompare(m_shadowFactor, shadowFactor))
        return;

    m_shadowFactor = shadowFactor;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowFactorChanged(m_shadowFactor);
    update();
}

void QQuick3DLight::setShadowMapResolution(int shadowMapResolution)
{
    if (m_shadowMapResolution == shadowMapResolution)
        return;

    m_shadowMapResolution = shadowMapResolution;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowMapResolutionChanged(m_shadowMapResolution);
    update();
}

void QQuick3DLight::setShadowMapFar(float shadowMapFar)
{
    if (qFuzzyCompare(m_shadowMapFar, shadowMapFar))
        return;

    m_shadowMapFar = shadowMapFar;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowMapFarChanged(m_shadowMapFar);
    update();
}

void QQuick3DLight::setShadowMapFieldOfView(float shadowMapFieldOfView)
{
    if (qFuzzyCompare(m_shadowMapFieldOfView, shadowMapFieldOfView))
        return;

    m_shadowMapFieldOfView = shadowMapFieldOfView;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowMapFieldOfViewChanged(m_shadowMapFieldOfView);
    update();
}

void QQuick3DLight::setShadowFilter(float shadowFilter)
{
    if (qFuzzyCompare(m_shadowFilter, shadowFilter))
        return;

    m_shadowFilter = shadowFilter;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowFilterChanged(m_shadowFilter);
    update();
}

void QQuick3DLight::setScope(QQuick3DNode *scope)
{
    if (m_scope == scope)
        return;

    m_scope = scope;
    emit scopeChanged(m_scope);
    update();
}

QDemonRenderGraphObject *QQuick3DLight::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderLight();

    QQuick3DNode::updateSpatialNode(node);

    QDemonRenderLight *light = static_cast<QDemonRenderLight *>(node);

    light->m_lightType = QDemonRenderLight::Type(m_lightType);
    if (m_dirtyFlags.testFlag(DirtyFlag::ColorDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::ColorDirty, false);
        light->m_diffuseColor = QVector3D(m_diffuseColor.redF(), m_diffuseColor.greenF(), m_diffuseColor.blueF());
        light->m_specularColor = QVector3D(m_specularColor.redF(), m_specularColor.greenF(), m_specularColor.blueF());
        light->m_ambientColor = QVector3D(m_ambientColor.redF(), m_ambientColor.greenF(), m_ambientColor.blueF());
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::BrightnessDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::BrightnessDirty, false);
        light->m_brightness = m_brightness;
        light->m_linearFade = m_linearFade;
        light->m_exponentialFade = m_exponentialFade;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::AreaDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::AreaDirty, false);
        light->m_areaWidth = m_areaWidth;
        light->m_areaHeight = m_areaHeight;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::ShadowDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty, false);
        light->m_castShadow = m_castShadow;
        light->m_shadowBias = m_shadowBias;
        light->m_shadowFactor = m_shadowFactor;
        light->m_shadowMapRes = m_shadowMapResolution;
        light->m_shadowMapFar = m_shadowMapFar;
        light->m_shadowMapFov = m_shadowMapFieldOfView;
        light->m_shadowFilter = m_shadowFilter;
    }

    if (m_scope)
        light->m_scope = static_cast<QDemonRenderNode*>(QQuick3DObjectPrivate::get(m_scope)->spatialNode);
    else
        light->m_scope = nullptr;

    return node;
}

QT_END_NAMESPACE
