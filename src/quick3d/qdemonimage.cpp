#include "qdemonimage.h"
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtQml/QQmlFile>

#include "qdemonobject_p.h"

QT_BEGIN_NAMESPACE
/*!
    \qmltype DemonImage
    \inqmlmodule QtDemon
    \brief Lets you add a texture image to the material
*/
QDemonImage::QDemonImage() {}

QDemonImage::~QDemonImage() {}

QUrl QDemonImage::source() const
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

void QDemonImage::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
    update();
}

void QDemonImage::setScaleU(float scaleu)
{
    if (qFuzzyCompare(m_scaleu, scaleu))
        return;

    m_scaleu = scaleu;
    emit scaleUChanged(m_scaleu);
    update();
}

void QDemonImage::setScaleV(float scalev)
{
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
    if (qFuzzyCompare(m_rotationuv, rotationuv))
        return;

    m_rotationuv = rotationuv;
    emit rotationUVChanged(m_rotationuv);
    update();
}

void QDemonImage::setPositionU(float positionu)
{
    if (qFuzzyCompare(m_positionu, positionu))
        return;

    m_positionu = positionu;
    emit positionUChanged(m_positionu);
    update();
}

void QDemonImage::setPositionV(float positionv)
{
    if (qFuzzyCompare(m_positionv, positionv))
        return;

    m_positionv = positionv;
    emit positionVChanged(m_positionv);
    update();
}

void QDemonImage::setPivotU(float pivotu)
{
    if (qFuzzyCompare(m_pivotu, pivotu))
        return;

    m_pivotu = pivotu;
    emit piviotUChanged(m_pivotu);
    update();
}

void QDemonImage::setPivotV(float pivotv)
{
    if (qFuzzyCompare(m_pivotv, pivotv))
        return;

    m_pivotv = pivotv;
    emit piviotVChanged(m_pivotv);
    update();
}

QDemonRenderGraphObject *QDemonImage::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderImage();

    auto imageNode = static_cast<QDemonRenderImage *>(node);

    imageNode->m_imagePath = QQmlFile::urlToLocalFileOrQrc(m_source);
    imageNode->m_scale = QVector2D(m_scaleu, m_scalev);
    imageNode->m_pivot = QVector2D(m_pivotu, m_pivotv);
    imageNode->m_rotation = m_rotationuv;
    imageNode->m_position = QVector2D(m_positionu, m_positionv);
    imageNode->m_mappingMode = QDemonRenderImage::MappingModes(m_mappingmode);
    imageNode->m_horizontalTilingMode = QDemonRenderTextureCoordOp(m_tilingmodehorz);
    imageNode->m_verticalTilingMode = QDemonRenderTextureCoordOp(m_tilingmodevert);
    // ### Make this more conditional
    imageNode->m_flags.setFlag(QDemonRenderImage::Flag::Dirty);
    imageNode->m_flags.setFlag(QDemonRenderImage::Flag::TransformDirty);

    return imageNode;
}

QDemonRenderImage *QDemonImage::getRenderImage()
{
    QDemonObjectPrivate *p = QDemonObjectPrivate::get(this);
    return static_cast<QDemonRenderImage *>(p->spatialNode);
}

QT_END_NAMESPACE
