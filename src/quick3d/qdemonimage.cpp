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
    return m_scaleU;
}

float QDemonImage::scaleV() const
{
    return m_scaleV;
}

QDemonImage::MappingMode QDemonImage::mappingMode() const
{
    return m_mappingMode;
}

QDemonImage::TilingMode QDemonImage::horizontalTiling() const
{
    return m_tilingModeHorizontal;
}

QDemonImage::TilingMode QDemonImage::verticalTiling() const
{
    return m_tilingModeVertical;
}

float QDemonImage::rotationUV() const
{
    return m_rotationUV;
}

float QDemonImage::positionU() const
{
    return m_positionU;
}

float QDemonImage::positionV() const
{
    return m_positionV;
}

float QDemonImage::pivotU() const
{
    return m_pivotU;
}

float QDemonImage::pivotV() const
{
    return m_pivotV;
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

void QDemonImage::setScaleU(float scaleU)
{
    if (qFuzzyCompare(m_scaleU, scaleU))
        return;

    m_scaleU = scaleU;
    emit scaleUChanged(m_scaleU);
    update();
}

void QDemonImage::setScaleV(float scaleV)
{
    if (qFuzzyCompare(m_scaleV, scaleV))
        return;

    m_scaleV = scaleV;
    emit scaleVChanged(m_scaleV);
    update();
}

void QDemonImage::setMappingMode(QDemonImage::MappingMode mappingMode)
{
    if (m_mappingMode == mappingMode)
        return;

    m_mappingMode = mappingMode;
    emit mappingModeChanged(m_mappingMode);
    update();
}

void QDemonImage::setHorizontalTiling(QDemonImage::TilingMode tilingModeHorizontal)
{
    if (m_tilingModeHorizontal == tilingModeHorizontal)
        return;

    m_tilingModeHorizontal = tilingModeHorizontal;
    emit horizontalTilingChanged(m_tilingModeHorizontal);
    update();
}

void QDemonImage::setVerticalTiling(QDemonImage::TilingMode tilingModeVertical)
{
    if (m_tilingModeVertical == tilingModeVertical)
        return;

    m_tilingModeVertical = tilingModeVertical;
    emit verticalTilingChanged(m_tilingModeVertical);
    update();
}

void QDemonImage::setRotationUV(float rotationUV)
{
    if (qFuzzyCompare(m_rotationUV, rotationUV))
        return;

    m_rotationUV = rotationUV;
    emit rotationUVChanged(m_rotationUV);
    update();
}

void QDemonImage::setPositionU(float positionU)
{
    if (qFuzzyCompare(m_positionU, positionU))
        return;

    m_positionU = positionU;
    emit positionUChanged(m_positionU);
    update();
}

void QDemonImage::setPositionV(float positionV)
{
    if (qFuzzyCompare(m_positionV, positionV))
        return;

    m_positionV = positionV;
    emit positionVChanged(m_positionV);
    update();
}

void QDemonImage::setPivotU(float pivotU)
{
    if (qFuzzyCompare(m_pivotU, pivotU))
        return;

    m_pivotU = pivotU;
    emit pivotUChanged(m_pivotU);
    update();
}

void QDemonImage::setPivotV(float pivotV)
{
    if (qFuzzyCompare(m_pivotV, pivotV))
        return;

    m_pivotV = pivotV;
    emit pivotVChanged(m_pivotV);
    update();
}

void QDemonImage::setFormat(QDemonImage::Format format)
{
    if (m_format == format)
        return;

    m_format = format;
    emit formatChanged(m_format);
    update();
}

QDemonRenderGraphObject *QDemonImage::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderImage();

    auto imageNode = static_cast<QDemonRenderImage *>(node);

    imageNode->m_imagePath = QQmlFile::urlToLocalFileOrQrc(m_source);
    imageNode->m_scale = QVector2D(m_scaleU, m_scaleV);
    imageNode->m_pivot = QVector2D(m_pivotU, m_pivotV);
    imageNode->m_rotation = m_rotationUV;
    imageNode->m_position = QVector2D(m_positionU, m_positionV);
    imageNode->m_mappingMode = QDemonRenderImage::MappingModes(m_mappingMode);
    imageNode->m_horizontalTilingMode = QDemonRenderTextureCoordOp(m_tilingModeHorizontal);
    imageNode->m_verticalTilingMode = QDemonRenderTextureCoordOp(m_tilingModeVertical);
    imageNode->m_format = QDemonRenderTextureFormat::Format(m_format);
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

QDemonImage::Format QDemonImage::format() const
{
    return m_format;
}

QT_END_NAMESPACE
