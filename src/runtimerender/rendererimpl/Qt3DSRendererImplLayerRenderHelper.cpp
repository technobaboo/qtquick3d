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
#include <Qt3DSRendererImplLayerRenderHelper.h>
#include <Qt3DSRenderLayer.h>
#include <Qt3DSTextRenderer.h>

using namespace render;

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
    return QVector2D(inCoords.x - inRect.m_X, inCoords.y - inRect.m_Y);
}
}

SLayerRenderHelper::SLayerRenderHelper()
    : m_Layer(nullptr)
    , m_Camera(nullptr)
    , m_Offscreen(false)
{
}

SLayerRenderHelper::SLayerRenderHelper(const QDemonRenderRectF &inPresentationViewport,
                                       const QDemonRenderRectF &inPresentationScissor,
                                       const QVector2D &inPresentationDesignDimensions,
                                       SLayer &inLayer, bool inOffscreen,
                                       ScaleModes::Enum inScaleMode,
                                       QVector2D inScaleFactor)
    : m_PresentationViewport(inPresentationViewport)
    , m_PresentationScissor(inPresentationScissor)
    , m_PresentationDesignDimensions(inPresentationDesignDimensions)
    , m_Layer(&inLayer)
    , m_Offscreen(inOffscreen)
    , m_ScaleMode(inScaleMode)
    , m_ScaleFactor(inScaleFactor)
{
    {
        float left = m_Layer->m_Left;
        float right = m_Layer->m_Right;
        float width = m_Layer->m_Width;

        if (m_ScaleMode == ScaleModes::FitSelected) {
            if (m_Layer->m_LeftUnits == LayerUnitTypes::Pixels)
                left *= m_ScaleFactor.x;

            if (m_Layer->m_RightUnits == LayerUnitTypes::Pixels)
                right *= m_ScaleFactor.x;

            if (m_Layer->m_WidthUnits == LayerUnitTypes::Pixels)
                width *= m_ScaleFactor.x;
        }

        float horzMin = GetMinValue(inPresentationViewport.m_X, inPresentationViewport.m_Width,
                                    left, m_Layer->m_LeftUnits);
        float horzWidth = GetValueLen(inPresentationViewport.m_Width, width, m_Layer->m_WidthUnits);
        float horzMax = GetMaxValue(inPresentationViewport.m_X, inPresentationViewport.m_Width,
                                    right, m_Layer->m_RightUnits);

        switch (inLayer.m_HorizontalFieldValues) {
        case HorizontalFieldValues::LeftWidth:
            m_Viewport.m_X = horzMin;
            m_Viewport.m_Width = horzWidth;
            break;
        case HorizontalFieldValues::LeftRight:
            m_Viewport.m_X = horzMin;
            m_Viewport.m_Width = horzMax - horzMin;
            break;
        case HorizontalFieldValues::WidthRight:
            m_Viewport.m_Width = horzWidth;
            m_Viewport.m_X = horzMax - horzWidth;
            break;
        }
    }
    {
        float top = m_Layer->m_Top;
        float bottom = m_Layer->m_Bottom;
        float height = m_Layer->m_Height;

        if (m_ScaleMode == ScaleModes::FitSelected) {

            if (m_Layer->m_TopUnits == LayerUnitTypes::Pixels)
                top *= m_ScaleFactor.y;

            if (m_Layer->m_BottomUnits == LayerUnitTypes::Pixels)
                bottom *= m_ScaleFactor.y;

            if (m_Layer->m_HeightUnits == LayerUnitTypes::Pixels)
                height *= m_ScaleFactor.y;
        }

        float vertMin = GetMinValue(inPresentationViewport.m_Y, inPresentationViewport.m_Height,
                                    bottom, m_Layer->m_BottomUnits);
        float vertWidth =
            GetValueLen(inPresentationViewport.m_Height, height, m_Layer->m_HeightUnits);
        float vertMax = GetMaxValue(inPresentationViewport.m_Y, inPresentationViewport.m_Height,
                                    top, m_Layer->m_TopUnits);

        switch (inLayer.m_VerticalFieldValues) {
        case VerticalFieldValues::HeightBottom:
            m_Viewport.m_Y = vertMin;
            m_Viewport.m_Height = vertWidth;
            break;
        case VerticalFieldValues::TopBottom:
            m_Viewport.m_Y = vertMin;
            m_Viewport.m_Height = vertMax - vertMin;
            break;
        case VerticalFieldValues::TopHeight:
            m_Viewport.m_Height = vertWidth;
            m_Viewport.m_Y = vertMax - vertWidth;
            break;
        }
    }

    m_Viewport.m_Width = NVMax(1.0f, m_Viewport.m_Width);
    m_Viewport.m_Height = NVMax(1.0f, m_Viewport.m_Height);
    // Now force the viewport to be a multiple of four in width and height.  This is because
    // when rendering to a texture we have to respect this and not forcing it causes scaling issues
    // that are noticeable especially in situations where customers are using text and such.
    float originalWidth = m_Viewport.m_Width;
    float originalHeight = m_Viewport.m_Height;

    m_Viewport.m_Width = (float)ITextRenderer::NextMultipleOf4((quint32)m_Viewport.m_Width);
    m_Viewport.m_Height = (float)ITextRenderer::NextMultipleOf4((quint32)m_Viewport.m_Height);

    // Now fudge the offsets to account for this slight difference
    m_Viewport.m_X += (originalWidth - m_Viewport.m_Width) / 2.0f;
    m_Viewport.m_Y += (originalHeight - m_Viewport.m_Height) / 2.0f;

    m_Scissor = m_Viewport;
    m_Scissor.EnsureInBounds(inPresentationScissor);
    Q_ASSERT(m_Scissor.m_Width >= 0.0f);
    Q_ASSERT(m_Scissor.m_Height >= 0.0f);
}

