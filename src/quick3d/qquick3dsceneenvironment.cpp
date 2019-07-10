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

#include "qquick3dsceneenvironment.h"
#include "qquick3dobject_p.h"
#include "qquick3dtexture.h"

QT_BEGIN_NAMESPACE

static void updateProperyListener(QQuick3DObject *newO, QQuick3DObject *oldO, QQuick3DSceneManager *manager, QHash<QObject*, QMetaObject::Connection> &connections, std::function<void(QQuick3DObject *o)> callFn) {
    // disconnect previous destruction listern
    if (oldO) {
        if (manager)
            QQuick3DObjectPrivate::get(oldO)->derefSceneRenderer();

        auto connection = connections.find(oldO);
        if (connection != connections.end()) {
            QObject::disconnect(connection.value());
            connections.erase(connection);
        }
    }

    // listen for new map's destruction
    if (newO) {
        if (manager)
            QQuick3DObjectPrivate::get(newO)->refSceneRenderer(manager);
        auto connection = QObject::connect(newO, &QObject::destroyed, [callFn](){
            callFn(nullptr);
        });
        connections.insert(newO, connection);
    }
}

/*!
    \qmltype SceneEnvironment
    \inqmlmodule QtQuick3D
    \brief Lets you configure the scene environment.
*/

QQuick3DSceneEnvironment::QQuick3DSceneEnvironment(QQuick3DObject *parent)
    : QQuick3DObject(parent)
{

}

QQuick3DSceneEnvironment::~QQuick3DSceneEnvironment()
{
    for (auto connection : m_connections)
        disconnect(connection);
}

QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues QQuick3DSceneEnvironment::progressiveAAMode() const
{
    return m_progressiveAAMode;
}

QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues QQuick3DSceneEnvironment::multisampleAAMode() const
{
    return m_multisampleAAMode;
}

QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes QQuick3DSceneEnvironment::backgroundMode() const
{
    return m_backgroundMode;
}

QColor QQuick3DSceneEnvironment::clearColor() const
{
    return m_clearColor;
}

QQuick3DSceneEnvironment::QQuick3DEnvironmentBlendTypes QQuick3DSceneEnvironment::blendType() const
{
    return m_blendType;
}

float QQuick3DSceneEnvironment::aoStrength() const
{
    return m_aoStrength;
}

float QQuick3DSceneEnvironment::aoDistance() const
{
    return m_aoDistance;
}

float QQuick3DSceneEnvironment::aoSoftness() const
{
    return m_aoSoftness;
}

bool QQuick3DSceneEnvironment::aoDither() const
{
    return m_aoDither;
}

int QQuick3DSceneEnvironment::aoSampleRate() const
{
    return m_aoSampleRate;
}

float QQuick3DSceneEnvironment::aoBias() const
{
    return m_aoBias;
}

float QQuick3DSceneEnvironment::shadowStrength() const
{
    return m_shadowStrength;
}

float QQuick3DSceneEnvironment::shadowDistance() const
{
    return m_shadowDistance;
}

float QQuick3DSceneEnvironment::shadowSoftness() const
{
    return m_shadowSoftness;
}

float QQuick3DSceneEnvironment::shadowBias() const
{
    return m_shadowBias;
}

QQuick3DTexture *QQuick3DSceneEnvironment::lightProbe() const
{
    return m_lightProbe;
}

float QQuick3DSceneEnvironment::probeBrightness() const
{
    return m_probeBrightness;
}

bool QQuick3DSceneEnvironment::fastIBL() const
{
    return m_fastIBL;
}

float QQuick3DSceneEnvironment::probeHorizon() const
{
    return m_probeHorizon;
}

float QQuick3DSceneEnvironment::probeFieldOfView() const
{
    return m_probeFieldOfView;
}

QQuick3DTexture *QQuick3DSceneEnvironment::lightProbe2() const
{
    return m_lightProbe2;
}

float QQuick3DSceneEnvironment::probe2Fade() const
{
    return m_probe2Fade;
}

float QQuick3DSceneEnvironment::probe2Window() const
{
    return m_probe2Window;
}

float QQuick3DSceneEnvironment::probe2Postion() const
{
    return m_probe2Postion;
}

bool QQuick3DSceneEnvironment::temporalAAEnabled() const
{
    return m_temporalAAEnabled;
}

QQmlListProperty<QQuick3DEffect> QQuick3DSceneEnvironment::effectsList()
{
    return QQmlListProperty<QQuick3DEffect>(this,
                                          nullptr,
                                          QQuick3DSceneEnvironment::qmlAppendEffect,
                                          QQuick3DSceneEnvironment::qmlEffectsCount,
                                          QQuick3DSceneEnvironment::qmlEffectAt,
                                          QQuick3DSceneEnvironment::qmlClearEffects);
}

bool QQuick3DSceneEnvironment::isDepthTestDisabled() const
{
    return m_isDepthTestDisabled;
}

bool QQuick3DSceneEnvironment::isDepthPrePassDisabled() const
{
    return m_isDepthPrePassDisabled;
}

