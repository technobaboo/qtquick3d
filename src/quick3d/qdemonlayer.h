#ifndef QDEMONLAYER_H
#define QDEMONLAYER_H

#include <QtQuick3d/qdemonnode.h>
#include <QtQuick3d/qdemoneffect.h>
#include <QtQuick3d/qdemoncamera.h>
#include <QtGui/QColor>
#include <QtQml/QQmlListProperty>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QDemonImage;
class Q_QUICK3D_EXPORT QDemonLayer : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(QString texturePath READ texturePath WRITE setTexturePath NOTIFY texturePathChanged)
    Q_PROPERTY(QQmlListProperty<QDemonEffect> effects READ effectsList)
    Q_PROPERTY(AAModeValues progressiveAAMode READ progressiveAAMode WRITE setProgressiveAAMode NOTIFY progressiveAAModeChanged)
    Q_PROPERTY(AAModeValues multisampleAAMode READ multisampleAAMode WRITE setMultisampleAAMode NOTIFY multisampleAAModeChanged)
    Q_PROPERTY(LayerBackground backgroundMode READ backgroundMode WRITE setBackgroundMode NOTIFY backgroundModeChanged)
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
    Q_PROPERTY(LayerBlendTypes blendType READ blendType WRITE setBlendType NOTIFY blendTypeChanged)
    Q_PROPERTY(HorizontalFieldValues horizontalFieldValue READ horizontalFieldValue WRITE setHorizontalFieldValue NOTIFY horizontalFieldValueChanged)
    Q_PROPERTY(VerticalFieldValues verticalFieldValue READ verticalFieldValue WRITE setVerticalFieldValue NOTIFY verticalFieldValueChanged)
    Q_PROPERTY(LayerUnitTypes leftUnits READ leftUnits WRITE setLeftUnits NOTIFY leftUnitsChanged)
    Q_PROPERTY(LayerUnitTypes rightUnits READ rightUnits WRITE setRightUnits NOTIFY rightUnitsChanged)
    Q_PROPERTY(LayerUnitTypes topUnits READ topUnits WRITE setTopUnits NOTIFY topUnitsChanged)
    Q_PROPERTY(LayerUnitTypes bottomUnits READ bottomUnits WRITE setBottomUnits NOTIFY bottomUnitsChanged)
    Q_PROPERTY(float left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(float right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(float width READ width WRITE setWidth NOTIFY widthChanged)

    Q_PROPERTY(float aoStrength READ aoStrength WRITE setAoStrength NOTIFY aoStrengthChanged)
    Q_PROPERTY(float aoDistance READ aoDistance WRITE setAoDistance NOTIFY aoDistanceChanged)
    Q_PROPERTY(float aoSoftness READ aoSoftness WRITE setAoSoftness NOTIFY aoSoftnessChanged)
    Q_PROPERTY(float aoDither READ aoDither WRITE setAoDither NOTIFY aoDitherChanged)
    Q_PROPERTY(int aoSampleRate READ aoSampleRate WRITE setAoSampleRate NOTIFY aoSampleRateChanged)
    Q_PROPERTY(float aoBias READ aoBias WRITE setAoBias NOTIFY aoBiasChanged)

    Q_PROPERTY(float shadowStrength READ shadowStrength WRITE setShadowStrength NOTIFY shadowStrengthChanged)
    Q_PROPERTY(float shadowDistance READ shadowDistance WRITE setShadowDistance NOTIFY shadowDistanceChanged)
    Q_PROPERTY(float shadowSoftness READ shadowSoftness WRITE setShadowSoftness NOTIFY shadowSoftnessChanged)
    Q_PROPERTY(float shadowBias READ shadowBias WRITE setShadowBias NOTIFY shadowBiasChanged)

    Q_PROPERTY(QDemonImage* lightProbe READ lightProbe WRITE setLightProbe NOTIFY lightProbeChanged)
    Q_PROPERTY(float probeBrightness READ probeBrightness WRITE setProbeBrightness NOTIFY probeBrightnessChanged)
    Q_PROPERTY(bool fastIBL READ fastIBL WRITE setFastIBL NOTIFY fastIBLChanged)
    Q_PROPERTY(float probeHorizon READ probeHorizon WRITE setProbeHorizon NOTIFY probeHorizonChanged)
    Q_PROPERTY(float probeFieldOfView READ probeFieldOfView WRITE setProbeFieldOfView NOTIFY probeFieldOfViewChanged)

    Q_PROPERTY(QDemonImage* lightProbe2 READ lightProbe2 WRITE setLightProbe2 NOTIFY lightProbe2Changed)
    Q_PROPERTY(float probe2Fade READ probe2Fade WRITE setProbe2Fade NOTIFY probe2FadeChanged)
    Q_PROPERTY(float probe2Window READ probe2Window WRITE setProbe2Window NOTIFY probe2WindowChanged)
    Q_PROPERTY(float probe2Postion READ probe2Postion WRITE setProbe2Postion NOTIFY probe2PostionChanged)

    Q_PROPERTY(bool temporalAAEnabled READ temporalAAEnabled WRITE setTemporalAAEnabled NOTIFY temporalAAEnabledChanged)

    Q_PROPERTY(QDemonCamera* activeCamera READ activeCamera WRITE setActiveCamera NOTIFY activeCameraChanged)

public:
    enum AAModeValues
    {
        NoAA = 0,
        SSAA = 1,
        X2 = 2,
        X4 = 4,
        X8 = 8
    };
    Q_ENUM(AAModeValues)

    enum HorizontalFieldValues
    {
        LeftWidth = 0,
        LeftRight,
        WidthRight
    };
    Q_ENUM(HorizontalFieldValues)

    enum VerticalFieldValues
    {
        TopHeight = 0,
        TopBottom,
        HeightBottom
    };
    Q_ENUM(VerticalFieldValues)

    enum LayerUnitTypes
    {
        Percent = 0,
        Pixels
    };
    Q_ENUM(LayerUnitTypes)

    enum LayerBackground
    {
        Transparent = 0,
        Unspecified,
        Color
    };
    Q_ENUM(LayerBackground)

    enum LayerBlendTypes
    {
        Normal = 0,
        Screen,
        Multiply,
        Add,
        Subtract,
        Overlay,
        ColorBurn,
        ColorDodge
    };
    Q_ENUM(LayerBlendTypes)

    QDemonLayer();
    ~QDemonLayer() override;

    QDemonObject::Type type() const override;

    QString texturePath() const;
    AAModeValues progressiveAAMode() const;
    AAModeValues multisampleAAMode() const;
    LayerBackground backgroundMode() const;
    QColor clearColor() const;
    LayerBlendTypes blendType() const;
    HorizontalFieldValues horizontalFieldValue() const;
    VerticalFieldValues verticalFieldValue() const;
    LayerUnitTypes leftUnits() const;
    LayerUnitTypes rightUnits() const;
    LayerUnitTypes topUnits() const;
    LayerUnitTypes bottomUnits() const;
    float left() const;
    float right() const;
    float top() const;
    float bottom() const;
    float height() const;
    float width() const;
    float aoStrength() const;
    float aoDistance() const;
    float aoSoftness() const;
    float aoDither() const;
    int aoSampleRate() const;
    float aoBias() const;
    float shadowStrength() const;
    float shadowDistance() const;
    float shadowSoftness() const;
    float shadowBias() const;

    QDemonImage *lightProbe() const;
    float probeBrightness() const;
    bool fastIBL() const;
    float probeHorizon() const;
    float probeFieldOfView() const;
    QDemonImage *lightProbe2() const;
    float probe2Fade() const;
    float probe2Window() const;
    float probe2Postion() const;

    bool temporalAAEnabled() const;

    QQmlListProperty<QDemonEffect> effectsList();

    QDemonCamera *activeCamera() const;

public Q_SLOTS:
    void setTexturePath(QString texturePath);
    void setProgressiveAAMode(AAModeValues progressiveAAMode);
    void setMultisampleAAMode(AAModeValues multisampleAAMode);
    void setBackgroundMode(LayerBackground backgroundMode);
    void setClearColor(QColor clearColor);
    void setBlendType(LayerBlendTypes blendType);
    void setHorizontalFieldValue(HorizontalFieldValues horizontalFieldValue);
    void setVerticalFieldValue(VerticalFieldValues verticalFieldValue);
    void setLeftUnits(LayerUnitTypes leftUnits);
    void setRightUnits(LayerUnitTypes rightUnits);
    void setTopUnits(LayerUnitTypes topUnits);
    void setBottomUnits(LayerUnitTypes bottomUnits);
    void setLeft(float left);
    void setRight(float right);
    void setTop(float top);
    void setBottom(float bottom);
    void setHeight(float height);
    void setWidth(float width);
    void setAoStrength(float aoStrength);
    void setAoDistance(float aoDistance);
    void setAoSoftness(float aoSoftness);
    void setAoDither(float aoDither);
    void setAoSampleRate(int aoSampleRate);
    void setAoBias(float aoBias);
    void setShadowStrength(float shadowStrength);
    void setShadowDistance(float shadowDistance);
    void setShadowSoftness(float shadowSoftness);
    void setShadowBias(float shadowBias);
    void setLightProbe(QDemonImage *lightProbe);
    void setProbeBrightness(float probeBrightness);
    void setFastIBL(bool fastIBL);
    void setProbeHorizon(float probeHorizon);
    void setProbeFieldOfView(float probeFieldOfView);
    void setLightProbe2(QDemonImage *lightProbe2);
    void setProbe2Fade(float probe2Fade);
    void setProbe2Window(float probe2Window);
    void setProbe2Postion(float probe2Postion);
    void setTemporalAAEnabled(bool temporalAAEnabled);
    void setActiveCamera(QDemonCamera *camera);

Q_SIGNALS:
    void texturePathChanged(QString texturePath);
    void progressiveAAModeChanged(AAModeValues progressiveAAMode);
    void multisampleAAModeChanged(AAModeValues multisampleAAMode);
    void backgroundModeChanged(LayerBackground backgroundMode);
    void clearColorChanged(QColor clearColor);
    void blendTypeChanged(LayerBlendTypes blendType);
    void horizontalFieldValueChanged(HorizontalFieldValues horizontalFieldValue);
    void verticalFieldValueChanged(VerticalFieldValues verticalFieldValue);
    void leftUnitsChanged(LayerUnitTypes leftUnits);
    void rightUnitsChanged(LayerUnitTypes rightUnits);
    void topUnitsChanged(LayerUnitTypes topUnits);
    void bottomUnitsChanged(LayerUnitTypes bottomUnits);
    void leftChanged(float left);
    void rightChanged(float right);
    void topChanged(float top);
    void bottomChanged(float bottom);
    void heightChanged(float height);
    void widthChanged(float width);
    void aoStrengthChanged(float aoStrength);
    void aoDistanceChanged(float aoDistance);
    void aoSoftnessChanged(float aoSoftness);
    void aoDitherChanged(float aoDither);
    void aoSampleRateChanged(int aoSampleRate);
    void aoBiasChanged(float aoBias);
    void shadowStrengthChanged(float shadowStrength);
    void shadowDistanceChanged(float shadowDistance);
    void shadowSoftnessChanged(float shadowSoftness);
    void shadowBiasChanged(float shadowBias);
    void lightProbeChanged(QDemonImage *lightProbe);
    void probeBrightnessChanged(float probeBrightness);
    void fastIBLChanged(bool fastIBL);
    void probeHorizonChanged(float probeHorizon);
    void probeFieldOfViewChanged(float probeFieldOfView);
    void lightProbe2Changed(QDemonImage *lightProbe2);
    void probe2FadeChanged(float probe2Fade);
    void probe2WindowChanged(float probe2Window);
    void probe2PostionChanged(float probe2Postion);
    void temporalAAEnabledChanged(bool temporalAAEnabled);
    void activeCameraChanged(QDemonCamera *camera);

protected:
    QDemonGraphObject *updateSpatialNode(QDemonGraphObject *node) override;
private:

    QString m_texturePath;
    AAModeValues m_progressiveAAMode;
    AAModeValues m_multisampleAAMode;
    LayerBackground m_backgroundMode;
    QColor m_clearColor;
    LayerBackground m_setBackgroundMode;
    LayerBlendTypes m_blendType;
    HorizontalFieldValues m_horizontalFieldValue;
    VerticalFieldValues m_verticalFieldValue;
    LayerUnitTypes m_leftUnits;
    LayerUnitTypes m_rightUnits;
    LayerUnitTypes m_topUnits;
    LayerUnitTypes m_bottomUnits;
    float m_left;
    float m_right;
    float m_top;
    float m_bottom;
    float m_height;
    float m_width;
    float m_aoStrength;
    float m_aoDistance;
    float m_aoSoftness;
    float m_aoDither;
    int m_aoSampleRate;
    float m_aoBias;
    float m_shadowStrength;
    float m_shadowDistance;
    float m_shadowSoftness;
    float m_shadowBias;
    QDemonImage *m_lightProbe;
    float m_probeBrightness;
    bool m_fastIBL;
    float m_probeHorizon;
    float m_probeFieldOfView;
    QDemonImage *m_lightProbe2;
    float m_probe2Fade;
    float m_probe2Window;
    float m_probe2Postion;
    bool m_temporalAAEnabled;

    static void qmlAppendEffect(QQmlListProperty<QDemonEffect> *list, QDemonEffect *effect);
    static QDemonEffect *qmlEffectAt(QQmlListProperty<QDemonEffect> *list, int index);
    static int qmlEffectsCount(QQmlListProperty<QDemonEffect> *list);
    static void qmlClearEffects(QQmlListProperty<QDemonEffect> *list);

    QVector<QDemonEffect *> m_effects;
    QDemonCamera *m_activeCamera;
    friend QDemonWindowPrivate;
};

QT_END_NAMESPACE

#endif // QDEMONLAYER_H
