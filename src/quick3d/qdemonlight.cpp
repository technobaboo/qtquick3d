#include "qdemonlight.h"

#include <QtDemonRuntimeRender/qdemonrenderlight.h>

QT_BEGIN_NAMESPACE

QDemonLight::QDemonLight()
{

}

QDemonLight::~QDemonLight()
{

}

QDemonObject::Type QDemonLight::type() const
{
    return QDemonObject::Light;
}

QDemonLight::RenderLightTypes QDemonLight::lightType() const
{
    return m_lightType;
}

QColor QDemonLight::diffuseColor() const
{
    return m_diffuseColor;
}

QColor QDemonLight::specularColor() const
{
    return m_specularColor;
}

QColor QDemonLight::ambientColor() const
{
    return m_ambientColor;
}

float QDemonLight::brightness() const
{
    return m_brightness;
}

float QDemonLight::linearFade() const
{
    return m_linearFade;
}

float QDemonLight::exponentialFade() const
{
    return m_exponentialFade;
}

float QDemonLight::areaWidth() const
{
    return m_areaWidth;
}

float QDemonLight::areaHeight() const
{
    return m_areaHeight;
}

bool QDemonLight::castShadow() const
{
    return m_castShadow;
}

float QDemonLight::shadowBias() const
{
    return m_shadowBias;
}

float QDemonLight::shadowFactor() const
{
    return m_shadowFactor;
}

int QDemonLight::shadowMapResolution() const
{
    return m_shadowMapResolution;
}

float QDemonLight::shadowMapFar() const
{
    return m_shadowMapFar;
}

float QDemonLight::shadowMapFieldOfView() const
{
    return m_shadowMapFieldOfView;
}

float QDemonLight::shadowFilter() const
{
    return m_shadowFilter;
}

void QDemonLight::setLightType(QDemonLight::RenderLightTypes lightType)
{
    if (m_lightType == lightType)
        return;

    m_lightType = lightType;
    emit lightTypeChanged(m_lightType);
    update();
}

void QDemonLight::setDiffuseColor(QColor diffuseColor)
{
    if (m_diffuseColor == diffuseColor)
        return;

    m_diffuseColor = diffuseColor;
    emit diffuseColorChanged(m_diffuseColor);
    update();
}

void QDemonLight::setSpecularColor(QColor specularColor)
{
    if (m_specularColor == specularColor)
        return;

    m_specularColor = specularColor;
    emit specularColorChanged(m_specularColor);
    update();
}

void QDemonLight::setAmbientColor(QColor ambientColor)
{
    if (m_ambientColor == ambientColor)
        return;

    m_ambientColor = ambientColor;
    emit ambientColorChanged(m_ambientColor);
    update();
}

void QDemonLight::setBrightness(float brightness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_brightness, brightness))
        return;

    m_brightness = brightness;
    emit brightnessChanged(m_brightness);
    update();
}

void QDemonLight::setLinearFade(float linearFade)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_linearFade, linearFade))
        return;

    m_linearFade = linearFade;
    emit linearFadeChanged(m_linearFade);
    update();
}

void QDemonLight::setExponentialFade(float exponentialFade)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_exponentialFade, exponentialFade))
        return;

    m_exponentialFade = exponentialFade;
    emit exponentialFadeChanged(m_exponentialFade);
    update();
}

void QDemonLight::setAreaWidth(float areaWidth)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_areaWidth, areaWidth))
        return;

    m_areaWidth = areaWidth;
    emit areaWidthChanged(m_areaWidth);
    update();
}

void QDemonLight::setAreaHeight(float areaHeight)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_areaHeight, areaHeight))
        return;

    m_areaHeight = areaHeight;
    emit areaHeightChanged(m_areaHeight);
    update();
}

void QDemonLight::setCastShadow(bool castShadow)
{
    if (m_castShadow == castShadow)
        return;

    m_castShadow = castShadow;
    emit castShadowChanged(m_castShadow);
    update();
}

void QDemonLight::setShadowBias(float shadowBias)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    emit shadowBiasChanged(m_shadowBias);
    update();
}

void QDemonLight::setShadowFactor(float shadowFactor)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowFactor, shadowFactor))
        return;

    m_shadowFactor = shadowFactor;
    emit shadowFactorChanged(m_shadowFactor);
    update();
}

void QDemonLight::setShadowMapResolution(int shadowMapResolution)
{
    if (m_shadowMapResolution == shadowMapResolution)
        return;

    m_shadowMapResolution = shadowMapResolution;
    emit shadowMapResolutionChanged(m_shadowMapResolution);
    update();
}

void QDemonLight::setShadowMapFar(float shadowMapFar)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowMapFar, shadowMapFar))
        return;

    m_shadowMapFar = shadowMapFar;
    emit shadowMapFarChanged(m_shadowMapFar);
    update();
}

void QDemonLight::setShadowMapFieldOfView(float shadowMapFieldOfView)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowMapFieldOfView, shadowMapFieldOfView))
        return;

    m_shadowMapFieldOfView = shadowMapFieldOfView;
    emit shadowMapFieldOfViewChanged(m_shadowMapFieldOfView);
    update();
}

void QDemonLight::setShadowFilter(float shadowFilter)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowFilter, shadowFilter))
        return;

    m_shadowFilter = shadowFilter;
    emit shadowFilterChanged(m_shadowFilter);
    update();
}

QDemonGraphObject *QDemonLight::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonRenderLight();

    // ### TODO: update light properties

    return node;
}

QT_END_NAMESPACE
