#include "qdemondefaultmaterial.h"
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

QT_BEGIN_NAMESPACE

QDemonDefaultMaterial::QDemonDefaultMaterial()
{

}

QDemonDefaultMaterial::~QDemonDefaultMaterial()
{

}

QDemonObject::Type QDemonDefaultMaterial::type() const
{
    return QDemonObject::DefaultMaterial;
}



QDemonDefaultMaterial::DefaultMaterialLighting QDemonDefaultMaterial::lighting() const
{
    return m_lighting;
}

QDemonDefaultMaterial::DefaultMaterialBlendMode QDemonDefaultMaterial::blendMode() const
{
    return m_blendMode;
}

QColor QDemonDefaultMaterial::diffuseColor() const
{
    return m_diffuseColor;
}

QDemonImage *QDemonDefaultMaterial::diffuseMap() const
{
    return m_diffuseMap;
}

QDemonImage *QDemonDefaultMaterial::diffuseMap2() const
{
    return m_diffuseMap2;
}

QDemonImage *QDemonDefaultMaterial::diffuseMap3() const
{
    return m_diffuseMap3;
}

float QDemonDefaultMaterial::emissivePower() const
{
    return m_emissivePower;
}

QDemonImage *QDemonDefaultMaterial::emissiveMap() const
{
    return m_emissiveMap;
}

QColor QDemonDefaultMaterial::emissiveColor() const
{
    return m_emissiveColor;
}

QDemonImage *QDemonDefaultMaterial::specularReflectionMap() const
{
    return m_specularReflectionMap;
}

QDemonImage *QDemonDefaultMaterial::specularMap() const
{
    return m_specularMap;
}

QDemonDefaultMaterial::DefaultMaterialSpecularModel QDemonDefaultMaterial::specularModel() const
{
    return m_specularModel;
}

QColor QDemonDefaultMaterial::specularTint() const
{
    return m_specularTint;
}

float QDemonDefaultMaterial::indexOfRefraction() const
{
    return m_indexOfRefraction;
}

float QDemonDefaultMaterial::fresnelPower() const
{
    return m_fresnelPower;
}

float QDemonDefaultMaterial::specularAmount() const
{
    return m_specularAmount;
}

float QDemonDefaultMaterial::specularRoughness() const
{
    return m_specularRoughness;
}

QDemonImage *QDemonDefaultMaterial::roughnessMap() const
{
    return m_roughnessMap;
}

float QDemonDefaultMaterial::opacity() const
{
    return m_opacity;
}

QDemonImage *QDemonDefaultMaterial::opacityMap() const
{
    return m_opacityMap;
}

QDemonImage *QDemonDefaultMaterial::bumpMap() const
{
    return m_bumpMap;
}

float QDemonDefaultMaterial::bumpAmount() const
{
    return m_bumpAmount;
}

QDemonImage *QDemonDefaultMaterial::normalMap() const
{
    return m_normalMap;
}

QDemonImage *QDemonDefaultMaterial::translucencyMap() const
{
    return m_translucencyMap;
}

float QDemonDefaultMaterial::translucentFalloff() const
{
    return m_translucentFalloff;
}

float QDemonDefaultMaterial::diffuseLightWrap() const
{
    return m_diffuseLightWrap;
}

bool QDemonDefaultMaterial::vertexColors() const
{
    return m_vertexColors;
}

void QDemonDefaultMaterial::setLighting(QDemonDefaultMaterial::DefaultMaterialLighting lighting)
{
    if (m_lighting == lighting)
        return;

    m_lighting = lighting;
    emit lightingChanged(m_lighting);
}

void QDemonDefaultMaterial::setBlendMode(QDemonDefaultMaterial::DefaultMaterialBlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;

    m_blendMode = blendMode;
    emit blendModeChanged(m_blendMode);
}

void QDemonDefaultMaterial::setDiffuseColor(QColor diffuseColor)
{
    if (m_diffuseColor == diffuseColor)
        return;

    m_diffuseColor = diffuseColor;
    emit diffuseColorChanged(m_diffuseColor);
}

void QDemonDefaultMaterial::setDiffuseMap(QDemonImage *diffuseMap)
{
    if (m_diffuseMap == diffuseMap)
        return;

    m_diffuseMap = diffuseMap;
    emit diffuseMapChanged(m_diffuseMap);
}

void QDemonDefaultMaterial::setDiffuseMap2(QDemonImage *diffuseMap2)
{
    if (m_diffuseMap2 == diffuseMap2)
        return;

    m_diffuseMap2 = diffuseMap2;
    emit diffuseMap2Changed(m_diffuseMap2);
}

void QDemonDefaultMaterial::setDiffuseMap3(QDemonImage *diffuseMap3)
{
    if (m_diffuseMap3 == diffuseMap3)
        return;

    m_diffuseMap3 = diffuseMap3;
    emit diffuseMap3Changed(m_diffuseMap3);
}

void QDemonDefaultMaterial::setEmissivePower(float emissivePower)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_emissivePower, emissivePower))
        return;

    m_emissivePower = emissivePower;
    emit emissivePowerChanged(m_emissivePower);
}

