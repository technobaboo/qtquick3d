#ifndef QDEMONIMAGE_H
#define QDEMONIMAGE_H

#include <qdemonobject.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QDemonImage : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float scaleu READ scaleU WRITE setScaleU NOTIFY scaleUChanged)
    Q_PROPERTY(float scalev READ scaleV WRITE setScaleV NOTIFY scaleVChanged)
    Q_PROPERTY(MappingMode mappingmode READ mappingMode WRITE setMappingMode NOTIFY mappingModeChanged)
    Q_PROPERTY(TilingMode tilingmodehorz READ horizontalTiling WRITE setHorizontalTiling NOTIFY horizontalTilingChanged)
    Q_PROPERTY(TilingMode tilingmodevert READ verticalTiling WRITE setVerticalTiling NOTIFY verticalTilingChanged)
    Q_PROPERTY(float rotationuv READ rotationUV WRITE setRotationUV NOTIFY rotationUVChanged)
    Q_PROPERTY(float positionu READ positionU WRITE setPositionU NOTIFY positionUChanged)
    Q_PROPERTY(float positionv READ positionV WRITE setPositionV NOTIFY positionVChanged)
    Q_PROPERTY(float pivotu READ pivotU WRITE setPivotU NOTIFY piviotUChanged)
    Q_PROPERTY(float pivotv READ pivotV WRITE setPivotV NOTIFY piviotVChanged)


public:
    enum MappingMode {
        UVMapping = 0,
        EnvironmentalMapping,
        LightProbe,
        IBLOverride
    };
    Q_ENUM(MappingMode)

    enum TilingMode {
        Tiled = 0,
        Mirrored,
        NoTiling
    };
    Q_ENUM(TilingMode)

    QDemonImage();
    ~QDemonImage() override;

    QString source() const;
    float scaleU() const;
    float scaleV() const;
    MappingMode mappingMode() const;
    TilingMode horizontalTiling() const;
    TilingMode verticalTiling() const;
    float rotationUV() const;
    float positionU() const;
    float positionV() const;
    float pivotU() const;
    float pivotV() const;
    QDemonObject::Type type() const override;

public Q_SLOTS:
    void setSource(QString source);
    void setScaleU(float scaleu);
    void setScaleV(float scalev);
    void setMappingMode(MappingMode mappingmode);
    void setHorizontalTiling(TilingMode tilingmodehorz);
    void setVerticalTiling(TilingMode tilingmodevert);
    void setRotationUV(float rotationuv);
    void setPositionU(float positionu);
    void setPositionV(float positionv);
    void setPivotU(float pivotu);
    void setPivotV(float pivotv);

Q_SIGNALS:
    void sourceChanged(QString source);
    void scaleUChanged(float scaleu);
    void scaleVChanged(float scalev);
    void mappingModeChanged(MappingMode mappingmode);
    void horizontalTilingChanged(TilingMode tilingmodehorz);
    void verticalTilingChanged(TilingMode tilingmodevert);
    void rotationUVChanged(float rotationuv);
    void positionUChanged(float positionu);
    void positionVChanged(float positionv);
    void piviotUChanged(float pivotu);
    void piviotVChanged(float pivotv);

protected:
    SGraphObject *updateSpatialNode(SGraphObject *node) override;

private:
    QString m_source;
    float m_scaleu;
    float m_scalev;
    MappingMode m_mappingmode;
    TilingMode m_tilingmodehorz;
    TilingMode m_tilingmodevert;
    float m_rotationuv;
    float m_positionu;
    float m_positionv;
    float m_pivotu;
    float m_pivotv;

};

QT_END_NAMESPACE

#endif // QDEMONIMAGE_H