// This is the viewport the camera will use to setup the projection.
QDemonRenderRectF SLayerRenderHelper::GetLayerRenderViewport() const
{
    if (m_Offscreen)
        return QDemonRenderRectF(0, 0, m_Viewport.m_Width, (float)m_Viewport.m_Height);
    else
        return m_Viewport;
}

QSize SLayerRenderHelper::GetTextureDimensions() const
{
    quint32 width = (quint32)m_Viewport.m_Width;
    quint32 height = (quint32)m_Viewport.m_Height;
    return QSize(ITextRenderer::NextMultipleOf4(width),
                             ITextRenderer::NextMultipleOf4(height));
}

SCameraGlobalCalculationResult SLayerRenderHelper::SetupCameraForRender(SCamera &inCamera)
{
    m_Camera = &inCamera;
    QDemonRenderRectF rect = GetLayerRenderViewport();
    if (m_ScaleMode == ScaleModes::FitSelected) {
        rect.m_Width =
            (float)(ITextRenderer::NextMultipleOf4((quint32)(rect.m_Width / m_ScaleFactor.x)));
        rect.m_Height =
            (float)(ITextRenderer::NextMultipleOf4((quint32)(rect.m_Height / m_ScaleFactor.y)));
    }
    return m_Camera->CalculateGlobalVariables(rect, m_PresentationDesignDimensions);
}

Option<QVector2D> SLayerRenderHelper::GetLayerMouseCoords(const QVector2D &inMouseCoords,
                                                       const QVector2D &inWindowDimensions,
                                                       bool inForceIntersect) const
{
    // First invert the y so we are dealing with numbers in a normal coordinate space.
    // Second, move into our layer's coordinate space
    QVector2D correctCoords(inMouseCoords.x, inWindowDimensions.y - inMouseCoords.y);
    QVector2D theLocalMouse = m_Viewport.ToRectRelative(correctCoords);

    float theRenderRectWidth = m_Viewport.m_Width;
    float theRenderRectHeight = m_Viewport.m_Height;
    // Crop the mouse to the rect.  Apply no further translations.
    if (inForceIntersect == false
        && (theLocalMouse.x < 0.0f || theLocalMouse.x >= theRenderRectWidth
            || theLocalMouse.y < 0.0f || theLocalMouse.y >= theRenderRectHeight)) {
        return Empty();
    }
    return theLocalMouse;
}

Option<SRay> SLayerRenderHelper::GetPickRay(const QVector2D &inMouseCoords,
                                            const QVector2D &inWindowDimensions,
                                            bool inForceIntersect) const
{
    if (m_Camera == nullptr)
        return Empty();
    Option<QVector2D> theCoords(
        GetLayerMouseCoords(inMouseCoords, inWindowDimensions, inForceIntersect));
    if (theCoords.hasValue()) {
        // The cameras projection is different if we are onscreen vs. offscreen.
        // When offscreen, we need to move the mouse coordinates into a local space
        // to the layer.
        return m_Camera->Unproject(*theCoords, m_Viewport, m_PresentationDesignDimensions);
    }
    return Empty();
}

bool SLayerRenderHelper::IsLayerVisible() const
{
    return m_Scissor.m_Height >= 2.0f && m_Scissor.m_Width >= 2.0f;
}
