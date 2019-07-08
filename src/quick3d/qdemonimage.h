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
    Q_PROPERTY(Format format READ format WRITE setFormat NOTIFY formatChanged)

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

    enum Format {
        Automatic = 0,
        R8,
        R16,
        R16F,
        R32I,
        R32UI,
        R32F,
        RG8,
        RGBA8,
        RGB8,
        SRGB8,
        SRGB8A8,
        RGB565,
        RGBA5551,
        Alpha8,
        Luminance8,
        Luminance16,
        LuminanceAlpha8,
        RGBA16F,
        RG16F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11G11B10,
        RGB9E5,
        RGBA_DXT1,
        RGB_DXT1,
        RGBA_DXT3,
        RGBA_DXT5,
        Depth16,
        Depth24,
        Depth32,
        Depth24Stencil8
    };
    Q_ENUM(Format)

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

    Format format() const;

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setScaleU(float scaleU);
    void setScaleV(float scaleV);
    void setMappingMode(MappingMode mappingMode);
    void setHorizontalTiling(TilingMode tilingModeHorizontal);
    void setVerticalTiling(TilingMode tilingModeVertical);
    void setRotationUV(float rotationUV);
    void setPositionU(float positionU);
    void setPositionV(float positionV);
    void setPivotU(float pivotU);
    void setPivotV(float pivotV);
    void setFormat(Format format);

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
    void formatChanged(Format format);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    QUrl m_source;
    float m_scaleU = 1.0f;
    float m_scaleV = 1.0f;
    MappingMode m_mappingMode = Normal;
    TilingMode m_tilingModeHorizontal = ClampToEdge;
    TilingMode m_tilingModeVertical = ClampToEdge;
    float m_rotationUV = 0;
    float m_positionU = 0;
    float m_positionV = 0;
    float m_pivotU = 0;
    float m_pivotV = 0;
    Format m_format = Automatic;
};

QT_END_NAMESPACE

#endif // QDEMONIMAGE_H
