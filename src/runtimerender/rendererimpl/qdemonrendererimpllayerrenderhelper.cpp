/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
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
#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderhelper.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendererutil.h>

QT_BEGIN_NAMESPACE

namespace {
// left/top
float getMinValue(float start, float width, float value, QDemonRenderLayer::UnitType units)
{
    if (units == QDemonRenderLayer::UnitType::Pixels)
        return start + value;

    return start + (value * width / 100.0f);
}

// width/height
float getValueLen(float width, float value, QDemonRenderLayer::UnitType units)
{
    if (units == QDemonRenderLayer::UnitType::Pixels)
        return value;

    return width * value / 100.0f;
}

// right/bottom
float getMaxValue(float start, float width, float value, QDemonRenderLayer::UnitType units)
{
    if (units == QDemonRenderLayer::UnitType::Pixels)
        return start + width - value;

    return start + width - (value * width / 100.0f);
}

QVector2D toRectRelativeCoords(const QVector2D &inCoords, const QRectF &inRect)
{
    return QVector2D(inCoords.x() - inRect.x(), inCoords.y() - inRect.y());
}
}

QDemonLayerRenderHelper::QDemonLayerRenderHelper(const QRectF &inPresentationViewport,
                                                 const QRectF &inPresentationScissor,
                                                 const QVector2D &inPresentationDesignDimensions,
                                                 QDemonRenderLayer &inLayer,
                                                 bool inOffscreen,
                                                 ScaleModes inScaleMode,
                                                 QVector2D inScaleFactor)
    : m_presentationViewport(inPresentationViewport)
    , m_presentationScissor(inPresentationScissor)
    , m_presentationDesignDimensions(inPresentationDesignDimensions)
    , m_layer(&inLayer)
    , m_offscreen(inOffscreen)
    , m_scaleMode(inScaleMode)
    , m_scaleFactor(inScaleFactor)
{
    {
        float left = m_layer->m_left;
        float right = m_layer->m_right;
        float width = m_layer->m_width;

        if (m_scaleMode == ScaleModes::FitSelected) {
            if (m_layer->leftUnits == QDemonRenderLayer::UnitType::Pixels)
                left *= m_scaleFactor.x();

            if (m_layer->rightUnits == QDemonRenderLayer::UnitType::Pixels)
                right *= m_scaleFactor.x();

            if (m_layer->widthUnits == QDemonRenderLayer::UnitType::Pixels)
                width *= m_scaleFactor.x();
        }

        float horzMin = getMinValue(inPresentationViewport.x(), inPresentationViewport.width(), left, m_layer->leftUnits);
        float horzWidth = getValueLen(inPresentationViewport.width(), width, m_layer->widthUnits);
        float horzMax = getMaxValue(inPresentationViewport.x(), inPresentationViewport.width(), right, m_layer->rightUnits);

        switch (inLayer.horizontalFieldValues) {
        case QDemonRenderLayer::HorizontalField::LeftWidth:
            m_viewport.setX(horzMin);
            m_viewport.setWidth(horzWidth);
            break;
        case QDemonRenderLayer::HorizontalField::LeftRight:
            m_viewport.setX(horzMin);
            m_viewport.setWidth(horzMax - horzMin);
            break;
        case QDemonRenderLayer::HorizontalField::WidthRight:
            m_viewport.setWidth(horzWidth);
            m_viewport.setX(horzMax - horzWidth);
            break;
        }
    }
    {
        float top = m_layer->m_top;
        float bottom = m_layer->m_bottom;
        float height = m_layer->m_height;

        if (m_scaleMode == ScaleModes::FitSelected) {

            if (m_layer->topUnits == QDemonRenderLayer::UnitType::Pixels)
                top *= m_scaleFactor.y();

            if (m_layer->bottomUnits == QDemonRenderLayer::UnitType::Pixels)
                bottom *= m_scaleFactor.y();

            if (m_layer->heightUnits == QDemonRenderLayer::UnitType::Pixels)
                height *= m_scaleFactor.y();
        }

        float vertMin = getMinValue(inPresentationViewport.y(), inPresentationViewport.height(), bottom, m_layer->bottomUnits);
        float vertWidth = getValueLen(inPresentationViewport.height(), height, m_layer->heightUnits);
        float vertMax = getMaxValue(inPresentationViewport.y(), inPresentationViewport.height(), top, m_layer->topUnits);

        switch (inLayer.verticalFieldValues) {
        case QDemonRenderLayer::VerticalField::HeightBottom:
            m_viewport.setY(vertMin);
            m_viewport.setHeight(vertWidth);
            break;
        case QDemonRenderLayer::VerticalField::TopBottom:
            m_viewport.setY(vertMin);
            m_viewport.setHeight(vertMax - vertMin);
            break;
        case QDemonRenderLayer::VerticalField::TopHeight:
            m_viewport.setHeight(vertWidth);
            m_viewport.setY(vertMax - vertWidth);
            break;
        }
    }

    m_viewport.setWidth(qMax<qreal>(1.0f, m_viewport.width()));
    m_viewport.setHeight(qMax<qreal>(1.0f, m_viewport.height()));
    // Now force the viewport to be a multiple of four in width and height.  This is because
    // when rendering to a texture we have to respect this and not forcing it causes scaling issues
    // that are noticeable especially in situations where customers are using text and such.
    float originalWidth = m_viewport.width();
    float originalHeight = m_viewport.height();

    m_viewport.setWidth((float)QDemonRendererUtil::nextMultipleOf4((quint32)m_viewport.width()));
    m_viewport.setHeight((float)QDemonRendererUtil::nextMultipleOf4((quint32)m_viewport.height()));

    // Now fudge the offsets to account for this slight difference
    m_viewport.setX(m_viewport.x() + (originalWidth - m_viewport.width()) / 2.0f);
    m_viewport.setY(m_viewport.y() + (originalHeight - m_viewport.height()) / 2.0f);

    m_scissor = m_viewport;
    m_scissor &= inPresentationScissor; // ensureInBounds/intersected
    Q_ASSERT(m_scissor.width() >= 0.0f);
    Q_ASSERT(m_scissor.height() >= 0.0f);
}

