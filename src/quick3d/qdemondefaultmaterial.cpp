#include "qdemondefaultmaterial.h"
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

QT_BEGIN_NAMESPACE

QDemonDefaultMaterial::QDemonDefaultMaterial() : m_diffuseColor(Qt::white), m_specularTint(Qt::white) {}

QDemonDefaultMaterial::~QDemonDefaultMaterial() {}

QDemonObject::Type QDemonDefaultMaterial::type() const
{
    return QDemonObject::DefaultMaterial;
}

QDemonDefaultMaterial::QDemonDefaultMaterialLighting QDemonDefaultMaterial::lighting() const
{
    return m_lighting;
}

QDemonDefaultMaterial::QDemonDefaultMaterialBlendMode QDemonDefaultMaterial::blendMode() const
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

QDemonDefaultMaterial::QDemonDefaultMaterialSpecularModel QDemonDefaultMaterial::specularModel() const
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

void QDemonDefaultMaterial::setLighting(QDemonDefaultMaterial::QDemonDefaultMaterialLighting lighting)
{
    if (m_lighting == lighting)
        return;

    m_lighting = lighting;
    emit lightingChanged(m_lighting);
    markDirty(LightingModeDirty);
}

void QDemonDefaultMaterial::setBlendMode(QDemonDefaultMaterial::QDemonDefaultMaterialBlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;

    m_blendMode = blendMode;
    emit blendModeChanged(m_blendMode);
    markDirty(BlendModeDirty);
}

void QDemonDefaultMaterial::setDiffuseColor(QColor diffuseColor)
{
    if (m_diffuseColor == diffuseColor)
        return;

    m_diffuseColor = diffuseColor;
    emit diffuseColorChanged(m_diffuseColor);
    markDirty(DiffuseDirty);
}

void QDemonDefaultMaterial::setDiffuseMap(QDemonImage *diffuseMap)
{
    if (m_diffuseMap == diffuseMap)
        return;

    m_diffuseMap = diffuseMap;
    emit diffuseMapChanged(m_diffuseMap);
    markDirty(DiffuseDirty);
}

void QDemonDefaultMaterial::setDiffuseMap2(QDemonImage *diffuseMap2)
{
    if (m_diffuseMap2 == diffuseMap2)
        return;

    m_diffuseMap2 = diffuseMap2;
    emit diffuseMap2Changed(m_diffuseMap2);
    markDirty(DiffuseDirty);
}

void QDemonDefaultMaterial::setDiffuseMap3(QDemonImage *diffuseMap3)
{
    if (m_diffuseMap3 == diffuseMap3)
        return;

    m_diffuseMap3 = diffuseMap3;
    emit diffuseMap3Changed(m_diffuseMap3);
    markDirty(DiffuseDirty);
}

void QDemonDefaultMaterial::setEmissivePower(float emissivePower)
{
    if (qFuzzyCompare(m_emissivePower, emissivePower))
        return;

    m_emissivePower = emissivePower;
    emit emissivePowerChanged(m_emissivePower);
    markDirty(EmissiveDirty);
}

void QDemonDefaultMaterial::setEmissiveMap(QDemonImage *emissiveMap)
{
    if (m_emissiveMap == emissiveMap)
        return;

    m_emissiveMap = emissiveMap;
    emit emissiveMapChanged(m_emissiveMap);
    markDirty(EmissiveDirty);
}

void QDemonDefaultMaterial::setEmissiveColor(QColor emissiveColor)
{
    if (m_emissiveColor == emissiveColor)
        return;

    m_emissiveColor = emissiveColor;
    emit emissiveColorChanged(m_emissiveColor);
    markDirty(EmissiveDirty);
}

