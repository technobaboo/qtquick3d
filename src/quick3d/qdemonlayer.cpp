#include "qdemonlayer.h"

#include <QtDemonRuntimeRender/qdemonrenderlayer.h>

#include <qdemonimage.h>
QT_BEGIN_NAMESPACE

QDemonLayer::QDemonLayer() {}

QDemonLayer::~QDemonLayer() {}

QDemonObject::Type QDemonLayer::type() const
{
    return QDemonObject::Layer;
}

QString QDemonLayer::texturePath() const
{
    return m_texturePath;
}

QDemonLayer::QDemonAAModeValues QDemonLayer::progressiveAAMode() const
{
    return m_progressiveAAMode;
}

QDemonLayer::QDemonAAModeValues QDemonLayer::multisampleAAMode() const
{
    return m_multisampleAAMode;
}

QDemonLayer::QDemonLayerBackgroundTypes QDemonLayer::backgroundMode() const
{
    return m_backgroundMode;
}

QColor QDemonLayer::clearColor() const
{
    return m_clearColor;
}

QDemonLayer::QDemonLayerBlendTypes QDemonLayer::blendType() const
{
    return m_blendType;
}

QDemonLayer::QDemonHorizontalFieldValues QDemonLayer::horizontalFieldValue() const
{
    return m_horizontalFieldValue;
}

QDemonLayer::QDemonVerticalFieldValues QDemonLayer::verticalFieldValue() const
{
    return m_verticalFieldValue;
}

QDemonLayer::QDemonLayerUnitTypes QDemonLayer::leftUnits() const
{
    return m_leftUnits;
}

QDemonLayer::QDemonLayerUnitTypes QDemonLayer::rightUnits() const
{
    return m_rightUnits;
}

QDemonLayer::QDemonLayerUnitTypes QDemonLayer::topUnits() const
{
    return m_topUnits;
}

QDemonLayer::QDemonLayerUnitTypes QDemonLayer::bottomUnits() const
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
    return QQmlListProperty<QDemonEffect>(this,
                                          nullptr,
                                          QDemonLayer::qmlAppendEffect,
                                          QDemonLayer::qmlEffectsCount,
                                          QDemonLayer::qmlEffectAt,
                                          QDemonLayer::qmlClearEffects);
}

QDemonCamera *QDemonLayer::activeCamera() const
{
    return m_activeCamera;
}

QDemonLayer::QDemonLayerUnitTypes QDemonLayer::widthUnits() const
{
    return m_widthUnits;
}

QDemonLayer::QDemonLayerUnitTypes QDemonLayer::heightUnits() const
{
    return m_heightUnits;
}

void QDemonLayer::setTexturePath(QString texturePath)
{
    if (m_texturePath == texturePath)
        return;

    m_texturePath = texturePath;
    emit texturePathChanged(m_texturePath);
    markDirty(RenderTarget);
}

void QDemonLayer::setProgressiveAAMode(QDemonLayer::QDemonAAModeValues progressiveAAMode)
{
    if (m_progressiveAAMode == progressiveAAMode)
        return;

    m_progressiveAAMode = progressiveAAMode;
    emit progressiveAAModeChanged(m_progressiveAAMode);
    markDirty(AntiAliasing);
}

void QDemonLayer::setMultisampleAAMode(QDemonLayer::QDemonAAModeValues multisampleAAMode)
{
    if (m_multisampleAAMode == multisampleAAMode)
        return;

    m_multisampleAAMode = multisampleAAMode;
    emit multisampleAAModeChanged(m_multisampleAAMode);
    markDirty(AntiAliasing);
}

void QDemonLayer::setBackgroundMode(QDemonLayer::QDemonLayerBackgroundTypes backgroundMode)
{
    if (m_backgroundMode == backgroundMode)
        return;

    m_backgroundMode = backgroundMode;
    emit backgroundModeChanged(m_backgroundMode);
    markDirty(Background);
}

void QDemonLayer::setClearColor(QColor clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged(m_clearColor);
    markDirty(Background);
}

void QDemonLayer::setBlendType(QDemonLayer::QDemonLayerBlendTypes blendType)
{
    if (m_blendType == blendType)
        return;

    m_blendType = blendType;
    emit blendTypeChanged(m_blendType);
    markDirty(Blending);
}

void QDemonLayer::setHorizontalFieldValue(QDemonLayer::QDemonHorizontalFieldValues horizontalFieldValue)
{
    if (m_horizontalFieldValue == horizontalFieldValue)
        return;

    m_horizontalFieldValue = horizontalFieldValue;
    emit horizontalFieldValueChanged(m_horizontalFieldValue);
    markDirty(Layout);
}

