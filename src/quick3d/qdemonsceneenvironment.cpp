#include "qdemonsceneenvironment.h"

QT_BEGIN_NAMESPACE

//static void updateProperyListener(QDemonObject *newO, QDemonObject *oldO, QDemonWindow *window, QHash<QObject*, QMetaObject::Connection> &connections, std::function<void(QDemonObject *o)> callFn) {
//    // disconnect previous destruction listern
//    if (oldO) {
//        if (window)
//            QDemonObjectPrivate::get(oldO)->derefWindow();

//        auto connection = connections.find(oldO);
//        if (connection != connections.end()) {
//            QObject::disconnect(connection.value());
//            connections.erase(connection);
//        }
//    }

//    // listen for new map's destruction
//    if (newO) {
//        if (window)
//            QDemonObjectPrivate::get(newO)->refWindow(window);
//        auto connection = QObject::connect(newO, &QObject::destroyed, [callFn](){
//            callFn(nullptr);
//        });
//        connections.insert(newO, connection);
//    }
//}

QDemonSceneEnvironment::QDemonSceneEnvironment(QObject *parent)
    : QObject(parent)
{

}

QDemonSceneEnvironment::~QDemonSceneEnvironment()
{
//    for (auto connection : m_connections)
//        disconnect(connection);
}

QDemonSceneEnvironment::QDemonEnvironmentAAModeValues QDemonSceneEnvironment::progressiveAAMode() const
{
    return m_progressiveAAMode;
}

QDemonSceneEnvironment::QDemonEnvironmentAAModeValues QDemonSceneEnvironment::multisampleAAMode() const
{
    return m_multisampleAAMode;
}

QDemonSceneEnvironment::QDemonEnvironmentBackgroundTypes QDemonSceneEnvironment::backgroundMode() const
{
    return m_backgroundMode;
}

QColor QDemonSceneEnvironment::clearColor() const
{
    return m_clearColor;
}

QDemonSceneEnvironment::QDemonEnvironmentBlendTypes QDemonSceneEnvironment::blendType() const
{
    return m_blendType;
}

float QDemonSceneEnvironment::aoStrength() const
{
    return m_aoStrength;
}

float QDemonSceneEnvironment::aoDistance() const
{
    return m_aoDistance;
}

float QDemonSceneEnvironment::aoSoftness() const
{
    return m_aoSoftness;
}

bool QDemonSceneEnvironment::aoDither() const
{
    return m_aoDither;
}

int QDemonSceneEnvironment::aoSampleRate() const
{
    return m_aoSampleRate;
}

float QDemonSceneEnvironment::aoBias() const
{
    return m_aoBias;
}

float QDemonSceneEnvironment::shadowStrength() const
{
    return m_shadowStrength;
}

float QDemonSceneEnvironment::shadowDistance() const
{
    return m_shadowDistance;
}

float QDemonSceneEnvironment::shadowSoftness() const
{
    return m_shadowSoftness;
}

float QDemonSceneEnvironment::shadowBias() const
{
    return m_shadowBias;
}

QDemonImage *QDemonSceneEnvironment::lightProbe() const
{
    return m_lightProbe;
}

float QDemonSceneEnvironment::probeBrightness() const
{
    return m_probeBrightness;
}

bool QDemonSceneEnvironment::fastIBL() const
{
    return m_fastIBL;
}

float QDemonSceneEnvironment::probeHorizon() const
{
    return m_probeHorizon;
}

float QDemonSceneEnvironment::probeFieldOfView() const
{
    return m_probeFieldOfView;
}

QDemonImage *QDemonSceneEnvironment::lightProbe2() const
{
    return m_lightProbe2;
}

float QDemonSceneEnvironment::probe2Fade() const
{
    return m_probe2Fade;
}

float QDemonSceneEnvironment::probe2Window() const
{
    return m_probe2Window;
}

float QDemonSceneEnvironment::probe2Postion() const
{
    return m_probe2Postion;
}

bool QDemonSceneEnvironment::temporalAAEnabled() const
{
    return m_temporalAAEnabled;
}

