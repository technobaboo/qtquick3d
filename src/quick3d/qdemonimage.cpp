#include "qdemonimage.h"
#include <QtDemonRuntimeRender/qdemonrenderimage.h>

QT_BEGIN_NAMESPACE

QDemonImage::QDemonImage()
{

}

QDemonImage::~QDemonImage()
{

}

QString QDemonImage::source() const
{
    return m_source;
}

float QDemonImage::scaleU() const
{
    return m_scaleu;
}

float QDemonImage::scaleV() const
{
    return m_scalev;
}

QDemonImage::MappingMode QDemonImage::mappingMode() const
{
    return m_mappingmode;
}

QDemonImage::TilingMode QDemonImage::horizontalTiling() const
{
    return m_tilingmodehorz;
}

QDemonImage::TilingMode QDemonImage::verticalTiling() const
{
    return m_tilingmodevert;
}

float QDemonImage::rotationUV() const
{
    return m_rotationuv;
}

float QDemonImage::positionU() const
{
    return m_positionu;
}

float QDemonImage::positionV() const
{
    return m_positionv;
}

float QDemonImage::pivotU() const
{
    return m_pivotu;
}

float QDemonImage::pivotV() const
{
    return m_pivotv;
}

QDemonObject::Type QDemonImage::type() const
{
    return QDemonObject::Image;
}

void QDemonImage::setSource(QString source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
    update();
}

void QDemonImage::setScaleU(float scaleu)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_scaleu, scaleu))
        return;

    m_scaleu = scaleu;
    emit scaleUChanged(m_scaleu);
    update();
}

void QDemonImage::setScaleV(float scalev)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_scalev, scalev))
        return;

    m_scalev = scalev;
    emit scaleVChanged(m_scalev);
    update();
}

void QDemonImage::setMappingMode(QDemonImage::MappingMode mappingmode)
{
    if (m_mappingmode == mappingmode)
        return;

    m_mappingmode = mappingmode;
    emit mappingModeChanged(m_mappingmode);
    update();
}

void QDemonImage::setHorizontalTiling(QDemonImage::TilingMode tilingmodehorz)
{
    if (m_tilingmodehorz == tilingmodehorz)
        return;

    m_tilingmodehorz = tilingmodehorz;
    emit horizontalTilingChanged(m_tilingmodehorz);
    update();
}

void QDemonImage::setVerticalTiling(QDemonImage::TilingMode tilingmodevert)
{
    if (m_tilingmodevert == tilingmodevert)
        return;

    m_tilingmodevert = tilingmodevert;
    emit verticalTilingChanged(m_tilingmodevert);
    update();
}

void QDemonImage::setRotationUV(float rotationuv)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_rotationuv, rotationuv))
        return;

    m_rotationuv = rotationuv;
    emit rotationUVChanged(m_rotationuv);
    update();
}

void QDemonImage::setPositionU(float positionu)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_positionu, positionu))
        return;

    m_positionu = positionu;
    emit positionUChanged(m_positionu);
    update();
}

void QDemonImage::setPositionV(float positionv)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_positionv, positionv))
        return;

    m_positionv = positionv;
    emit positionVChanged(m_positionv);
    update();
}

void QDemonImage::setPivotU(float pivotu)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_pivotu, pivotu))
        return;

    m_pivotu = pivotu;
    emit piviotUChanged(m_pivotu);
    update();
}

void QDemonImage::setPivotV(float pivotv)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_pivotv, pivotv))
        return;

    m_pivotv = pivotv;
    emit piviotVChanged(m_pivotv);
    update();
}

SGraphObject *QDemonImage::updateSpatialNode(SGraphObject *node)
{
    if (!node)
        node = new SImage();

    auto imageNode = static_cast<SImage*>(node);

//    String imagePath
//    String imageShaderName

//    String offscreenRenderId
//    IOffscreenRenderer* lastFrameOffscreenRenderer
//    SGraphObject* parent

//    SImageTextureData textureData

//    NodeFlags flags

//    Vec2 scale
//    Vec2 pivot
//    Float rotation
//    Vec2 position
//    Enum mappingMode
//    Enum horizontalTilingMode
//    Enum verticalTilingMode

//    mat44 textureTransform
    return imageNode;
}

QT_END_NAMESPACE