QQuick3DObject::Type QQuick3DSceneEnvironment::type() const
{
    return QQuick3DObject::SceneEnvironment;
}

void QQuick3DSceneEnvironment::setProgressiveAAMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues progressiveAAMode)
{
    if (m_progressiveAAMode == progressiveAAMode)
        return;

    m_progressiveAAMode = progressiveAAMode;
    emit progressiveAAModeChanged(m_progressiveAAMode);
    update();
}

void QQuick3DSceneEnvironment::setMultisampleAAMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues multisampleAAMode)
{
    if (m_multisampleAAMode == multisampleAAMode)
        return;

    m_multisampleAAMode = multisampleAAMode;
    emit multisampleAAModeChanged(m_multisampleAAMode);
    update();
}

void QQuick3DSceneEnvironment::setBackgroundMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes backgroundMode)
{
    if (m_backgroundMode == backgroundMode)
        return;

    m_backgroundMode = backgroundMode;
    emit backgroundModeChanged(m_backgroundMode);
    update();
}

void QQuick3DSceneEnvironment::setClearColor(QColor clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged(m_clearColor);
    update();
}

void QQuick3DSceneEnvironment::setBlendType(QQuick3DSceneEnvironment::QQuick3DEnvironmentBlendTypes blendType)
{
    if (m_blendType == blendType)
        return;

    m_blendType = blendType;
    emit blendTypeChanged(m_blendType);
    update();
}

void QQuick3DSceneEnvironment::setAoStrength(float aoStrength)
{
    if (qFuzzyCompare(m_aoStrength, aoStrength))
        return;

    m_aoStrength = aoStrength;
    emit aoStrengthChanged(m_aoStrength);
    update();
}

void QQuick3DSceneEnvironment::setAoDistance(float aoDistance)
{
    if (qFuzzyCompare(m_aoDistance, aoDistance))
        return;

    m_aoDistance = aoDistance;
    emit aoDistanceChanged(m_aoDistance);
    update();
}

void QQuick3DSceneEnvironment::setAoSoftness(float aoSoftness)
{
    if (qFuzzyCompare(m_aoSoftness, aoSoftness))
        return;

    m_aoSoftness = aoSoftness;
    emit aoSoftnessChanged(m_aoSoftness);
    update();
}

void QQuick3DSceneEnvironment::setAoDither(bool aoDither)
{
    if (m_aoDither == aoDither)
        return;

    m_aoDither = aoDither;
    emit aoDitherChanged(m_aoDither);
    update();
}

void QQuick3DSceneEnvironment::setAoSampleRate(int aoSampleRate)
{
    if (m_aoSampleRate == aoSampleRate)
        return;

    m_aoSampleRate = aoSampleRate;
    emit aoSampleRateChanged(m_aoSampleRate);
    update();
}

void QQuick3DSceneEnvironment::setAoBias(float aoBias)
{
    if (qFuzzyCompare(m_aoBias, aoBias))
        return;

    m_aoBias = aoBias;
    emit aoBiasChanged(m_aoBias);
    update();
}

void QQuick3DSceneEnvironment::setShadowStrength(float shadowStrength)
{
    if (qFuzzyCompare(m_shadowStrength, shadowStrength))
        return;

    m_shadowStrength = shadowStrength;
    emit shadowStrengthChanged(m_shadowStrength);
    update();
}

void QQuick3DSceneEnvironment::setShadowDistance(float shadowDistance)
{
    if (qFuzzyCompare(m_shadowDistance, shadowDistance))
        return;

    m_shadowDistance = shadowDistance;
    emit shadowDistanceChanged(m_shadowDistance);
    update();
}

void QQuick3DSceneEnvironment::setShadowSoftness(float shadowSoftness)
{
    if (qFuzzyCompare(m_shadowSoftness, shadowSoftness))
        return;

    m_shadowSoftness = shadowSoftness;
    emit shadowSoftnessChanged(m_shadowSoftness);
    update();
}

void QQuick3DSceneEnvironment::setShadowBias(float shadowBias)
{
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    emit shadowBiasChanged(m_shadowBias);
    update();
}

void QQuick3DSceneEnvironment::setLightProbe(QQuick3DTexture *lightProbe)
{
    if (m_lightProbe == lightProbe)
        return;

    updateProperyListener(lightProbe, m_lightProbe, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setLightProbe(qobject_cast<QQuick3DTexture *>(n));
    });

    m_lightProbe = lightProbe;
    emit lightProbeChanged(m_lightProbe);
    update();
}

void QQuick3DSceneEnvironment::setProbeBrightness(float probeBrightness)
{
    if (qFuzzyCompare(m_probeBrightness, probeBrightness))
        return;

    m_probeBrightness = probeBrightness;
    emit probeBrightnessChanged(m_probeBrightness);
    update();
}

void QQuick3DSceneEnvironment::setFastIBL(bool fastIBL)
{
    if (m_fastIBL == fastIBL)
        return;

    m_fastIBL = fastIBL;
    emit fastIBLChanged(m_fastIBL);
    update();
}