QQmlListProperty<QDemonEffect> QDemonSceneEnvironment::effectsList()
{
    return QQmlListProperty<QDemonEffect>(this,
                                          nullptr,
                                          QDemonSceneEnvironment::qmlAppendEffect,
                                          QDemonSceneEnvironment::qmlEffectsCount,
                                          QDemonSceneEnvironment::qmlEffectAt,
                                          QDemonSceneEnvironment::qmlClearEffects);
}

void QDemonSceneEnvironment::setProgressiveAAMode(QDemonSceneEnvironment::QDemonEnvironmentAAModeValues progressiveAAMode)
{
    if (m_progressiveAAMode == progressiveAAMode)
        return;

    m_progressiveAAMode = progressiveAAMode;
    emit progressiveAAModeChanged(m_progressiveAAMode);
}

void QDemonSceneEnvironment::setMultisampleAAMode(QDemonSceneEnvironment::QDemonEnvironmentAAModeValues multisampleAAMode)
{
    if (m_multisampleAAMode == multisampleAAMode)
        return;

    m_multisampleAAMode = multisampleAAMode;
    emit multisampleAAModeChanged(m_multisampleAAMode);
}

void QDemonSceneEnvironment::setBackgroundMode(QDemonSceneEnvironment::QDemonEnvironmentBackgroundTypes backgroundMode)
{
    if (m_backgroundMode == backgroundMode)
        return;

    m_backgroundMode = backgroundMode;
    emit backgroundModeChanged(m_backgroundMode);
}

void QDemonSceneEnvironment::setClearColor(QColor clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged(m_clearColor);
}

void QDemonSceneEnvironment::setBlendType(QDemonSceneEnvironment::QDemonEnvironmentBlendTypes blendType)
{
    if (m_blendType == blendType)
        return;

    m_blendType = blendType;
    emit blendTypeChanged(m_blendType);
}

void QDemonSceneEnvironment::setAoStrength(float aoStrength)
{
    if (qFuzzyCompare(m_aoStrength, aoStrength))
        return;

    m_aoStrength = aoStrength;
    emit aoStrengthChanged(m_aoStrength);
}

void QDemonSceneEnvironment::setAoDistance(float aoDistance)
{
    if (qFuzzyCompare(m_aoDistance, aoDistance))
        return;

    m_aoDistance = aoDistance;
    emit aoDistanceChanged(m_aoDistance);
}

void QDemonSceneEnvironment::setAoSoftness(float aoSoftness)
{
    if (qFuzzyCompare(m_aoSoftness, aoSoftness))
        return;

    m_aoSoftness = aoSoftness;
    emit aoSoftnessChanged(m_aoSoftness);
}

void QDemonSceneEnvironment::setAoDither(bool aoDither)
{
    if (m_aoDither == aoDither)
        return;

    m_aoDither = aoDither;
    emit aoDitherChanged(m_aoDither);
}

void QDemonSceneEnvironment::setAoSampleRate(int aoSampleRate)
{
    if (m_aoSampleRate == aoSampleRate)
        return;

    m_aoSampleRate = aoSampleRate;
    emit aoSampleRateChanged(m_aoSampleRate);
}

void QDemonSceneEnvironment::setAoBias(float aoBias)
{
    if (qFuzzyCompare(m_aoBias, aoBias))
        return;

    m_aoBias = aoBias;
    emit aoBiasChanged(m_aoBias);
}

void QDemonSceneEnvironment::setShadowStrength(float shadowStrength)
{
    if (qFuzzyCompare(m_shadowStrength, shadowStrength))
        return;

    m_shadowStrength = shadowStrength;
    emit shadowStrengthChanged(m_shadowStrength);
}

void QDemonSceneEnvironment::setShadowDistance(float shadowDistance)
{
    if (qFuzzyCompare(m_shadowDistance, shadowDistance))
        return;

    m_shadowDistance = shadowDistance;
    emit shadowDistanceChanged(m_shadowDistance);
}

void QDemonSceneEnvironment::setShadowSoftness(float shadowSoftness)
{
    if (qFuzzyCompare(m_shadowSoftness, shadowSoftness))
        return;

    m_shadowSoftness = shadowSoftness;
    emit shadowSoftnessChanged(m_shadowSoftness);
}

void QDemonSceneEnvironment::setShadowBias(float shadowBias)
{
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    emit shadowBiasChanged(m_shadowBias);
}