void QDemonDefaultMaterial::setEmissiveMap(QDemonImage *emissiveMap)
{
    if (m_emissiveMap == emissiveMap)
        return;

    m_emissiveMap = emissiveMap;
    emit emissiveMapChanged(m_emissiveMap);
}

void QDemonDefaultMaterial::setEmissiveColor(QColor emissiveColor)
{
    if (m_emissiveColor == emissiveColor)
        return;

    m_emissiveColor = emissiveColor;
    emit emissiveColorChanged(m_emissiveColor);
}

void QDemonDefaultMaterial::setSpecularReflectionMap(QDemonImage *specularReflectionMap)
{
    if (m_specularReflectionMap == specularReflectionMap)
        return;

    m_specularReflectionMap = specularReflectionMap;
    emit specularReflectionMapChanged(m_specularReflectionMap);
}

void QDemonDefaultMaterial::setSpecularMap(QDemonImage *specularMap)
{
    if (m_specularMap == specularMap)
        return;

    m_specularMap = specularMap;
    emit specularMapChanged(m_specularMap);
}

void QDemonDefaultMaterial::setSpecularModel(QDemonDefaultMaterial::DefaultMaterialSpecularModel specularModel)
{
    if (m_specularModel == specularModel)
        return;

    m_specularModel = specularModel;
    emit specularModelChanged(m_specularModel);
}

void QDemonDefaultMaterial::setSpecularTint(QColor specularTint)
{
    if (m_specularTint == specularTint)
        return;

    m_specularTint = specularTint;
    emit specularTintChanged(m_specularTint);
}

void QDemonDefaultMaterial::setIndexOfRefraction(float indexOfRefraction)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_indexOfRefraction, indexOfRefraction))
        return;

    m_indexOfRefraction = indexOfRefraction;
    emit indexOfRefractionChanged(m_indexOfRefraction);
}

void QDemonDefaultMaterial::setFresnelPower(float fresnelPower)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_fresnelPower, fresnelPower))
        return;

    m_fresnelPower = fresnelPower;
    emit fresnelPowerChanged(m_fresnelPower);
}

void QDemonDefaultMaterial::setSpecularAmount(float specularAmount)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_specularAmount, specularAmount))
        return;

    m_specularAmount = specularAmount;
    emit specularAmountChanged(m_specularAmount);
}

void QDemonDefaultMaterial::setSpecularRoughness(float specularRoughness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_specularRoughness, specularRoughness))
        return;

    m_specularRoughness = specularRoughness;
    emit specularRoughnessChanged(m_specularRoughness);
}

void QDemonDefaultMaterial::setRoughnessMap(QDemonImage *roughnessMap)
{
    if (m_roughnessMap == roughnessMap)
        return;

    m_roughnessMap = roughnessMap;
    emit roughnessMapChanged(m_roughnessMap);
}

void QDemonDefaultMaterial::setOpacity(float opacity)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    emit opacityChanged(m_opacity);
}

void QDemonDefaultMaterial::setOpacityMap(QDemonImage *opacityMap)
{
    if (m_opacityMap == opacityMap)
        return;

    m_opacityMap = opacityMap;
    emit opacityMapChanged(m_opacityMap);
}

void QDemonDefaultMaterial::setBumpMap(QDemonImage *bumpMap)
{
    if (m_bumpMap == bumpMap)
        return;

    m_bumpMap = bumpMap;
    emit bumpMapChanged(m_bumpMap);
}

void QDemonDefaultMaterial::setBumpAmount(float bumpAmount)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_bumpAmount, bumpAmount))
        return;

    m_bumpAmount = bumpAmount;
    emit bumpAmountChanged(m_bumpAmount);
}

void QDemonDefaultMaterial::setNormalMap(QDemonImage *normalMap)
{
    if (m_normalMap == normalMap)
        return;

    m_normalMap = normalMap;
    emit normalMapChanged(m_normalMap);
}

void QDemonDefaultMaterial::setTranslucencyMap(QDemonImage *translucencyMap)
{
    if (m_translucencyMap == translucencyMap)
        return;

    m_translucencyMap = translucencyMap;
    emit translucencyMapChanged(m_translucencyMap);
}

void QDemonDefaultMaterial::setTranslucentFalloff(float translucentFalloff)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_translucentFalloff, translucentFalloff))
        return;

    m_translucentFalloff = translucentFalloff;
    emit translucentFalloffChanged(m_translucentFalloff);
}

void QDemonDefaultMaterial::setDiffuseLightWrap(float diffuseLightWrap)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_diffuseLightWrap, diffuseLightWrap))
        return;

    m_diffuseLightWrap = diffuseLightWrap;
    emit diffuseLightWrapChanged(m_diffuseLightWrap);
}

void QDemonDefaultMaterial::setVertexColors(bool vertexColors)
{
    if (m_vertexColors == vertexColors)
        return;

    m_vertexColors = vertexColors;
    emit vertexColorsChanged(m_vertexColors);
}

QDemonGraphObject *QDemonDefaultMaterial::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonRenderDefaultMaterial();

    QDemonMaterial::updateSpatialNode(node);

    // TODO: Add Default Material update

    return node;
}

QT_END_NAMESPACE
