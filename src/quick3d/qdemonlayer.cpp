#include "qdemonlayer.h"

#include <QtDemonRuntimeRender/qdemonrenderlayer.h>

#include <qdemonimage.h>
QT_BEGIN_NAMESPACE


QDemonLayer::QDemonLayer()
{

}

QDemonLayer::~QDemonLayer()
{

}

QDemonObject::Type QDemonLayer::type() const
{
    return QDemonObject::Layer;
}

QString QDemonLayer::texturePath() const
{
    return m_texturePath;
}

QDemonLayer::AAModeValues QDemonLayer::progressiveAAMode() const
{
    return m_progressiveAAMode;
}

QDemonLayer::AAModeValues QDemonLayer::multisampleAAMode() const
{
    return m_multisampleAAMode;
}

QDemonLayer::LayerBackgroundTypes QDemonLayer::backgroundMode() const
{
    return m_backgroundMode;
}

QColor QDemonLayer::clearColor() const
{
    return m_clearColor;
}

QDemonLayer::LayerBlendTypes QDemonLayer::blendType() const
{
    return m_blendType;
}

QDemonLayer::HorizontalFieldValues QDemonLayer::horizontalFieldValue() const
{
    return m_horizontalFieldValue;
}

QDemonLayer::VerticalFieldValues QDemonLayer::verticalFieldValue() const
{
    return m_verticalFieldValue;
}

QDemonLayer::LayerUnitTypes QDemonLayer::leftUnits() const
{
    return m_leftUnits;
}

QDemonLayer::LayerUnitTypes QDemonLayer::rightUnits() const
{
    return m_rightUnits;
}

QDemonLayer::LayerUnitTypes QDemonLayer::topUnits() const
{
    return m_topUnits;
}

QDemonLayer::LayerUnitTypes QDemonLayer::bottomUnits() const
{
    return m_bottomUnits;
}

float QDemonLayer::left() const
{
    return m_left;
}

float QDemonLayer::right() const
{
    return m_right;
}

float QDemonLayer::top() const
{
    return m_top;
}

float QDemonLayer::bottom() const
{
    return m_bottom;
}

float QDemonLayer::height() const
{
    return m_height;
}

float QDemonLayer::width() const
{
    return m_width;
}

float QDemonLayer::aoStrength() const
{
    return m_aoStrength;
}

float QDemonLayer::aoDistance() const
{
    return m_aoDistance;
}

float QDemonLayer::aoSoftness() const
{
    return m_aoSoftness;
}

bool QDemonLayer::aoDither() const
{
    return m_aoDither;
}

int QDemonLayer::aoSampleRate() const
{
    return m_aoSampleRate;
}

float QDemonLayer::aoBias() const
{
    return m_aoBias;
}

float QDemonLayer::shadowStrength() const
{
    return m_shadowStrength;
}

float QDemonLayer::shadowDistance() const
{
    return m_shadowDistance;
}

float QDemonLayer::shadowSoftness() const
{
    return m_shadowSoftness;
}

float QDemonLayer::shadowBias() const
{
    return m_shadowBias;
}

QDemonImage *QDemonLayer::lightProbe() const
{
    return m_lightProbe;
}

float QDemonLayer::probeBrightness() const
{
    return m_probeBrightness;
}

bool QDemonLayer::fastIBL() const
{
    return m_fastIBL;
}

float QDemonLayer::probeHorizon() const
{
    return m_probeHorizon;
}

float QDemonLayer::probeFieldOfView() const
{
    return m_probeFieldOfView;
}

QDemonImage *QDemonLayer::lightProbe2() const
{
    return m_lightProbe2;
}

float QDemonLayer::probe2Fade() const
{
    return m_probe2Fade;
}

float QDemonLayer::probe2Window() const
{
    return m_probe2Window;
}

float QDemonLayer::probe2Postion() const
{
    return m_probe2Postion;
}