void QDemonDefaultMaterial::setSpecularReflectionMap(QDemonImage *specularReflectionMap)
{
    if (m_specularReflectionMap == specularReflectionMap)
        return;

    m_specularReflectionMap = specularReflectionMap;
    emit specularReflectionMapChanged(m_specularReflectionMap);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setSpecularMap(QDemonImage *specularMap)
{
    if (m_specularMap == specularMap)
        return;

    m_specularMap = specularMap;
    emit specularMapChanged(m_specularMap);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setSpecularModel(QDemonDefaultMaterial::QDemonDefaultMaterialSpecularModel specularModel)
{
    if (m_specularModel == specularModel)
        return;

    m_specularModel = specularModel;
    emit specularModelChanged(m_specularModel);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setSpecularTint(QColor specularTint)
{
    if (m_specularTint == specularTint)
        return;

    m_specularTint = specularTint;
    emit specularTintChanged(m_specularTint);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setIndexOfRefraction(float indexOfRefraction)
{
    if (qFuzzyCompare(m_indexOfRefraction, indexOfRefraction))
        return;

    m_indexOfRefraction = indexOfRefraction;
    emit indexOfRefractionChanged(m_indexOfRefraction);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setFresnelPower(float fresnelPower)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_fresnelPower, fresnelPower))
        return;

    m_fresnelPower = fresnelPower;
    emit fresnelPowerChanged(m_fresnelPower);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setSpecularAmount(float specularAmount)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_specularAmount, specularAmount))
        return;

    m_specularAmount = specularAmount;
    emit specularAmountChanged(m_specularAmount);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setSpecularRoughness(float specularRoughness)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_specularRoughness, specularRoughness))
        return;

    m_specularRoughness = specularRoughness;
    emit specularRoughnessChanged(m_specularRoughness);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setRoughnessMap(QDemonImage *roughnessMap)
{
    if (m_roughnessMap == roughnessMap)
        return;

    m_roughnessMap = roughnessMap;
    emit roughnessMapChanged(m_roughnessMap);
    markDirty(SpecularDirty);
}

void QDemonDefaultMaterial::setOpacity(float opacity)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    emit opacityChanged(m_opacity);
    markDirty(OpacityDirty);
}

void QDemonDefaultMaterial::setOpacityMap(QDemonImage *opacityMap)
{
    if (m_opacityMap == opacityMap)
        return;

    m_opacityMap = opacityMap;
    emit opacityMapChanged(m_opacityMap);
    markDirty(OpacityDirty);
}

void QDemonDefaultMaterial::setBumpMap(QDemonImage *bumpMap)
{
    if (m_bumpMap == bumpMap)
        return;

    m_bumpMap = bumpMap;
    emit bumpMapChanged(m_bumpMap);
    markDirty(BumpDirty);
}

void QDemonDefaultMaterial::setBumpAmount(float bumpAmount)
{
    if (qFuzzyCompare(m_bumpAmount, bumpAmount))
        return;

    m_bumpAmount = bumpAmount;
    emit bumpAmountChanged(m_bumpAmount);
    markDirty(BumpDirty);
}

void QDemonDefaultMaterial::setNormalMap(QDemonImage *normalMap)
{
    if (m_normalMap == normalMap)
        return;

    m_normalMap = normalMap;
    emit normalMapChanged(m_normalMap);
    markDirty(NormalDirty);
}

void QDemonDefaultMaterial::setTranslucencyMap(QDemonImage *translucencyMap)
{
    if (m_translucencyMap == translucencyMap)
        return;

    m_translucencyMap = translucencyMap;
    emit translucencyMapChanged(m_translucencyMap);
    markDirty(TranslucencyDirty);
}

void QDemonDefaultMaterial::setTranslucentFalloff(float translucentFalloff)
{
    if (qFuzzyCompare(m_translucentFalloff, translucentFalloff))
        return;

    m_translucentFalloff = translucentFalloff;
    emit translucentFalloffChanged(m_translucentFalloff);
    markDirty(TranslucencyDirty);
}

void QDemonDefaultMaterial::setDiffuseLightWrap(float diffuseLightWrap)
{
    if (qFuzzyCompare(m_diffuseLightWrap, diffuseLightWrap))
        return;

    m_diffuseLightWrap = diffuseLightWrap;
    emit diffuseLightWrapChanged(m_diffuseLightWrap);
    markDirty(DiffuseDirty);
}

void QDemonDefaultMaterial::setVertexColors(bool vertexColors)
{
    if (m_vertexColors == vertexColors)
        return;

    m_vertexColors = vertexColors;
    emit vertexColorsChanged(m_vertexColors);
    markDirty(VertexColorsDirty);
}

QDemonGraphObject *QDemonDefaultMaterial::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonRenderDefaultMaterial();

    // Set common material properties
    QDemonMaterial::updateSpatialNode(node);

    QDemonRenderDefaultMaterial *material = static_cast<QDemonRenderDefaultMaterial *>(node);

    if (m_dirtyAttributes & LightingModeDirty)
        material->lighting = DefaultMaterialLighting(m_lighting);

    if (m_dirtyAttributes & BlendModeDirty)
        material->blendMode = DefaultMaterialBlendMode(m_blendMode);

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

        material->specularModel = DefaultMaterialSpecularModel(m_specularModel);
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

void QDemonDefaultMaterial::markDirty(QDemonDefaultMaterial::QDemonDefaultMaterialDirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

QT_END_NAMESPACE
