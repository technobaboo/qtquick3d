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

QDemonLayer::LayerBackground QDemonLayer::backgroundMode() const
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

float QDemonLayer::aoDither() const
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

void QDemonLayer::setTexturePath(QString texturePath)
{
    if (m_texturePath == texturePath)
        return;

    m_texturePath = texturePath;
    emit texturePathChanged(m_texturePath);
}

void QDemonLayer::setProgressiveAAMode(QDemonLayer::AAModeValues progressiveAAMode)
{
    if (m_progressiveAAMode == progressiveAAMode)
        return;

    m_progressiveAAMode = progressiveAAMode;
    emit progressiveAAModeChanged(m_progressiveAAMode);
}

void QDemonLayer::setMultisampleAAMode(QDemonLayer::AAModeValues multisampleAAMode)
{
    if (m_multisampleAAMode == multisampleAAMode)
        return;

    m_multisampleAAMode = multisampleAAMode;
    emit multisampleAAModeChanged(m_multisampleAAMode);
}

void QDemonLayer::setBackgroundMode(QDemonLayer::LayerBackground backgroundMode)
{
    if (m_backgroundMode == backgroundMode)
        return;

    m_backgroundMode = backgroundMode;
    emit backgroundModeChanged(m_backgroundMode);
}

void QDemonLayer::setClearColor(QColor clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged(m_clearColor);
}

void QDemonLayer::setBlendType(QDemonLayer::LayerBlendTypes blendType)
{
    if (m_blendType == blendType)
        return;

    m_blendType = blendType;
    emit blendTypeChanged(m_blendType);
}

void QDemonLayer::setHorizontalFieldValue(QDemonLayer::HorizontalFieldValues horizontalFieldValue)
{
    if (m_horizontalFieldValue == horizontalFieldValue)
        return;

    m_horizontalFieldValue = horizontalFieldValue;
    emit horizontalFieldValueChanged(m_horizontalFieldValue);
}

void QDemonLayer::setVerticalFieldValue(QDemonLayer::VerticalFieldValues verticalFieldValue)
{
    if (m_verticalFieldValue == verticalFieldValue)
        return;

    m_verticalFieldValue = verticalFieldValue;
    emit verticalFieldValueChanged(m_verticalFieldValue);
}

void QDemonLayer::setLeftUnits(QDemonLayer::LayerUnitTypes leftUnits)
{
    if (m_leftUnits == leftUnits)
        return;

    m_leftUnits = leftUnits;
    emit leftUnitsChanged(m_leftUnits);
}

void QDemonLayer::setRightUnits(QDemonLayer::LayerUnitTypes rightUnits)
{
    if (m_rightUnits == rightUnits)
        return;

    m_rightUnits = rightUnits;
    emit rightUnitsChanged(m_rightUnits);
}

void QDemonLayer::setTopUnits(QDemonLayer::LayerUnitTypes topUnits)
{
    if (m_topUnits == topUnits)
        return;

    m_topUnits = topUnits;
    emit topUnitsChanged(m_topUnits);
}

void QDemonLayer::setBottomUnits(QDemonLayer::LayerUnitTypes bottomUnits)
{
    if (m_bottomUnits == bottomUnits)
        return;

    m_bottomUnits = bottomUnits;
    emit bottomUnitsChanged(m_bottomUnits);
}

void QDemonLayer::setLeft(float left)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_left, left))
        return;

    m_left = left;
    emit leftChanged(m_left);
}

void QDemonLayer::setRight(float right)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_right, right))
        return;

    m_right = right;
    emit rightChanged(m_right);
}

void QDemonLayer::setTop(float top)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_top, top))
        return;

    m_top = top;
    emit topChanged(m_top);
}

void QDemonLayer::setBottom(float bottom)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_bottom, bottom))
        return;

    m_bottom = bottom;
    emit bottomChanged(m_bottom);
}

void QDemonLayer::setHeight(float height)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_height, height))
        return;

    m_height = height;
    emit heightChanged(m_height);
}

void QDemonLayer::setWidth(float width)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_width, width))
        return;

    m_width = width;
    emit widthChanged(m_width);
}

void QDemonLayer::setAoStrength(float aoStrength)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoStrength, aoStrength))
        return;

    m_aoStrength = aoStrength;
    emit aoStrengthChanged(m_aoStrength);
}

