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
#include <QtQuick/QQuickItem>
#include <QtQuick/private/qquickitem_p.h>
#include <qquick3dviewport.h>

#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dutils_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Texture
    \inqmlmodule QtQuick3D
    \brief Lets you add a texture image to the material.
*/

QQuick3DTexture::QQuick3DTexture() {}

QQuick3DTexture::~QQuick3DTexture()
{
    delete m_layer;
}

QUrl QQuick3DTexture::source() const
{
    return m_source;
}

QQuickItem *QQuick3DTexture::sourceItem() const
{
    return m_sourceItem;
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
    m_dirtyFlags.setFlag(DirtyFlag::SourceDirty);
    emit sourceChanged(m_source);
    update();
}

void QQuick3DTexture::setSourceItem(QQuickItem *sourceItem)
{
    if (m_sourceItem == sourceItem)
        return;

    //TODO: Also clear the source property?

    if (m_sourceItem) {
        QQuickItemPrivate *d = QQuickItemPrivate::get(m_sourceItem);
        d->derefFromEffectItem(m_sourceItemReparented);
        d->removeItemChangeListener(this, QQuickItemPrivate::Geometry);
        disconnect(m_sourceItem, SIGNAL(destroyed(QObject*)), this, SLOT(sourceItemDestroyed(QObject*)));
        if (m_sourceItem->window())
            d->derefWindow();
        if (m_sourceItemReparented) {
            m_sourceItem->setParentItem(nullptr);
            m_sourceItemReparented = false;
        }
    }

    m_sourceItem = sourceItem;

    if (sourceItem) {
        if (!sourceItem->parentItem()) {
            QQuick3DViewport *view3D = nullptr;
            for (auto *p = parent(); p != nullptr && view3D == nullptr; p = p->parent())
                view3D = qobject_cast<QQuick3DViewport *>(p);

            if (!view3D)
                qWarning() << "TODO: handle this case"; // TODO

            m_sourceItem->setParentItem(view3D);
            m_sourceItemReparented = true;
        }

        connect(m_sourceItem, SIGNAL(destroyed(QObject*)), this, SLOT(sourceItemDestroyed(QObject*)));

        auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
        sourcePrivate->refFromEffectItem(m_sourceItemReparented);
        sourcePrivate->addItemChangeListener(this, QQuickItemPrivate::Geometry);
    }

    emit sourceItemChanged(m_sourceItem);
    update();
}

void QQuick3DTexture::setScaleU(float scaleU)
{
    if (qFuzzyCompare(m_scaleU, scaleU))
        return;

    m_scaleU = scaleU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit scaleUChanged(m_scaleU);
    update();
}

void QQuick3DTexture::setScaleV(float scaleV)
{
    if (qFuzzyCompare(m_scaleV, scaleV))
        return;

    m_scaleV = scaleV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
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
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit rotationUVChanged(m_rotationUV);
    update();
}

void QQuick3DTexture::setPositionU(float positionU)
{
    if (qFuzzyCompare(m_positionU, positionU))
        return;

    m_positionU = positionU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit positionUChanged(m_positionU);
    update();
}

void QQuick3DTexture::setPositionV(float positionV)
{
    if (qFuzzyCompare(m_positionV, positionV))
        return;

    m_positionV = positionV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit positionVChanged(m_positionV);
    update();
}

void QQuick3DTexture::setPivotU(float pivotU)
{
    if (qFuzzyCompare(m_pivotU, pivotU))
        return;

    m_pivotU = pivotU;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
    emit pivotUChanged(m_pivotU);
    update();
}

