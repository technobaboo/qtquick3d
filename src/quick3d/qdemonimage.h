#ifndef QDEMONIMAGE_H
#define QDEMONIMAGE_H

#include <QtQuick3d/qdemonobject.h>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

struct QDemonRenderImage;
class Q_QUICK3D_EXPORT QDemonImage : public QDemonObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float scaleU READ scaleU WRITE setScaleU NOTIFY scaleUChanged)
    Q_PROPERTY(float scaleV READ scaleV WRITE setScaleV NOTIFY scaleVChanged)
    Q_PROPERTY(MappingMode mappingMode READ mappingMode WRITE setMappingMode NOTIFY mappingModeChanged)
    Q_PROPERTY(TilingMode tilingModeHorizontal READ horizontalTiling WRITE setHorizontalTiling NOTIFY horizontalTilingChanged)
    Q_PROPERTY(TilingMode tilingModeVertical READ verticalTiling WRITE setVerticalTiling NOTIFY verticalTilingChanged)
    Q_PROPERTY(float rotationUV READ rotationUV WRITE setRotationUV NOTIFY rotationUVChanged)
    Q_PROPERTY(float positionU READ positionU WRITE setPositionU NOTIFY positionUChanged)
    Q_PROPERTY(float positionV READ positionV WRITE setPositionV NOTIFY positionVChanged)
    Q_PROPERTY(float pivotU READ pivotU WRITE setPivotU NOTIFY pivotUChanged)
    Q_PROPERTY(float pivotV READ pivotV WRITE setPivotV NOTIFY pivotVChanged)

public:
    enum MappingMode
    {
        Normal = 0, // UV mapping
        Environment = 1,
        LightProbe = 2,
    };
    Q_ENUM(MappingMode)

    enum TilingMode
    {
        Unknown = 0,
        ClampToEdge,
        MirroredRepeat,
        Repeat
    };
    Q_ENUM(TilingMode)

    QDemonImage();
    ~QDemonImage() override;

    QUrl source() const;
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

    QDemonRenderImage *getRenderImage();

public Q_SLOTS:
    void setSource(const QUrl &source);
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
    void sourceChanged(const QUrl &source);
    void scaleUChanged(float scaleU);
    void scaleVChanged(float scaleV);
    void mappingModeChanged(MappingMode mappingMode);
    void horizontalTilingChanged(TilingMode tilingModeHorizontal);
    void verticalTilingChanged(TilingMode tilingModeVertical);
    void rotationUVChanged(float rotationUV);
    void positionUChanged(float positionU);
    void positionVChanged(float positionV);
    void pivotUChanged(float pivotU);
    void pivotVChanged(float pivotV);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    QUrl m_source;
    float m_scaleu = 1.0f;
    float m_scalev = 1.0f;
    MappingMode m_mappingmode = Normal;
    TilingMode m_tilingmodehorz = ClampToEdge;
    TilingMode m_tilingmodevert = ClampToEdge;
    float m_rotationuv = 0;
    float m_positionu = 0;
    float m_positionv = 0;
    float m_pivotu = 0;
    float m_pivotv = 0;
};

QT_END_NAMESPACE

#endif // QDEMONIMAGE_H
