#ifndef QDEMONLIGHT_H
#define QDEMONLIGHT_H

#include <QtQuick3d/qdemonnode.h>
#include <QColor>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonLight : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(QDemonRenderLightTypes lightType READ lightType WRITE setLightType NOTIFY lightTypeChanged)
    Q_PROPERTY(QColor diffuseColor READ diffuseColor WRITE setDiffuseColor NOTIFY diffuseColorChanged)
    Q_PROPERTY(QColor specularColor READ specularColor WRITE setSpecularColor NOTIFY specularColorChanged)
    Q_PROPERTY(QColor ambientColor READ ambientColor WRITE setAmbientColor NOTIFY ambientColorChanged)
    Q_PROPERTY(float brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(float linearFade READ linearFade WRITE setLinearFade NOTIFY linearFadeChanged)
    Q_PROPERTY(float exponentialFade READ exponentialFade WRITE setExponentialFade NOTIFY exponentialFadeChanged)
    Q_PROPERTY(float areaWidth READ areaWidth WRITE setAreaWidth NOTIFY areaWidthChanged)
    Q_PROPERTY(float areaHeight READ areaHeight WRITE setAreaHeight NOTIFY areaHeightChanged)
    Q_PROPERTY(bool castShadow READ castShadow WRITE setCastShadow NOTIFY castShadowChanged)
    Q_PROPERTY(float shadowBias READ shadowBias WRITE setShadowBias NOTIFY shadowBiasChanged)
    Q_PROPERTY(float shadowFactor READ shadowFactor WRITE setShadowFactor NOTIFY shadowFactorChanged)
    Q_PROPERTY(int shadowMapResolution READ shadowMapResolution WRITE setShadowMapResolution NOTIFY shadowMapResolutionChanged)
    Q_PROPERTY(float shadowMapFar READ shadowMapFar WRITE setShadowMapFar NOTIFY shadowMapFarChanged)
    Q_PROPERTY(float shadowMapFieldOfView READ shadowMapFieldOfView WRITE setShadowMapFieldOfView NOTIFY shadowMapFieldOfViewChanged)
    Q_PROPERTY(float shadowFilter READ shadowFilter WRITE setShadowFilter NOTIFY shadowFilterChanged)
    Q_PROPERTY(QDemonNode *scope READ scope WRITE setScope NOTIFY scopeChanged)

public:
    enum QDemonRenderLightTypes {
        Unknown = 0,
        Directional,
        Point,
        Area,
    };
    Q_ENUM(QDemonRenderLightTypes)

    QDemonLight();
    ~QDemonLight() override;

    QDemonObject::Type type() const override;
    QDemonRenderLightTypes lightType() const;
    QColor diffuseColor() const;
    QColor specularColor() const;
    QColor ambientColor() const;
    float brightness() const;
    float linearFade() const;
    float exponentialFade() const;
    float areaWidth() const;
    float areaHeight() const;
    bool castShadow() const;
    float shadowBias() const;
    float shadowFactor() const;
    int shadowMapResolution() const;
    float shadowMapFar() const;
    float shadowMapFieldOfView() const;
    float shadowFilter() const;
    QDemonNode *scope() const;

public Q_SLOTS:
    void setLightType(QDemonRenderLightTypes lightType);
    void setDiffuseColor(QColor diffuseColor);
    void setSpecularColor(QColor specularColor);
    void setAmbientColor(QColor ambientColor);
    void setBrightness(float brightness);
    void setLinearFade(float linearFade);
    void setExponentialFade(float exponentialFade);
    void setAreaWidth(float areaWidth);
    void setAreaHeight(float areaHeight);
    void setCastShadow(bool castShadow);
    void setShadowBias(float shadowBias);
    void setShadowFactor(float shadowFactor);
    void setShadowMapResolution(int shadowMapResolution);
    void setShadowMapFar(float shadowMapFar);
    void setShadowMapFieldOfView(float shadowMapFieldOfView);
    void setShadowFilter(float shadowFilter);
    void setScope(QDemonNode * scope);

Q_SIGNALS:
    void lightTypeChanged(QDemonRenderLightTypes lightType);
    void diffuseColorChanged(QColor diffuseColor);
    void specularColorChanged(QColor specularColor);
    void ambientColorChanged(QColor ambientColor);
    void brightnessChanged(float brightness);
    void linearFadeChanged(float linearFade);
    void exponentialFadeChanged(float exponentialFade);
    void areaWidthChanged(float areaWidth);
    void areaHeightChanged(float areaHeight);
    void castShadowChanged(bool castShadow);
    void shadowBiasChanged(float shadowBias);
    void shadowFactorChanged(float shadowFactor);
    void shadowMapResolutionChanged(int shadowMapResolution);
    void shadowMapFarChanged(float shadowMapFar);
    void shadowMapFieldOfViewChanged(float shadowMapFieldOfView);
    void shadowFilterChanged(float shadowFilter);
    void scopeChanged(QDemonNode *scope);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    QDemonRenderLightTypes m_lightType = Directional;
    QColor m_diffuseColor;
    QColor m_specularColor;
    QColor m_ambientColor;
    float m_brightness = 100.0f;
    float m_linearFade = 0.0f;
    float m_exponentialFade = 0.0f;
    float m_areaWidth = 100.0f;
    float m_areaHeight = 100.0f;
    bool m_castShadow = false;
    float m_shadowBias = 0.0f;
    float m_shadowFactor = 5.0f;
    int m_shadowMapResolution = 9;
    float m_shadowMapFar = 5000.0f;
    float m_shadowMapFieldOfView = 90.0f;
    float m_shadowFilter = 35.0f;
    QDemonNode *m_scope = nullptr;
};

QT_END_NAMESPACE
#endif // QDEMONLIGHT_H