void QDemonLayer::setAoDistance(float aoDistance)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoDistance, aoDistance))
        return;

    m_aoDistance = aoDistance;
    emit aoDistanceChanged(m_aoDistance);
}

void QDemonLayer::setAoSoftness(float aoSoftness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoSoftness, aoSoftness))
        return;

    m_aoSoftness = aoSoftness;
    emit aoSoftnessChanged(m_aoSoftness);
}

void QDemonLayer::setAoDither(float aoDither)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoDither, aoDither))
        return;

    m_aoDither = aoDither;
    emit aoDitherChanged(m_aoDither);
}

void QDemonLayer::setAoSampleRate(int aoSampleRate)
{
    if (m_aoSampleRate == aoSampleRate)
        return;

    m_aoSampleRate = aoSampleRate;
    emit aoSampleRateChanged(m_aoSampleRate);
}

void QDemonLayer::setAoBias(float aoBias)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_aoBias, aoBias))
        return;

    m_aoBias = aoBias;
    emit aoBiasChanged(m_aoBias);
}

void QDemonLayer::setShadowStrength(float shadowStrength)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowStrength, shadowStrength))
        return;

    m_shadowStrength = shadowStrength;
    emit shadowStrengthChanged(m_shadowStrength);
}

void QDemonLayer::setShadowDistance(float shadowDistance)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowDistance, shadowDistance))
        return;

    m_shadowDistance = shadowDistance;
    emit shadowDistanceChanged(m_shadowDistance);
}

void QDemonLayer::setShadowSoftness(float shadowSoftness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowSoftness, shadowSoftness))
        return;

    m_shadowSoftness = shadowSoftness;
    emit shadowSoftnessChanged(m_shadowSoftness);
}

void QDemonLayer::setShadowBias(float shadowBias)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    emit shadowBiasChanged(m_shadowBias);
}

void QDemonLayer::setLightProbe(QDemonImage *lightProbe)
{
    if (m_lightProbe == lightProbe)
        return;

    m_lightProbe = lightProbe;
    emit lightProbeChanged(m_lightProbe);
}

void QDemonLayer::setProbeBrightness(float probeBrightness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probeBrightness, probeBrightness))
        return;

    m_probeBrightness = probeBrightness;
    emit probeBrightnessChanged(m_probeBrightness);
}

void QDemonLayer::setFastIBL(bool fastIBL)
{
    if (m_fastIBL == fastIBL)
        return;

    m_fastIBL = fastIBL;
    emit fastIBLChanged(m_fastIBL);
}

void QDemonLayer::setProbeHorizon(float probeHorizon)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probeHorizon, probeHorizon))
        return;

    m_probeHorizon = probeHorizon;
    emit probeHorizonChanged(m_probeHorizon);
}

void QDemonLayer::setProbeFieldOfView(float probeFieldOfView)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probeFieldOfView, probeFieldOfView))
        return;

    m_probeFieldOfView = probeFieldOfView;
    emit probeFieldOfViewChanged(m_probeFieldOfView);
}

void QDemonLayer::setLightProbe2(QDemonImage *lightProbe2)
{
    if (m_lightProbe2 == lightProbe2)
        return;

    m_lightProbe2 = lightProbe2;
    emit lightProbe2Changed(m_lightProbe2);
}

void QDemonLayer::setProbe2Fade(float probe2Fade)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probe2Fade, probe2Fade))
        return;

    m_probe2Fade = probe2Fade;
    emit probe2FadeChanged(m_probe2Fade);
}

void QDemonLayer::setProbe2Window(float probe2Window)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probe2Window, probe2Window))
        return;

    m_probe2Window = probe2Window;
    emit probe2WindowChanged(m_probe2Window);
}

void QDemonLayer::setProbe2Postion(float probe2Postion)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_probe2Postion, probe2Postion))
        return;

    m_probe2Postion = probe2Postion;
    emit probe2PostionChanged(m_probe2Postion);
}

void QDemonLayer::setTemporalAAEnabled(bool temporalAAEnabled)
{
    if (m_temporalAAEnabled == temporalAAEnabled)
        return;

    m_temporalAAEnabled = temporalAAEnabled;
    emit temporalAAEnabledChanged(m_temporalAAEnabled);
}

SGraphObject *QDemonLayer::updateSpatialNode(SGraphObject *node)
{
    if (!node)
        node = new SLayer();

    // Update super properties
    QDemonNode::updateSpatialNode(node);


    // TODO: Update layer properties

    return node;
}

QT_END_NAMESPACE