bool QDemonLayer::temporalAAEnabled() const
{
    return m_temporalAAEnabled;
}

QQmlListProperty<QDemonEffect> QDemonLayer::effectsList()
{
    return QQmlListProperty<QDemonEffect>(this, nullptr,
                                          QDemonLayer::qmlAppendEffect,
                                          QDemonLayer::qmlEffectsCount,
                                          QDemonLayer::qmlEffectAt,
                                          QDemonLayer::qmlClearEffects);
}

QDemonCamera *QDemonLayer::activeCamera() const
{
    return m_activeCamera;
}

void QDemonLayer::setTexturePath(QString texturePath)
{
    if (m_texturePath == texturePath)
        return;

    m_texturePath = texturePath;
    emit texturePathChanged(m_texturePath);
    update();
}

void QDemonLayer::setProgressiveAAMode(QDemonLayer::AAModeValues progressiveAAMode)
{
    if (m_progressiveAAMode == progressiveAAMode)
        return;

    m_progressiveAAMode = progressiveAAMode;
    emit progressiveAAModeChanged(m_progressiveAAMode);
    update();
}

void QDemonLayer::setMultisampleAAMode(QDemonLayer::AAModeValues multisampleAAMode)
{
    if (m_multisampleAAMode == multisampleAAMode)
        return;

    m_multisampleAAMode = multisampleAAMode;
    emit multisampleAAModeChanged(m_multisampleAAMode);
    update();
}

void QDemonLayer::setBackgroundMode(QDemonLayer::LayerBackgroundTypes backgroundMode)
{
    if (m_backgroundMode == backgroundMode)
        return;

    m_backgroundMode = backgroundMode;
    emit backgroundModeChanged(m_backgroundMode);
    update();
}

void QDemonLayer::setClearColor(QColor clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged(m_clearColor);
    update();
}

void QDemonLayer::setBlendType(QDemonLayer::LayerBlendTypes blendType)
{
    if (m_blendType == blendType)
        return;

    m_blendType = blendType;
    emit blendTypeChanged(m_blendType);
    update();
}

void QDemonLayer::setHorizontalFieldValue(QDemonLayer::HorizontalFieldValues horizontalFieldValue)
{
    if (m_horizontalFieldValue == horizontalFieldValue)
        return;

    m_horizontalFieldValue = horizontalFieldValue;
    emit horizontalFieldValueChanged(m_horizontalFieldValue);
    update();
}

void QDemonLayer::setVerticalFieldValue(QDemonLayer::VerticalFieldValues verticalFieldValue)
{
    if (m_verticalFieldValue == verticalFieldValue)
        return;

    m_verticalFieldValue = verticalFieldValue;
    emit verticalFieldValueChanged(m_verticalFieldValue);
    update();
}

void QDemonLayer::setLeftUnits(QDemonLayer::LayerUnitTypes leftUnits)
{
    if (m_leftUnits == leftUnits)
        return;

    m_leftUnits = leftUnits;
    emit leftUnitsChanged(m_leftUnits);
    update();
}

void QDemonLayer::setRightUnits(QDemonLayer::LayerUnitTypes rightUnits)
{
    if (m_rightUnits == rightUnits)
        return;

    m_rightUnits = rightUnits;
    emit rightUnitsChanged(m_rightUnits);
    update();
}

void QDemonLayer::setTopUnits(QDemonLayer::LayerUnitTypes topUnits)
{
    if (m_topUnits == topUnits)
        return;

    m_topUnits = topUnits;
    emit topUnitsChanged(m_topUnits);
    update();
}

void QDemonLayer::setBottomUnits(QDemonLayer::LayerUnitTypes bottomUnits)
{
    if (m_bottomUnits == bottomUnits)
        return;

    m_bottomUnits = bottomUnits;
    emit bottomUnitsChanged(m_bottomUnits);
    update();
}

