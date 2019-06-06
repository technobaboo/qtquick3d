#ifndef QDEMONSCENEENVIRONMENT_H
#define QDEMONSCENEENVIRONMENT_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QtQuick3d/qdemonnode.h>
#include <QtQuick3d/qdemoneffect.h>
#include <QtQml/QQmlListProperty>

QT_BEGIN_NAMESPACE

class QDemonImage;
class Q_QUICK3D_EXPORT QDemonSceneEnvironment : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QDemonEffect> effects READ effectsList)
    Q_PROPERTY(QDemonEnvironmentAAModeValues progressiveAAMode READ progressiveAAMode WRITE setProgressiveAAMode NOTIFY progressiveAAModeChanged)
    Q_PROPERTY(QDemonEnvironmentAAModeValues multisampleAAMode READ multisampleAAMode WRITE setMultisampleAAMode NOTIFY multisampleAAModeChanged)
    Q_PROPERTY(bool temporalAAEnabled READ temporalAAEnabled WRITE setTemporalAAEnabled NOTIFY temporalAAEnabledChanged)
    Q_PROPERTY(QDemonEnvironmentBackgroundTypes backgroundMode READ backgroundMode WRITE setBackgroundMode NOTIFY backgroundModeChanged)
    Q_PROPERTY(QColor clearColor READ clearColor WRITE setClearColor NOTIFY clearColorChanged)
    Q_PROPERTY(QDemonEnvironmentBlendTypes blendType READ blendType WRITE setBlendType NOTIFY blendTypeChanged)
    Q_PROPERTY(bool isDepthTestDisabled READ isDepthTestDisabled WRITE setIsDepthTestDisabled NOTIFY isDepthTestDisabledChanged)
    Q_PROPERTY(bool isDepthPrePassDisabled READ isDepthPrePassDisabled WRITE setIsDepthPrePassDisabled NOTIFY isDepthPrePassDisabledChanged)

    Q_PROPERTY(float aoStrength READ aoStrength WRITE setAoStrength NOTIFY aoStrengthChanged)
    Q_PROPERTY(float aoDistance READ aoDistance WRITE setAoDistance NOTIFY aoDistanceChanged)
    Q_PROPERTY(float aoSoftness READ aoSoftness WRITE setAoSoftness NOTIFY aoSoftnessChanged)
    Q_PROPERTY(bool aoDither READ aoDither WRITE setAoDither NOTIFY aoDitherChanged)
    Q_PROPERTY(int aoSampleRate READ aoSampleRate WRITE setAoSampleRate NOTIFY aoSampleRateChanged)
    Q_PROPERTY(float aoBias READ aoBias WRITE setAoBias NOTIFY aoBiasChanged)

    Q_PROPERTY(float shadowStrength READ shadowStrength WRITE setShadowStrength NOTIFY shadowStrengthChanged)
    Q_PROPERTY(float shadowDistance READ shadowDistance WRITE setShadowDistance NOTIFY shadowDistanceChanged)
    Q_PROPERTY(float shadowSoftness READ shadowSoftness WRITE setShadowSoftness NOTIFY shadowSoftnessChanged)
    Q_PROPERTY(float shadowBias READ shadowBias WRITE setShadowBias NOTIFY shadowBiasChanged)

    Q_PROPERTY(QDemonImage *lightProbe READ lightProbe WRITE setLightProbe NOTIFY lightProbeChanged)
    Q_PROPERTY(float probeBrightness READ probeBrightness WRITE setProbeBrightness NOTIFY probeBrightnessChanged)
    Q_PROPERTY(bool fastIBL READ fastIBL WRITE setFastIBL NOTIFY fastIBLChanged)
    Q_PROPERTY(float probeHorizon READ probeHorizon WRITE setProbeHorizon NOTIFY probeHorizonChanged)
    Q_PROPERTY(float probeFieldOfView READ probeFieldOfView WRITE setProbeFieldOfView NOTIFY probeFieldOfViewChanged)

    Q_PROPERTY(QDemonImage *lightProbe2 READ lightProbe2 WRITE setLightProbe2 NOTIFY lightProbe2Changed)
    Q_PROPERTY(float probe2Fade READ probe2Fade WRITE setProbe2Fade NOTIFY probe2FadeChanged)
    Q_PROPERTY(float probe2Window READ probe2Window WRITE setProbe2Window NOTIFY probe2WindowChanged)
    Q_PROPERTY(float probe2Postion READ probe2Postion WRITE setProbe2Postion NOTIFY probe2PostionChanged)

