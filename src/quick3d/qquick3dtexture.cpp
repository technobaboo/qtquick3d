/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquick3dtexture.h"
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtQml/QQmlFile>

#include "qquick3dobject_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Texture
    \inqmlmodule QtQuick3D
    \brief Lets you add a texture image to the material.
*/

QQuick3DTexture::QQuick3DTexture() {}

QQuick3DTexture::~QQuick3DTexture() {}

QUrl QQuick3DTexture::source() const
{
    return m_source;
}

float QQuick3DTexture::scaleU() const
{
    return m_scaleU;
}

float QQuick3DTexture::scaleV() const
{
    return m_scaleV;
}

QQuick3DTexture::MappingMode QQuick3DTexture::mappingMode() const
{
    return m_mappingMode;
}

QQuick3DTexture::TilingMode QQuick3DTexture::horizontalTiling() const
{
    return m_tilingModeHorizontal;
}

QQuick3DTexture::TilingMode QQuick3DTexture::verticalTiling() const
{
    return m_tilingModeVertical;
}

float QQuick3DTexture::rotationUV() const
{
    return m_rotationUV;
}

float QQuick3DTexture::positionU() const
{
    return m_positionU;
}

float QQuick3DTexture::positionV() const
{
    return m_positionV;
}

float QQuick3DTexture::pivotU() const
{
    return m_pivotU;
}

float QQuick3DTexture::pivotV() const
{
    return m_pivotV;
}

QQuick3DObject::Type QQuick3DTexture::type() const
{
    return QQuick3DObject::Image;
}

void QQuick3DTexture::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
    update();
}

void QQuick3DTexture::setScaleU(float scaleU)
{
    if (qFuzzyCompare(m_scaleU, scaleU))
        return;

    m_scaleU = scaleU;
    emit scaleUChanged(m_scaleU);
    update();
}

void QQuick3DTexture::setScaleV(float scaleV)
{
    if (qFuzzyCompare(m_scaleV, scaleV))
        return;

    m_scaleV = scaleV;
    emit scaleVChanged(m_scaleV);
    update();
}

void QQuick3DTexture::setMappingMode(QQuick3DTexture::MappingMode mappingMode)
{
    if (m_mappingMode == mappingMode)
        return;

    m_mappingMode = mappingMode;
    emit mappingModeChanged(m_mappingMode);
    update();
}

void QQuick3DTexture::setHorizontalTiling(QQuick3DTexture::TilingMode tilingModeHorizontal)
{
    if (m_tilingModeHorizontal == tilingModeHorizontal)
        return;

    m_tilingModeHorizontal = tilingModeHorizontal;
    emit horizontalTilingChanged(m_tilingModeHorizontal);
    update();
}

void QQuick3DTexture::setVerticalTiling(QQuick3DTexture::TilingMode tilingModeVertical)
{
    if (m_tilingModeVertical == tilingModeVertical)
        return;

    m_tilingModeVertical = tilingModeVertical;
    emit verticalTilingChanged(m_tilingModeVertical);
    update();
}

void QQuick3DTexture::setRotationUV(float rotationUV)
{
    if (qFuzzyCompare(m_rotationUV, rotationUV))
        return;

    m_rotationUV = rotationUV;
    emit rotationUVChanged(m_rotationUV);
    update();
}

void QQuick3DTexture::setPositionU(float positionU)
{
    if (qFuzzyCompare(m_positionU, positionU))
        return;

    m_positionU = positionU;
    emit positionUChanged(m_positionU);
    update();
}

void QQuick3DTexture::setPositionV(float positionV)
{
    if (qFuzzyCompare(m_positionV, positionV))
        return;

    m_positionV = positionV;
    emit positionVChanged(m_positionV);
    update();
}

void QQuick3DTexture::setPivotU(float pivotU)
{
    if (qFuzzyCompare(m_pivotU, pivotU))
        return;

    m_pivotU = pivotU;
    emit pivotUChanged(m_pivotU);
    update();
}

void QQuick3DTexture::setPivotV(float pivotV)
{
    if (qFuzzyCompare(m_pivotV, pivotV))
        return;

    m_pivotV = pivotV;
    emit pivotVChanged(m_pivotV);
    update();
}

void QQuick3DTexture::setFormat(QQuick3DTexture::Format format)
{
    if (m_format == format)
        return;

    m_format = format;
    emit formatChanged(m_format);
    update();
}

QDemonRenderGraphObject *QQuick3DTexture::updateSpatialNode(QDemonRenderGraphObject *node)
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

QDemonRenderImage *QQuick3DTexture::getRenderImage()
{
    QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(this);
    return static_cast<QDemonRenderImage *>(p->spatialNode);
}

QQuick3DTexture::Format QQuick3DTexture::format() const
{
    return m_format;
}

QT_END_NAMESPACE