void QDemonLayer::setLeft(float left)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_left, left))
        return;

    m_left = left;
    emit leftChanged(m_left);
    update();
}

void QDemonLayer::setRight(float right)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_right, right))
        return;

    m_right = right;
    emit rightChanged(m_right);
    update();
}

void QDemonLayer::setTop(float top)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_top, top))
        return;

    m_top = top;
    emit topChanged(m_top);
    update();
}

void QDemonLayer::setBottom(float bottom)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_bottom, bottom))
        return;

    m_bottom = bottom;
    emit bottomChanged(m_bottom);
    update();
}

void QDemonLayer::setHeight(float height)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_height, height))
        return;

    m_height = height;
    emit heightChanged(m_height);
    update();
}

void QDemonLayer::setWidth(float width)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_width, width))
        return;

    m_width = width;
    emit widthChanged(m_width);
    update();
}

void QDemonLayer::setAoStrength(float aoStrength)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoStrength, aoStrength))
        return;

    m_aoStrength = aoStrength;
    emit aoStrengthChanged(m_aoStrength);
    update();
}

void QDemonLayer::setAoDistance(float aoDistance)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoDistance, aoDistance))
        return;

    m_aoDistance = aoDistance;
    emit aoDistanceChanged(m_aoDistance);
    update();
}

void QDemonLayer::setAoSoftness(float aoSoftness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoSoftness, aoSoftness))
        return;

    m_aoSoftness = aoSoftness;
    emit aoSoftnessChanged(m_aoSoftness);
    update();
}

void QDemonLayer::setAoDither(bool aoDither)
{
    if (m_aoDither == aoDither)
        return;

    m_aoDither = aoDither;
    emit aoDitherChanged(m_aoDither);
    update();
}

void QDemonLayer::setAoSampleRate(int aoSampleRate)
{
    if (m_aoSampleRate == aoSampleRate)
        return;

    m_aoSampleRate = aoSampleRate;
    emit aoSampleRateChanged(m_aoSampleRate);
    update();
}

void QDemonLayer::setAoBias(float aoBias)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoBias, aoBias))
        return;

    m_aoBias = aoBias;
    emit aoBiasChanged(m_aoBias);
    update();
}

void QDemonLayer::setShadowStrength(float shadowStrength)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowStrength, shadowStrength))
        return;

    m_shadowStrength = shadowStrength;
    emit shadowStrengthChanged(m_shadowStrength);
    update();
}

void QDemonLayer::setShadowDistance(float shadowDistance)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowDistance, shadowDistance))
        return;

    m_shadowDistance = shadowDistance;
    emit shadowDistanceChanged(m_shadowDistance);
    update();
}

void QDemonLayer::setShadowSoftness(float shadowSoftness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowSoftness, shadowSoftness))
        return;

    m_shadowSoftness = shadowSoftness;
    emit shadowSoftnessChanged(m_shadowSoftness);
    update();
}

void QDemonLayer::setShadowBias(float shadowBias)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    emit shadowBiasChanged(m_shadowBias);
    update();
}

void QDemonLayer::setLightProbe(QDemonImage *lightProbe)
{
    if (m_lightProbe == lightProbe)
        return;

    m_lightProbe = lightProbe;
    emit lightProbeChanged(m_lightProbe);
    update();
}

void QDemonLayer::setProbeBrightness(float probeBrightness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probeBrightness, probeBrightness))
        return;

    m_probeBrightness = probeBrightness;
    emit probeBrightnessChanged(m_probeBrightness);
    update();
}

void QDemonLayer::setFastIBL(bool fastIBL)
{
    if (m_fastIBL == fastIBL)
        return;

    m_fastIBL = fastIBL;
    emit fastIBLChanged(m_fastIBL);
    update();
}

void QDemonLayer::setProbeHorizon(float probeHorizon)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probeHorizon, probeHorizon))
        return;

    m_probeHorizon = probeHorizon;
    emit probeHorizonChanged(m_probeHorizon);
    update();
}