public:
    enum QDemonEnvironmentAAModeValues {
        NoAA = 0,
        SSAA = 1,
        X2 = 2,
        X4 = 4,
        X8 = 8
    };
    Q_ENUM(QDemonEnvironmentAAModeValues)
    enum QDemonEnvironmentBackgroundTypes {
        Transparent = 0,
        Unspecified,
        Color,
        SkyBox
    };
    Q_ENUM(QDemonEnvironmentBackgroundTypes)

    enum QDemonEnvironmentBlendTypes {
        Normal = 0,
        Screen,
        Multiply,
        Add,
        Subtract,
        Overlay,
        ColorBurn,
        ColorDodge
    };
    Q_ENUM(QDemonEnvironmentBlendTypes)


    explicit QDemonSceneEnvironment(QDemonObject *parent = nullptr);
    ~QDemonSceneEnvironment() override;

    QDemonEnvironmentAAModeValues progressiveAAMode() const;
    QDemonEnvironmentAAModeValues multisampleAAMode() const;
    bool temporalAAEnabled() const;

    QDemonEnvironmentBackgroundTypes backgroundMode() const;
    QColor clearColor() const;
    QDemonEnvironmentBlendTypes blendType() const;

    float aoStrength() const;
    float aoDistance() const;
    float aoSoftness() const;
    bool aoDither() const;
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

    QQmlListProperty<QDemonEffect> effectsList();

    bool isDepthTestDisabled() const;
    bool isDepthPrePassDisabled() const;

    QDemonObject::Type type() const override;

public Q_SLOTS:
    void setProgressiveAAMode(QDemonEnvironmentAAModeValues progressiveAAMode);
    void setMultisampleAAMode(QDemonEnvironmentAAModeValues multisampleAAMode);
    void setTemporalAAEnabled(bool temporalAAEnabled);

    void setBackgroundMode(QDemonEnvironmentBackgroundTypes backgroundMode);
    void setClearColor(QColor clearColor);
    void setBlendType(QDemonEnvironmentBlendTypes blendType);

    void setAoStrength(float aoStrength);
    void setAoDistance(float aoDistance);
    void setAoSoftness(float aoSoftness);
    void setAoDither(bool aoDither);
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

    void setIsDepthTestDisabled(bool isDepthTestDisabled);
    void setIsDepthPrePassDisabled(bool isDepthPrePassDisabled);

Q_SIGNALS:
    void progressiveAAModeChanged(QDemonEnvironmentAAModeValues progressiveAAMode);
    void multisampleAAModeChanged(QDemonEnvironmentAAModeValues multisampleAAMode);
    void temporalAAEnabledChanged(bool temporalAAEnabled);

    void backgroundModeChanged(QDemonEnvironmentBackgroundTypes backgroundMode);
    void clearColorChanged(QColor clearColor);
    void blendTypeChanged(QDemonEnvironmentBlendTypes blendType);

    void aoStrengthChanged(float aoStrength);
    void aoDistanceChanged(float aoDistance);
    void aoSoftnessChanged(float aoSoftness);
    void aoDitherChanged(bool aoDither);
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

    void isDepthTestDisabledChanged(bool isDepthTestDisabled);
    void isDepthPrePassDisabledChanged(bool isDepthPrePassDisabled);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;
    void itemChange(ItemChange, const ItemChangeData &) override;

private:
    void updateSceneManager(QDemonSceneManager *manager);

    QDemonEnvironmentAAModeValues m_progressiveAAMode = NoAA;
    QDemonEnvironmentAAModeValues m_multisampleAAMode = NoAA;
    bool m_temporalAAEnabled = false;

    QDemonEnvironmentBackgroundTypes m_backgroundMode = Transparent;
    QColor m_clearColor = Qt::black;
    QDemonEnvironmentBlendTypes m_blendType = Normal;

    float m_aoStrength = 0.0f;
    float m_aoDistance = 5.0f;
    float m_aoSoftness = 50.0f;
    bool m_aoDither = false;
    int m_aoSampleRate = 2;
    float m_aoBias = 0.0f;
    float m_shadowStrength = 0.0f;
    float m_shadowDistance = 10.0f;
    float m_shadowSoftness = 100.0f;
    float m_shadowBias = 0.0f;
    QDemonImage *m_lightProbe = nullptr;
    float m_probeBrightness = 100.0f;
    bool m_fastIBL = false;
    float m_probeHorizon = -1.0f;
    float m_probeFieldOfView = 180.0f;
    QDemonImage *m_lightProbe2 = nullptr;
    float m_probe2Fade = 1.0f;
    float m_probe2Window = 1.0f;
    float m_probe2Postion = 0.5f;

    QVector<QDemonEffect *> m_effects;

    static void qmlAppendEffect(QQmlListProperty<QDemonEffect> *list, QDemonEffect *effect);
    static QDemonEffect *qmlEffectAt(QQmlListProperty<QDemonEffect> *list, int index);
    static int qmlEffectsCount(QQmlListProperty<QDemonEffect> *list);
    static void qmlClearEffects(QQmlListProperty<QDemonEffect> *list);

    QHash<QObject*, QMetaObject::Connection> m_connections;
    bool m_isDepthTestDisabled = false;
    bool m_isDepthPrePassDisabled = false;
};

QT_END_NAMESPACE

#endif // QDEMONSCENEENVIRONMENT_H