void QDemonLayer::setVerticalFieldValue(QDemonLayer::QDemonVerticalFieldValues verticalFieldValue)
{
    if (m_verticalFieldValue == verticalFieldValue)
        return;

    m_verticalFieldValue = verticalFieldValue;
    emit verticalFieldValueChanged(m_verticalFieldValue);
    markDirty(Layout);
}

void QDemonLayer::setLeftUnits(QDemonLayer::QDemonLayerUnitTypes leftUnits)
{
    if (m_leftUnits == leftUnits)
        return;

    m_leftUnits = leftUnits;
    emit leftUnitsChanged(m_leftUnits);
    markDirty(Layout);
}

void QDemonLayer::setRightUnits(QDemonLayer::QDemonLayerUnitTypes rightUnits)
{
    if (m_rightUnits == rightUnits)
        return;

    m_rightUnits = rightUnits;
    emit rightUnitsChanged(m_rightUnits);
    markDirty(Layout);
}

void QDemonLayer::setTopUnits(QDemonLayer::QDemonLayerUnitTypes topUnits)
{
    if (m_topUnits == topUnits)
        return;

    m_topUnits = topUnits;
    emit topUnitsChanged(m_topUnits);
    markDirty(Layout);
}

void QDemonLayer::setBottomUnits(QDemonLayer::QDemonLayerUnitTypes bottomUnits)
{
    if (m_bottomUnits == bottomUnits)
        return;

    m_bottomUnits = bottomUnits;
    emit bottomUnitsChanged(m_bottomUnits);
    markDirty(Layout);
}

void QDemonLayer::setLeft(float left)
{
    if (qFuzzyCompare(m_left, left))
        return;

    m_left = left;
    emit leftChanged(m_left);
    markDirty(Layout);
}

void QDemonLayer::setRight(float right)
{
    if (qFuzzyCompare(m_right, right))
        return;

    m_right = right;
    emit rightChanged(m_right);
    markDirty(Layout);
}

void QDemonLayer::setTop(float top)
{
    if (qFuzzyCompare(m_top, top))
        return;

    m_top = top;
    emit topChanged(m_top);
    markDirty(Layout);
}

void QDemonLayer::setBottom(float bottom)
{
    if (qFuzzyCompare(m_bottom, bottom))
        return;

    m_bottom = bottom;
    emit bottomChanged(m_bottom);
    markDirty(Layout);
}

void QDemonLayer::setHeight(float height)
{
    if (qFuzzyCompare(m_height, height))
        return;

    m_height = height;
    emit heightChanged(m_height);
    markDirty(Layout);
}

void QDemonLayer::setWidth(float width)
{
    if (qFuzzyCompare(m_width, width))
        return;

    m_width = width;
    emit widthChanged(m_width);
    markDirty(Layout);
}

void QDemonLayer::setAoStrength(float aoStrength)
{
    if (qFuzzyCompare(m_aoStrength, aoStrength))
        return;

    m_aoStrength = aoStrength;
    emit aoStrengthChanged(m_aoStrength);
    markDirty(AmbientOcclusion);
}

void QDemonLayer::setAoDistance(float aoDistance)
{
    if (qFuzzyCompare(m_aoDistance, aoDistance))
        return;

    m_aoDistance = aoDistance;
    emit aoDistanceChanged(m_aoDistance);
    markDirty(AmbientOcclusion);
}

void QDemonLayer::setAoSoftness(float aoSoftness)
{
    if (qFuzzyCompare(m_aoSoftness, aoSoftness))
        return;

    m_aoSoftness = aoSoftness;
    emit aoSoftnessChanged(m_aoSoftness);
    markDirty(AmbientOcclusion);
}

void QDemonLayer::setAoDither(bool aoDither)
{
    if (m_aoDither == aoDither)
        return;

    m_aoDither = aoDither;
    emit aoDitherChanged(m_aoDither);
    markDirty(AmbientOcclusion);
}

void QDemonLayer::setAoSampleRate(int aoSampleRate)
{
    if (m_aoSampleRate == aoSampleRate)
        return;

    m_aoSampleRate = aoSampleRate;
    emit aoSampleRateChanged(m_aoSampleRate);
    markDirty(AmbientOcclusion);
}

void QDemonLayer::setAoBias(float aoBias)
{
    if (qFuzzyCompare(m_aoBias, aoBias))
        return;

    m_aoBias = aoBias;
    emit aoBiasChanged(m_aoBias);
    markDirty(AmbientOcclusion);
}