void QQuick3DSceneEnvironment::setProbeHorizon(float probeHorizon)
{
    if (qFuzzyCompare(m_probeHorizon, probeHorizon))
        return;

    m_probeHorizon = probeHorizon;
    emit probeHorizonChanged(m_probeHorizon);
    update();
}

void QQuick3DSceneEnvironment::setProbeFieldOfView(float probeFieldOfView)
{
    if (qFuzzyCompare(m_probeFieldOfView, probeFieldOfView))
        return;

    m_probeFieldOfView = probeFieldOfView;
    emit probeFieldOfViewChanged(m_probeFieldOfView);
    update();
}

void QQuick3DSceneEnvironment::setLightProbe2(QQuick3DTexture *lightProbe2)
{
    if (m_lightProbe2 == lightProbe2)
        return;

    updateProperyListener(lightProbe2, m_lightProbe2, sceneRenderer(), m_connections, [this](QQuick3DObject *n) {
        setLightProbe2(qobject_cast<QQuick3DTexture *>(n));
    });

    m_lightProbe2 = lightProbe2;
    emit lightProbe2Changed(m_lightProbe2);
    update();
}

void QQuick3DSceneEnvironment::setProbe2Fade(float probe2Fade)
{
    if (qFuzzyCompare(m_probe2Fade, probe2Fade))
        return;

    m_probe2Fade = probe2Fade;
    emit probe2FadeChanged(m_probe2Fade);
    update();
}

void QQuick3DSceneEnvironment::setProbe2Window(float probe2Window)
{
    if (qFuzzyCompare(m_probe2Window, probe2Window))
        return;

    m_probe2Window = probe2Window;
    emit probe2WindowChanged(m_probe2Window);
    update();
}

void QQuick3DSceneEnvironment::setProbe2Postion(float probe2Postion)
{
    if (qFuzzyCompare(m_probe2Postion, probe2Postion))
        return;

    m_probe2Postion = probe2Postion;
    emit probe2PostionChanged(m_probe2Postion);
    update();
}

void QQuick3DSceneEnvironment::setIsDepthTestDisabled(bool isDepthTestDisabled)
{
    if (m_isDepthTestDisabled == isDepthTestDisabled)
        return;

    m_isDepthTestDisabled = isDepthTestDisabled;
    emit isDepthTestDisabledChanged(m_isDepthTestDisabled);
    update();
}

void QQuick3DSceneEnvironment::setIsDepthPrePassDisabled(bool isDepthPrePassDisabled)
{
    if (m_isDepthPrePassDisabled == isDepthPrePassDisabled)
        return;

    m_isDepthPrePassDisabled = isDepthPrePassDisabled;
    emit isDepthPrePassDisabledChanged(m_isDepthPrePassDisabled);
    update();
}

QDemonRenderGraphObject *QQuick3DSceneEnvironment::updateSpatialNode(QDemonRenderGraphObject *node)
{
    // Don't do anything, these properties get set by the scene renderer
    return node;
}

void QQuick3DSceneEnvironment::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneRenderer);
}

void QQuick3DSceneEnvironment::updateSceneManager(QQuick3DSceneManager *manager)
{
    if (manager) {
        if (m_lightProbe)
            QQuick3DObjectPrivate::get(m_lightProbe)->refSceneRenderer(manager);
        if (m_lightProbe2)
            QQuick3DObjectPrivate::get(m_lightProbe2)->refSceneRenderer(manager);
    } else {
        if (m_lightProbe)
            QQuick3DObjectPrivate::get(m_lightProbe)->derefSceneRenderer();
        if (m_lightProbe2)
            QQuick3DObjectPrivate::get(m_lightProbe2)->derefSceneRenderer();
    }
}

void QQuick3DSceneEnvironment::setTemporalAAEnabled(bool temporalAAEnabled)
{
    if (m_temporalAAEnabled == temporalAAEnabled)
        return;

    m_temporalAAEnabled = temporalAAEnabled;
    emit temporalAAEnabledChanged(m_temporalAAEnabled);
    update();
}

void QQuick3DSceneEnvironment::qmlAppendEffect(QQmlListProperty<QQuick3DEffect> *list, QQuick3DEffect *effect)
{
    if (effect == nullptr)
        return;
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    self->m_effects.push_back(effect);
}

QQuick3DEffect *QQuick3DSceneEnvironment::qmlEffectAt(QQmlListProperty<QQuick3DEffect> *list, int index)
{
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    return self->m_effects.at(index);
}

int QQuick3DSceneEnvironment::qmlEffectsCount(QQmlListProperty<QQuick3DEffect> *list)
{
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    return self->m_effects.count();
}

void QQuick3DSceneEnvironment::qmlClearEffects(QQmlListProperty<QQuick3DEffect> *list)
{
    QQuick3DSceneEnvironment *self = static_cast<QQuick3DSceneEnvironment *>(list->object);
    self->m_effects.clear();
}

QT_END_NAMESPACE