void QQuick3DTexture::setPivotV(float pivotV)
{
    if (qFuzzyCompare(m_pivotV, pivotV))
        return;

    m_pivotV = pivotV;
    m_dirtyFlags.setFlag(DirtyFlag::TransformDirty);
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

    if (m_dirtyFlags.testFlag(DirtyFlag::TransformDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::TransformDirty, false);
        imageNode->m_scale = QVector2D(m_scaleU, m_scaleV);
        imageNode->m_pivot = QVector2D(m_pivotU, m_pivotV);
        imageNode->m_rotation = m_rotationUV;
        imageNode->m_position = QVector2D(m_positionU, m_positionV);

        imageNode->m_flags.setFlag(QDemonRenderImage::Flag::TransformDirty);
    }

    bool nodeChanged = false;
    if (m_dirtyFlags.testFlag(DirtyFlag::SourceDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::SourceDirty, false);
        imageNode->m_imagePath = QQmlFile::urlToLocalFileOrQrc(m_source);
        nodeChanged = true;
    }

    nodeChanged |= qUpdateIfNeeded(imageNode->m_mappingMode,
                                  QDemonRenderImage::MappingModes(m_mappingMode));
    nodeChanged |= qUpdateIfNeeded(imageNode->m_horizontalTilingMode,
                                  QDemonRenderTextureCoordOp(m_tilingModeHorizontal));
    nodeChanged |= qUpdateIfNeeded(imageNode->m_verticalTilingMode,
                                  QDemonRenderTextureCoordOp(m_tilingModeVertical));
    QDemonRenderTextureFormat format{QDemonRenderTextureFormat::Format(m_format)};
    nodeChanged |= qUpdateIfNeeded(imageNode->m_format, format);

    if (m_sourceItem) { // TODO: handle width == 0 || height == 0
        QQuickWindow *window = m_sourceItem->window();
        if (!window) {
            // Do a hack to set the window
            for (auto *p = parent(); p != nullptr; p = p->parent()) {
                if (auto *view = qobject_cast<QQuick3DViewport *>(p)) {
                    window = view->window(); // TODO: will probably break if we nest?
                    break;
                }
            }
            if (!window) {
                qWarning() << "Unable to get window, this will probably not work";
            } else {
                auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
                sourcePrivate->refWindow(window); // TODO: deref window as well
            }
        }

        // TODO: handle the simpler case where the sourceItem is a texture provider

        ensureTexture();

        m_layer->setItem(QQuickItemPrivate::get(m_sourceItem)->itemNode());
        QRectF sourceRect = QRectF(0, 0, m_sourceItem->width(), m_sourceItem->height());
        m_layer->setRect(sourceRect);

        QSize textureSize(qCeil(qAbs(sourceRect.width())), qCeil(qAbs(sourceRect.height())));

        // TODO: create larger textures on high-dpi displays?

        auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
        const QSize minTextureSize = sourcePrivate->sceneGraphContext()->minimumFBOSize();
        // Keep power-of-two by doubling the size.
        while (textureSize.width() < minTextureSize.width())
            textureSize.rwidth() *= 2;
        while (textureSize.height() < minTextureSize.height())
            textureSize.rheight() *= 2;

        m_layer->setSize(textureSize);

        // TODO: set mipmapFiltering, filtering, hWrap, vWrap?

        imageNode->m_qsgTexture = m_layer;
        nodeChanged = true; // @todo: make more granular
    } else {
        if (m_layer)
            m_layer->setItem(nullptr);
        nodeChanged |= qUpdateIfNeeded(imageNode->m_qsgTexture, static_cast<QSGTexture *>(nullptr));
    }

    if (nodeChanged)
        imageNode->m_flags.setFlag(QDemonRenderImage::Flag::Dirty);

    return imageNode;
}

void QQuick3DTexture::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemChange::ItemSceneChange && m_sourceItem) {
        qWarning() << Q_FUNC_INFO << "TODO: deref old and ref new window for m_sourceItem";
//        auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
//        if (value.window)
//            sourcePrivate->refWindow(value.window);
//        else
//            sourcePrivate->derefWindow();
    }
    QQuick3DObject::itemChange(change, value);
}

void QQuick3DTexture::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &geometry)
{
    Q_ASSERT(item == m_sourceItem);
    Q_UNUSED(item);
    Q_UNUSED(geometry);
    if (change.sizeChange())
        update();
}

void QQuick3DTexture::sourceItemDestroyed(QObject *item)
{
    Q_ASSERT(item == m_sourceItem);
    Q_UNUSED(item);
    m_sourceItem = nullptr;
    emit sourceItemChanged(m_sourceItem);
    update();
}

void QQuick3DTexture::ensureTexture()
{
    if (m_layer)
        return;

    auto *sourcePrivate = QQuickItemPrivate::get(m_sourceItem);
    Q_ASSERT_X(sourcePrivate->window && sourcePrivate->sceneGraphRenderContext()
               && QThread::currentThread() == sourcePrivate->sceneGraphRenderContext()->thread(),
               Q_FUNC_INFO, "Cannot be used outside the rendering thread");

    QSGRenderContext *rc = sourcePrivate->sceneGraphRenderContext();
    auto *layer = rc->sceneGraphContext()->createLayer(rc);
    connect(sourcePrivate->window, SIGNAL(sceneGraphInvalidated()), layer, SLOT(invalidated()), Qt::DirectConnection);
    connect(layer, SIGNAL(updateRequested()), this, SLOT(update()));
    //connect(layer, SIGNAL(scheduledUpdateCompleted()), this, SIGNAL(scheduledUpdateCompleted()));

    QQuick3DSceneManager *sceneManager = sceneRenderer();
    sceneManager->qsgDynamicTextures << layer;
    connect(layer, &QObject::destroyed, sceneManager, [sceneManager, layer]() {
        sceneManager->qsgDynamicTextures.removeAll(layer);
    });

    m_layer = layer;
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