void QDemonLayer::setShadowStrength(float shadowStrength)
{
    if (qFuzzyCompare(m_shadowStrength, shadowStrength))
        return;

    m_shadowStrength = shadowStrength;
    emit shadowStrengthChanged(m_shadowStrength);
    markDirty(Shadow);
}

void QDemonLayer::setShadowDistance(float shadowDistance)
{
    if (qFuzzyCompare(m_shadowDistance, shadowDistance))
        return;

    m_shadowDistance = shadowDistance;
    emit shadowDistanceChanged(m_shadowDistance);
    markDirty(Shadow);
}

void QDemonLayer::setShadowSoftness(float shadowSoftness)
{
    if (qFuzzyCompare(m_shadowSoftness, shadowSoftness))
        return;

    m_shadowSoftness = shadowSoftness;
    emit shadowSoftnessChanged(m_shadowSoftness);
    markDirty(Shadow);
}

void QDemonLayer::setShadowBias(float shadowBias)
{
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    emit shadowBiasChanged(m_shadowBias);
    markDirty(Shadow);
}

void QDemonLayer::setLightProbe(QDemonImage *lightProbe)
{
    if (m_lightProbe == lightProbe)
        return;

    m_lightProbe = lightProbe;
    emit lightProbeChanged(m_lightProbe);
    markDirty(LightProbe1);
}

void QDemonLayer::setProbeBrightness(float probeBrightness)
{
    if (qFuzzyCompare(m_probeBrightness, probeBrightness))
        return;

    m_probeBrightness = probeBrightness;
    emit probeBrightnessChanged(m_probeBrightness);
    markDirty(LightProbe1);
}

void QDemonLayer::setFastIBL(bool fastIBL)
{
    if (m_fastIBL == fastIBL)
        return;

    m_fastIBL = fastIBL;
    emit fastIBLChanged(m_fastIBL);
    markDirty(LightProbe1);
}

void QDemonLayer::setProbeHorizon(float probeHorizon)
{
    if (qFuzzyCompare(m_probeHorizon, probeHorizon))
        return;

    m_probeHorizon = probeHorizon;
    emit probeHorizonChanged(m_probeHorizon);
    markDirty(LightProbe1);
}

void QDemonLayer::setProbeFieldOfView(float probeFieldOfView)
{
    if (qFuzzyCompare(m_probeFieldOfView, probeFieldOfView))
        return;

    m_probeFieldOfView = probeFieldOfView;
    emit probeFieldOfViewChanged(m_probeFieldOfView);
    markDirty(LightProbe1);
}

void QDemonLayer::setLightProbe2(QDemonImage *lightProbe2)
{
    if (m_lightProbe2 == lightProbe2)
        return;

    m_lightProbe2 = lightProbe2;
    emit lightProbe2Changed(m_lightProbe2);
    markDirty(LightProbe2);
}

void QDemonLayer::setProbe2Fade(float probe2Fade)
{
    if (qFuzzyCompare(m_probe2Fade, probe2Fade))
        return;

    m_probe2Fade = probe2Fade;
    emit probe2FadeChanged(m_probe2Fade);
    markDirty(LightProbe2);
}

void QDemonLayer::setProbe2Window(float probe2Window)
{
    if (qFuzzyCompare(m_probe2Window, probe2Window))
        return;

    m_probe2Window = probe2Window;
    emit probe2WindowChanged(m_probe2Window);
    markDirty(LightProbe2);
}

void QDemonLayer::setProbe2Postion(float probe2Postion)
{
    if (qFuzzyCompare(m_probe2Postion, probe2Postion))
        return;

    m_probe2Postion = probe2Postion;
    emit probe2PostionChanged(m_probe2Postion);
    markDirty(LightProbe2);
}

void QDemonLayer::setTemporalAAEnabled(bool temporalAAEnabled)
{
    if (m_temporalAAEnabled == temporalAAEnabled)
        return;

    m_temporalAAEnabled = temporalAAEnabled;
    emit temporalAAEnabledChanged(m_temporalAAEnabled);
    markDirty(AntiAliasing);
}

void QDemonLayer::setActiveCamera(QDemonCamera *camera)
{
    if (m_activeCamera == camera)
        return;
    m_activeCamera = camera;
    emit activeCameraChanged(m_activeCamera);
    markDirty(Camera);
}