void QDemonSceneEnvironment::setLightProbe(QDemonImage *lightProbe)
{
    if (m_lightProbe == lightProbe)
        return;

//    updateProperyListener(lightProbe, m_lightProbe, window(), m_connections, [this](QDemonObject *n) {
//        setLightProbe(qobject_cast<QDemonImage *>(n));
//    });

    m_lightProbe = lightProbe;
    emit lightProbeChanged(m_lightProbe);
}

void QDemonSceneEnvironment::setProbeBrightness(float probeBrightness)
{
    if (qFuzzyCompare(m_probeBrightness, probeBrightness))
        return;

    m_probeBrightness = probeBrightness;
    emit probeBrightnessChanged(m_probeBrightness);
}

void QDemonSceneEnvironment::setFastIBL(bool fastIBL)
{
    if (m_fastIBL == fastIBL)
        return;

    m_fastIBL = fastIBL;
    emit fastIBLChanged(m_fastIBL);
}

void QDemonSceneEnvironment::setProbeHorizon(float probeHorizon)
{
    if (qFuzzyCompare(m_probeHorizon, probeHorizon))
        return;

    m_probeHorizon = probeHorizon;
    emit probeHorizonChanged(m_probeHorizon);
}

void QDemonSceneEnvironment::setProbeFieldOfView(float probeFieldOfView)
{
    if (qFuzzyCompare(m_probeFieldOfView, probeFieldOfView))
        return;

    m_probeFieldOfView = probeFieldOfView;
    emit probeFieldOfViewChanged(m_probeFieldOfView);
}

void QDemonSceneEnvironment::setLightProbe2(QDemonImage *lightProbe2)
{
    if (m_lightProbe2 == lightProbe2)
        return;

//    updateProperyListener(lightProbe2, m_lightProbe2, window(), m_connections, [this](QDemonObject *n) {
//        setLightProbe2(qobject_cast<QDemonImage *>(n));
//    });

    m_lightProbe2 = lightProbe2;
    emit lightProbe2Changed(m_lightProbe2);
}

void QDemonSceneEnvironment::setProbe2Fade(float probe2Fade)
{
    if (qFuzzyCompare(m_probe2Fade, probe2Fade))
        return;

    m_probe2Fade = probe2Fade;
    emit probe2FadeChanged(m_probe2Fade);
}

void QDemonSceneEnvironment::setProbe2Window(float probe2Window)
{
    if (qFuzzyCompare(m_probe2Window, probe2Window))
        return;

    m_probe2Window = probe2Window;
    emit probe2WindowChanged(m_probe2Window);
}

void QDemonSceneEnvironment::setProbe2Postion(float probe2Postion)
{
    if (qFuzzyCompare(m_probe2Postion, probe2Postion))
        return;

    m_probe2Postion = probe2Postion;
    emit probe2PostionChanged(m_probe2Postion);
}

void QDemonSceneEnvironment::setTemporalAAEnabled(bool temporalAAEnabled)
{
    if (m_temporalAAEnabled == temporalAAEnabled)
        return;

    m_temporalAAEnabled = temporalAAEnabled;
    emit temporalAAEnabledChanged(m_temporalAAEnabled);
}

void QDemonSceneEnvironment::qmlAppendEffect(QQmlListProperty<QDemonEffect> *list, QDemonEffect *effect)
{
    if (effect == nullptr)
        return;
    QDemonSceneEnvironment *self = static_cast<QDemonSceneEnvironment *>(list->object);
    self->m_effects.push_back(effect);
}

QDemonEffect *QDemonSceneEnvironment::qmlEffectAt(QQmlListProperty<QDemonEffect> *list, int index)
{
    QDemonSceneEnvironment *self = static_cast<QDemonSceneEnvironment *>(list->object);
    return self->m_effects.at(index);
}

int QDemonSceneEnvironment::qmlEffectsCount(QQmlListProperty<QDemonEffect> *list)
{
    QDemonSceneEnvironment *self = static_cast<QDemonSceneEnvironment *>(list->object);
    return self->m_effects.count();
}

void QDemonSceneEnvironment::qmlClearEffects(QQmlListProperty<QDemonEffect> *list)
{
    QDemonSceneEnvironment *self = static_cast<QDemonSceneEnvironment *>(list->object);
    self->m_effects.clear();
}

QT_END_NAMESPACE