void QDemonLayer::setProbeFieldOfView(float probeFieldOfView)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probeFieldOfView, probeFieldOfView))
        return;

    m_probeFieldOfView = probeFieldOfView;
    emit probeFieldOfViewChanged(m_probeFieldOfView);
    update();
}

void QDemonLayer::setLightProbe2(QDemonImage *lightProbe2)
{
    if (m_lightProbe2 == lightProbe2)
        return;

    m_lightProbe2 = lightProbe2;
    emit lightProbe2Changed(m_lightProbe2);
    update();
}

void QDemonLayer::setProbe2Fade(float probe2Fade)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probe2Fade, probe2Fade))
        return;

    m_probe2Fade = probe2Fade;
    emit probe2FadeChanged(m_probe2Fade);
    update();
}

void QDemonLayer::setProbe2Window(float probe2Window)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probe2Window, probe2Window))
        return;

    m_probe2Window = probe2Window;
    emit probe2WindowChanged(m_probe2Window);
    update();
}

void QDemonLayer::setProbe2Postion(float probe2Postion)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probe2Postion, probe2Postion))
        return;

    m_probe2Postion = probe2Postion;
    emit probe2PostionChanged(m_probe2Postion);
    update();
}

void QDemonLayer::setTemporalAAEnabled(bool temporalAAEnabled)
{
    if (m_temporalAAEnabled == temporalAAEnabled)
        return;

    m_temporalAAEnabled = temporalAAEnabled;
    emit temporalAAEnabledChanged(m_temporalAAEnabled);
    update();
}

void QDemonLayer::setActiveCamera(QDemonCamera *camera)
{
    if (m_activeCamera == camera)
        return;
    m_activeCamera = camera;
    emit activeCameraChanged(m_activeCamera);
    update();
}

QDemonGraphObject *QDemonLayer::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonRenderLayer();

    // Update super properties
    QDemonNode::updateSpatialNode(node);

    QDemonRenderLayer *layerNode = static_cast<QDemonRenderLayer*>(node);
    //    layerNode->m_TexturePath = m_texturePath;
    //    layerNode->m_ProgressiveAAMode = m_progressiveAAMode;
    //    layerNode->m_MultisampleAAMode = m_multisampleAAMode;
    //    layerNode->m_Background = m_backgroundMode;
    layerNode->background = LayerBackground::Color;
    layerNode->clearColor = QVector3D(m_clearColor.redF(),
                                        m_clearColor.greenF(),
                                        m_clearColor.blueF());
    layerNode->m_height = m_height;
    layerNode->m_width = m_width;
    //    layerNode->m_BlendType = m_blendType;
    //    layerNode->m_HorizontalFieldValues = m_horizontalFieldValue;
    //    layerNode->m_Left = m_left;
    //    layerNode->m_LeftUnits =

    return layerNode;
}

void QDemonLayer::qmlAppendEffect(QQmlListProperty<QDemonEffect> *list, QDemonEffect *effect)
{
    if (effect == nullptr)
        return;
    QDemonLayer *self = static_cast<QDemonLayer *>(list->object);
    self->m_effects.push_back(effect);
}

QDemonEffect *QDemonLayer::qmlEffectAt(QQmlListProperty<QDemonEffect> *list, int index)
{
    QDemonLayer *self = static_cast<QDemonLayer *>(list->object);
    return self->m_effects.at(index);
}

int QDemonLayer::qmlEffectsCount(QQmlListProperty<QDemonEffect> *list)
{
    QDemonLayer *self = static_cast<QDemonLayer *>(list->object);
    return self->m_effects.count();
}

void QDemonLayer::qmlClearEffects(QQmlListProperty<QDemonEffect> *list)
{
    QDemonLayer *self = static_cast<QDemonLayer *>(list->object);
    self->m_effects.clear();
}

QT_END_NAMESPACE