void QDemonLayer::setWidthUnits(QDemonLayer::QDemonLayerUnitTypes widthUnits)
{
    if (m_widthUnits == widthUnits)
        return;

    m_widthUnits = widthUnits;
    emit widthUnitsChanged(m_widthUnits);
    markDirty(Layout);
}

void QDemonLayer::setHeightUnits(QDemonLayer::QDemonLayerUnitTypes heightUnits)
{
    if (m_heightUnits == heightUnits)
        return;

    m_heightUnits = heightUnits;
    emit heightUnitsChanged(m_heightUnits);
    markDirty(Layout);
}

QDemonGraphObject *QDemonLayer::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonRenderLayer();

    // Update super properties
    QDemonNode::updateSpatialNode(node);

    QDemonRenderLayer *layerNode = static_cast<QDemonRenderLayer *>(node);
    if (m_dirtyAttributes & RenderTarget)
        layerNode->texturePath = m_texturePath;
    if (m_dirtyAttributes & AntiAliasing) {
        layerNode->progressiveAAMode = AAModeValues::Enum(m_progressiveAAMode);
        layerNode->multisampleAAMode = AAModeValues::Enum(m_multisampleAAMode);
        layerNode->temporalAAEnabled = m_temporalAAEnabled;
    }

    if (m_dirtyAttributes & Background) {
        layerNode->background = LayerBackground::Enum(m_backgroundMode);
        layerNode->clearColor = QVector3D(m_clearColor.redF(), m_clearColor.greenF(), m_clearColor.blueF());
    }
    if (m_dirtyAttributes & Layout) {
        layerNode->m_height = m_height;
        layerNode->m_width = m_width;
        layerNode->blendType = LayerBlendTypes::Enum(m_blendType);
        layerNode->horizontalFieldValues = HorizontalFieldValues::Enum(m_horizontalFieldValue);
        layerNode->m_left = m_left;
        layerNode->leftUnits = LayerUnitTypes::Enum(m_leftUnits);
        layerNode->m_right = m_right;
        layerNode->rightUnits = LayerUnitTypes::Enum(m_rightUnits);
        layerNode->m_top = m_top;
        layerNode->topUnits = LayerUnitTypes::Enum(m_topUnits);
        layerNode->m_bottom = m_bottom;
        layerNode->bottomUnits = LayerUnitTypes::Enum(m_bottomUnits);
        layerNode->m_width = m_width;
        layerNode->widthUnits = LayerUnitTypes::Enum(m_widthUnits);
        layerNode->m_height = m_height;
        layerNode->heightUnits = LayerUnitTypes::Enum(m_heightUnits);
    }

    if (m_dirtyAttributes & AmbientOcclusion) {
        layerNode->aoStrength = m_aoStrength;
        layerNode->aoDistance = m_aoDistance;
        layerNode->aoSoftness = m_aoSoftness;
        layerNode->aoBias = m_aoBias;
        layerNode->aoSamplerate = m_aoSampleRate;
        layerNode->aoDither = m_aoDither;
    }

    if (m_dirtyAttributes & Shadow) {
        layerNode->shadowStrength = m_shadowStrength;
        layerNode->shadowDist = m_shadowDistance;
        layerNode->shadowSoftness = m_shadowSoftness;
        layerNode->shadowBias = m_shadowBias;
    }

    if (m_dirtyAttributes & LightProbe1) {
        if (m_lightProbe)
            layerNode->lightProbe = m_lightProbe->getRenderImage();
        else
            layerNode->lightProbe = nullptr;

        layerNode->probeBright = m_probeBrightness;
        layerNode->fastIbl = m_fastIBL;
        layerNode->probeHorizon = m_probeHorizon;
        layerNode->probeFov = m_probeFieldOfView;
    }

    if (m_dirtyAttributes & LightProbe2) {
        if (m_lightProbe2)
            layerNode->lightProbe2 = m_lightProbe->getRenderImage();
        else
            layerNode->lightProbe2 = nullptr;

        layerNode->probe2Fade = m_probe2Fade;
        layerNode->probe2Window = m_probe2Window;
        layerNode->probe2Pos = m_probe2Postion;
    }

    // ### Make sure effects are also handled
    if (m_dirtyAttributes & Effect) {
    }

    // ### Make sure active camera is correct
    if (m_dirtyAttributes & Camera) {
    }

    m_dirtyAttributes = 0;

    return layerNode;
}

void QDemonLayer::markDirty(QDemonLayer::QDemonLayerDirtyType type)
{
    if (!(m_dirtyAttributes & type)) {
        m_dirtyAttributes |= type;
        update();
    }
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