// This is the viewport the camera will use to setup the projection.
QRectF QDemonLayerRenderHelper::getLayerRenderViewport() const
{
    if (m_offscreen)
        return QRectF(0, 0, m_viewport.width(), (float)m_viewport.height());
    else
        return m_viewport;
}

QSize QDemonLayerRenderHelper::getTextureDimensions() const
{
    quint32 width = (quint32)m_viewport.width();
    quint32 height = (quint32)m_viewport.height();
    return QSize(QDemonRendererUtil::nextMultipleOf4(width), QDemonRendererUtil::nextMultipleOf4(height));
}

QDemonCameraGlobalCalculationResult QDemonLayerRenderHelper::setupCameraForRender(QDemonRenderCamera &inCamera)
{
    m_camera = &inCamera;
    QRectF rect = getLayerRenderViewport();
    if (m_scaleMode == ScaleModes::FitSelected) {
        rect.setWidth((float)(QDemonRendererUtil::nextMultipleOf4((quint32)(rect.width() / m_scaleFactor.x()))));
        rect.setHeight((float)(QDemonRendererUtil::nextMultipleOf4((quint32)(rect.height() / m_scaleFactor.y()))));
    }
    return m_camera->calculateGlobalVariables(rect, m_presentationDesignDimensions);
}

QDemonOption<QVector2D> QDemonLayerRenderHelper::getLayerMouseCoords(const QVector2D &inMouseCoords,
                                                                     const QVector2D &inWindowDimensions,
                                                                     bool inForceIntersect) const
{
    // First invert the y so we are dealing with numbers in a normal coordinate space.
    // Second, move into our layer's coordinate space
    QVector2D correctCoords(inMouseCoords.x(), inWindowDimensions.y() - inMouseCoords.y());
    QVector2D theLocalMouse = toRectRelative(m_viewport, correctCoords);

    float theRenderRectWidth = m_viewport.width();
    float theRenderRectHeight = m_viewport.height();
    // Crop the mouse to the rect.  Apply no further translations.
    if (inForceIntersect == false
        && (theLocalMouse.x() < 0.0f || theLocalMouse.x() >= theRenderRectWidth || theLocalMouse.y() < 0.0f
            || theLocalMouse.y() >= theRenderRectHeight)) {
        return QDemonEmpty();
    }
    return theLocalMouse;
}

QDemonOption<QDemonRenderRay> QDemonLayerRenderHelper::getPickRay(const QVector2D &inMouseCoords,
                                                                  const QVector2D &inWindowDimensions,
                                                                  bool inForceIntersect) const
{
    if (m_camera == nullptr)
        return QDemonEmpty();
    QDemonOption<QVector2D> theCoords(getLayerMouseCoords(inMouseCoords, inWindowDimensions, inForceIntersect));
    if (theCoords.hasValue()) {
        // The cameras projection is different if we are onscreen vs. offscreen.
        // When offscreen, we need to move the mouse coordinates into a local space
        // to the layer.
        return m_camera->unproject(*theCoords, m_viewport, m_presentationDesignDimensions);
    }
    return QDemonEmpty();
}

bool QDemonLayerRenderHelper::isLayerVisible() const
{
    return m_scissor.height() >= 2.0f && m_scissor.width() >= 2.0f;
}

QT_END_NAMESPACE
