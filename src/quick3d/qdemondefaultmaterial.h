#ifndef QDEMONDEFAULTMATERIAL_H
#define QDEMONDEFAULTMATERIAL_H

#include <qdemonmaterial.h>
#include <qdemonimage.h>
#include <QColor>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonDefaultMaterial : public QDemonMaterial
{
    Q_OBJECT
    Q_PROPERTY(DefaultMaterialLighting lighting READ lighting WRITE setLighting NOTIFY lightingChanged)
    Q_PROPERTY(DefaultMaterialBlendMode blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)

    Q_PROPERTY(QColor diffuseColor READ diffuseColor WRITE setDiffuseColor NOTIFY diffuseColorChanged)
    Q_PROPERTY(QDemonImage* diffuseMap READ diffuseMap WRITE setDiffuseMap NOTIFY diffuseMapChanged)
    Q_PROPERTY(QDemonImage* diffuseMap2 READ diffuseMap2 WRITE setDiffuseMap2 NOTIFY diffuseMap2Changed)
    Q_PROPERTY(QDemonImage* diffuseMap3 READ diffuseMap3 WRITE setDiffuseMap3 NOTIFY diffuseMap3Changed)

    Q_PROPERTY(float emissivePower READ emissivePower WRITE setEmissivePower NOTIFY emissivePowerChanged)
    Q_PROPERTY(QDemonImage* emissiveMap READ emissiveMap WRITE setEmissiveMap NOTIFY emissiveMapChanged)
    Q_PROPERTY(QColor emissiveColor READ emissiveColor WRITE setEmissiveColor NOTIFY emissiveColorChanged)

    Q_PROPERTY(QDemonImage* specularReflectionMap READ specularReflectionMap WRITE setSpecularReflectionMap NOTIFY specularReflectionMapChanged)
    Q_PROPERTY(QDemonImage* specularMap READ specularMap WRITE setSpecularMap NOTIFY specularMapChanged)
    Q_PROPERTY(DefaultMaterialSpecularModel specularModel READ specularModel WRITE setSpecularModel NOTIFY specularModelChanged)
    Q_PROPERTY(QColor specularTint READ specularTint WRITE setSpecularTint NOTIFY specularTintChanged)

    Q_PROPERTY(float indexOfRefraction READ indexOfRefraction WRITE setIndexOfRefraction NOTIFY indexOfRefractionChanged)
    Q_PROPERTY(float fresnelPower READ fresnelPower WRITE setFresnelPower NOTIFY fresnelPowerChanged)
    Q_PROPERTY(float specularAmount READ specularAmount WRITE setSpecularAmount NOTIFY specularAmountChanged)
    Q_PROPERTY(float specularRoughness READ specularRoughness WRITE setSpecularRoughness NOTIFY specularRoughnessChanged)
    Q_PROPERTY(QDemonImage* roughnessMap READ roughnessMap WRITE setRoughnessMap NOTIFY roughnessMapChanged)

    Q_PROPERTY(float opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QDemonImage* opacityMap READ opacityMap WRITE setOpacityMap NOTIFY opacityMapChanged)

    Q_PROPERTY(QDemonImage* bumpMap READ bumpMap WRITE setBumpMap NOTIFY bumpMapChanged)
    Q_PROPERTY(float bumpAmount READ bumpAmount WRITE setBumpAmount NOTIFY bumpAmountChanged)

    Q_PROPERTY(QDemonImage* normalMap READ normalMap WRITE setNormalMap NOTIFY normalMapChanged)

    Q_PROPERTY(QDemonImage* translucencyMap READ translucencyMap WRITE setTranslucencyMap NOTIFY translucencyMapChanged)
    Q_PROPERTY(float translucentFalloff READ translucentFalloff WRITE setTranslucentFalloff NOTIFY translucentFalloffChanged)

    Q_PROPERTY(float diffuseLightWrap READ diffuseLightWrap WRITE setDiffuseLightWrap NOTIFY diffuseLightWrapChanged)

    Q_PROPERTY(bool vertexColors READ vertexColors WRITE setVertexColors NOTIFY vertexColorsChanged)



public:
    enum DefaultMaterialLighting
    {
        NoLighting = 0,
        VertexLighting,
        FragmentLighting
    };
    Q_ENUM(DefaultMaterialLighting)

    enum DefaultMaterialBlendMode
    {
        Normal = 0,
        Screen,
        Multiply,
        Overlay,
        ColorBurn,
        ColorDodge
    };
    Q_ENUM(DefaultMaterialBlendMode)

    enum DefaultMaterialSpecularModel
    {
        Default = 0,
        KGGX,
        KWard
    };
    Q_ENUM(DefaultMaterialSpecularModel)

    QDemonDefaultMaterial();
    ~QDemonDefaultMaterial() override;


    QDemonObject::Type type() const override;

    DefaultMaterialLighting lighting() const;
    DefaultMaterialBlendMode blendMode() const;
    QColor diffuseColor() const;
    QDemonImage *diffuseMap() const;
    QDemonImage* diffuseMap2() const;
    QDemonImage* diffuseMap3() const;
    float emissivePower() const;
    QDemonImage* emissiveMap() const;
    QColor emissiveColor() const;
    QDemonImage* specularReflectionMap() const;
    QDemonImage* specularMap() const;
    DefaultMaterialSpecularModel specularModel() const;
    QColor specularTint() const;
    float indexOfRefraction() const;
    float fresnelPower() const;
    float specularAmount() const;
    float specularRoughness() const;
    QDemonImage* roughnessMap() const;
    float opacity() const;
    QDemonImage* opacityMap() const;
    QDemonImage* bumpMap() const;
    float bumpAmount() const;
    QDemonImage* normalMap() const;

    QDemonImage* translucencyMap() const;
    float translucentFalloff() const;
    float diffuseLightWrap() const;
    bool vertexColors() const;

public Q_SLOTS:

    void setLighting(DefaultMaterialLighting lighting);
    void setBlendMode(DefaultMaterialBlendMode blendMode);
    void setDiffuseColor(QColor diffuseColor);
    void setDiffuseMap(QDemonImage* diffuseMap);
    void setDiffuseMap2(QDemonImage* diffuseMap2);
    void setDiffuseMap3(QDemonImage* diffuseMap3);
    void setEmissivePower(float emissivePower);
    void setEmissiveMap(QDemonImage* emissiveMap);

    void setEmissiveColor(QColor emissiveColor);
    void setSpecularReflectionMap(QDemonImage* specularReflectionMap);
    void setSpecularMap(QDemonImage* specularMap);
    void setSpecularModel(DefaultMaterialSpecularModel specularModel);
    void setSpecularTint(QColor specularTint);
    void setIndexOfRefraction(float indexOfRefraction);
    void setFresnelPower(float fresnelPower);
    void setSpecularAmount(float specularAmount);
    void setSpecularRoughness(float specularRoughness);
    void setRoughnessMap(QDemonImage* roughnessMap);
    void setOpacity(float opacity);
    void setOpacityMap(QDemonImage* opacityMap);
    void setBumpMap(QDemonImage* bumpMap);
    void setBumpAmount(float bumpAmount);
    void setNormalMap(QDemonImage* normalMap);

    void setTranslucencyMap(QDemonImage* translucencyMap);
    void setTranslucentFalloff(float translucentFalloff);
    void setDiffuseLightWrap(float diffuseLightWrap);
    void setVertexColors(bool vertexColors);

Q_SIGNALS:
    void lightingChanged(DefaultMaterialLighting lighting);
    void blendModeChanged(DefaultMaterialBlendMode blendMode);
    void diffuseColorChanged(QColor diffuseColor);
    void diffuseMapChanged(QDemonImage* diffuseMap);
    void diffuseMap2Changed(QDemonImage* diffuseMap2);
    void diffuseMap3Changed(QDemonImage* diffuseMap3);
    void emissivePowerChanged(float emissivePower);
    void emissiveMapChanged(QDemonImage* emissiveMap);
    void emissiveColorChanged(QColor emissiveColor);
    void specularReflectionMapChanged(QDemonImage* specularReflectionMap);
    void specularMapChanged(QDemonImage* specularMap);
    void specularModelChanged(DefaultMaterialSpecularModel specularModel);
    void specularTintChanged(QColor specularTint);
    void indexOfRefractionChanged(float indexOfRefraction);
    void fresnelPowerChanged(float fresnelPower);
    void specularAmountChanged(float specularAmount);
    void specularRoughnessChanged(float specularRoughness);
    void roughnessMapChanged(QDemonImage* roughnessMap);
    void opacityChanged(float opacity);
    void opacityMapChanged(QDemonImage* opacityMap);
    void bumpMapChanged(QDemonImage* bumpMap);
    void bumpAmountChanged(float bumpAmount);
    void normalMapChanged(QDemonImage* normalMap);
    void translucencyMapChanged(QDemonImage* translucencyMap);
    void translucentFalloffChanged(float translucentFalloff);
    void diffuseLightWrapChanged(float diffuseLightWrap);
    void vertexColorsChanged(bool vertexColors);

protected:
    QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node) override;

private:

    DefaultMaterialLighting m_lighting;
    DefaultMaterialBlendMode m_blendMode;
    QColor m_diffuseColor;
    QDemonImage* m_diffuseMap;
    QDemonImage* m_diffuseMap2;
    QDemonImage* m_diffuseMap3;
    float m_emissivePower;
    QDemonImage* m_emissiveMap;

    QColor m_emissiveColor;
    QDemonImage* m_specularReflectionMap;
    QDemonImage* m_specularMap;
    DefaultMaterialSpecularModel m_specularModel;
    QColor m_specularTint;
    float m_indexOfRefraction;
    float m_fresnelPower;
    float m_specularAmount;
    float m_specularRoughness;
    QDemonImage* m_roughnessMap;
    float m_opacity;
    QDemonImage* m_opacityMap;
    QDemonImage* m_bumpMap;
    float m_bumpAmount;
    QDemonImage* m_normalMap;

    QDemonImage* m_translucencyMap;
    float m_translucentFalloff;
    float m_diffuseLightWrap;
    bool m_vertexColors;
};

QT_END_NAMESPACE

#endif // QDEMONDEFAULTMATERIAL_H
