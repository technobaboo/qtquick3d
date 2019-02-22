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
#include <QtDemonRuntimeRender/qdemontextrenderer.h>

QT_BEGIN_NAMESPACE

namespace {
// left/top
float GetMinValue(float start, float width, float value, LayerUnitTypes::Enum units)
{
    if (units == LayerUnitTypes::Pixels)
        return start + value;

    return start + (value * width / 100.0f);
}

// width/height
float GetValueLen(float width, float value, LayerUnitTypes::Enum units)
{
    if (units == LayerUnitTypes::Pixels)
        return value;

    return width * value / 100.0f;
}

// right/bottom
float GetMaxValue(float start, float width, float value, LayerUnitTypes::Enum units)
{
    if (units == LayerUnitTypes::Pixels)
        return start + width - value;

    return start + width - (value * width / 100.0f);
}

QVector2D ToRectRelativeCoords(const QVector2D &inCoords, const QDemonRenderRectF &inRect)
{
    return QVector2D(inCoords.x() - inRect.m_x, inCoords.y() - inRect.m_y);
}
}

QDemonLayerRenderHelper::QDemonLayerRenderHelper(const QDemonRenderRectF &inPresentationViewport,
                                                 const QDemonRenderRectF &inPresentationScissor,
                                                 const QVector2D &inPresentationDesignDimensions,
                                                 QDemonRenderLayer &inLayer,
                                                 bool inOffscreen,
                                                 ScaleModes::Enum inScaleMode,
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
            if (m_layer->leftUnits == LayerUnitTypes::Pixels)
                left *= m_scaleFactor.x();

            if (m_layer->rightUnits == LayerUnitTypes::Pixels)
                right *= m_scaleFactor.x();

            if (m_layer->widthUnits == LayerUnitTypes::Pixels)
                width *= m_scaleFactor.x();
        }

        float horzMin = GetMinValue(inPresentationViewport.m_x, inPresentationViewport.m_width,
                                    left, m_layer->leftUnits);
        float horzWidth = GetValueLen(inPresentationViewport.m_width, width, m_layer->widthUnits);
        float horzMax = GetMaxValue(inPresentationViewport.m_x, inPresentationViewport.m_width,
                                    right, m_layer->rightUnits);

        switch (inLayer.horizontalFieldValues) {
        case HorizontalFieldValues::LeftWidth:
            m_viewport.m_x = horzMin;
            m_viewport.m_width = horzWidth;
            break;
        case HorizontalFieldValues::LeftRight:
            m_viewport.m_x = horzMin;
            m_viewport.m_width = horzMax - horzMin;
            break;
        case HorizontalFieldValues::WidthRight:
            m_viewport.m_width = horzWidth;
            m_viewport.m_x = horzMax - horzWidth;
            break;
        }
    }
    {
        float top = m_layer->m_top;
        float bottom = m_layer->m_bottom;
        float height = m_layer->m_height;

        if (m_scaleMode == ScaleModes::FitSelected) {

            if (m_layer->topUnits == LayerUnitTypes::Pixels)
                top *= m_scaleFactor.y();

            if (m_layer->bottomUnits == LayerUnitTypes::Pixels)
                bottom *= m_scaleFactor.y();

            if (m_layer->heightUnits == LayerUnitTypes::Pixels)
                height *= m_scaleFactor.y();
        }

        float vertMin = GetMinValue(inPresentationViewport.m_y, inPresentationViewport.m_height,
                                    bottom, m_layer->bottomUnits);
        float vertWidth =
                GetValueLen(inPresentationViewport.m_height, height, m_layer->heightUnits);
        float vertMax = GetMaxValue(inPresentationViewport.m_y, inPresentationViewport.m_height,
                                    top, m_layer->topUnits);

        switch (inLayer.verticalFieldValues) {
        case VerticalFieldValues::HeightBottom:
            m_viewport.m_y = vertMin;
            m_viewport.m_height = vertWidth;
            break;
        case VerticalFieldValues::TopBottom:
            m_viewport.m_y = vertMin;
            m_viewport.m_height = vertMax - vertMin;
            break;
        case VerticalFieldValues::TopHeight:
            m_viewport.m_height = vertWidth;
            m_viewport.m_y = vertMax - vertWidth;
            break;
        }
    }

    m_viewport.m_width = qMax(1.0f, m_viewport.m_width);
    m_viewport.m_height = qMax(1.0f, m_viewport.m_height);
    // Now force the viewport to be a multiple of four in width and height.  This is because
    // when rendering to a texture we have to respect this and not forcing it causes scaling issues
    // that are noticeable especially in situations where customers are using text and such.
    float originalWidth = m_viewport.m_width;
    float originalHeight = m_viewport.m_height;

    m_viewport.m_width = (float)QDemonTextRendererInterface::nextMultipleOf4((quint32)m_viewport.m_width);
    m_viewport.m_height = (float)QDemonTextRendererInterface::nextMultipleOf4((quint32)m_viewport.m_height);

    // Now fudge the offsets to account for this slight difference
    m_viewport.m_x += (originalWidth - m_viewport.m_width) / 2.0f;
    m_viewport.m_y += (originalHeight - m_viewport.m_height) / 2.0f;

    m_scissor = m_viewport;
    m_scissor.ensureInBounds(inPresentationScissor);
    Q_ASSERT(m_scissor.m_width >= 0.0f);
    Q_ASSERT(m_scissor.m_height >= 0.0f);
}

// This is the viewport the camera will use to setup the projection.
QDemonRenderRectF QDemonLayerRenderHelper::getLayerRenderViewport() const
{
    if (m_offscreen)
        return QDemonRenderRectF(0, 0, m_viewport.m_width, (float)m_viewport.m_height);
    else
        return m_viewport;
}

QSize QDemonLayerRenderHelper::getTextureDimensions() const
{
    quint32 width = (quint32)m_viewport.m_width;
    quint32 height = (quint32)m_viewport.m_height;
    return QSize(QDemonTextRendererInterface::nextMultipleOf4(width),
                 QDemonTextRendererInterface::nextMultipleOf4(height));
}

QDemonCameraGlobalCalculationResult QDemonLayerRenderHelper::setupCameraForRender(QDemonRenderCamera &inCamera)
{
    m_camera = &inCamera;
    QDemonRenderRectF rect = getLayerRenderViewport();
    if (m_scaleMode == ScaleModes::FitSelected) {
        rect.m_width =
                (float)(QDemonTextRendererInterface::nextMultipleOf4((quint32)(rect.m_width / m_scaleFactor.x())));
        rect.m_height =
                (float)(QDemonTextRendererInterface::nextMultipleOf4((quint32)(rect.m_height / m_scaleFactor.y())));
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
    QVector2D theLocalMouse = m_viewport.toRectRelative(correctCoords);

    float theRenderRectWidth = m_viewport.m_width;
    float theRenderRectHeight = m_viewport.m_height;
    // Crop the mouse to the rect.  Apply no further translations.
    if (inForceIntersect == false
            && (theLocalMouse.x() < 0.0f || theLocalMouse.x() >= theRenderRectWidth
                || theLocalMouse.y() < 0.0f || theLocalMouse.y() >= theRenderRectHeight)) {
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
    QDemonOption<QVector2D> theCoords(
                getLayerMouseCoords(inMouseCoords, inWindowDimensions, inForceIntersect));
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
    return m_scissor.m_height >= 2.0f && m_scissor.m_width >= 2.0f;
}

QT_END_NAMESPACE
