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
#ifndef QDEMON_RENDER_LAYER_HELPER_IMPL_H
#define QDEMON_RENDER_LAYER_HELPER_IMPL_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

QT_BEGIN_NAMESPACE

/**	An independent, testable entity to encapsulate taking at least:
     *  layer, current viewport rect, current scissor rect, presentation design dimensions
     *	and producing a set of rectangles:
     *	layer viewport rect (inside viewport rect and calculated using outer viewport rect info)
     *	layer scissor rect (inside current scissor rect)
     *	layer camera rect (may be the viewport rect)
     *
     *  In the case where we have to render offscreen for this layer then we need to handle produce
     *	a set of texture dimensions and the layer camera rect ends up being same size but with no
     *offsets.
     *
     *  This object should handle part of any translation from screenspace to global space.
     *	I am using language level access control on this object because it needs specific
     *	interface design that will enable future modifications.
     */
struct QDemonLayerRenderHelper
{
private:
    QDemonRenderRectF m_presentationViewport;
    QDemonRenderRectF m_presentationScissor;
    QVector2D m_presentationDesignDimensions;
    QDemonRenderLayer *m_layer = nullptr;
    QDemonRenderCamera *m_camera = nullptr;
    bool m_offscreen = false;

    QDemonRenderRectF m_viewport;
    QDemonRenderRectF m_scissor;

    ScaleModes::Enum m_scaleMode;
    QVector2D m_scaleFactor;

public:
    QDemonLayerRenderHelper() = default;

    QDemonLayerRenderHelper(const QDemonRenderRectF &inPresentationViewport,
                            const QDemonRenderRectF &inPresentationScissor,
                            const QVector2D &inPresentationDesignDimensions,
                            QDemonRenderLayer &inLayer,
                            bool inOffscreen,
                            ScaleModes::Enum inScaleMode,
                            QVector2D inScaleFactor);

    QDemonRenderRectF getPresentationViewport() const { return m_presentationViewport; }
    QDemonRenderRectF getPresentationScissor() const { return m_presentationScissor; }
    QVector2D getPresentationDesignDimensions() const { return m_presentationDesignDimensions; }
    QDemonRenderLayer *getLayer() const { return m_layer; }
    QDemonRenderCamera *getCamera() const { return m_camera; }
    bool isOffscreen() const { return m_offscreen; }

    // Does not differ whether offscreen or not, simply states how this layer maps to the
    // presentation
    QDemonRenderRectF getLayerToPresentationViewport() const { return m_viewport; }
    // Does not differ whether offscreen or not, scissor rect of how this layer maps to
    // presentation.
    QDemonRenderRectF getLayerToPresentationScissorRect() const { return m_scissor; }

    QSize getTextureDimensions() const;

    QDemonCameraGlobalCalculationResult setupCameraForRender(QDemonRenderCamera &inCamera);

    QDemonOption<QVector2D> getLayerMouseCoords(const QVector2D &inMouseCoords,
                                                const QVector2D &inWindowDimensions,
                                                bool inForceIntersect) const;

    QDemonOption<QDemonRenderRay> getPickRay(const QVector2D &inMouseCoords,
                                             const QVector2D &inWindowDimensions,
                                             bool inForceIntersect) const;

    // Checks the various viewports and determines if the layer is visible or not.
    bool isLayerVisible() const;

private:
    // Viewport used when actually rendering.  In the case where this is an offscreen item then
    // it may be
    // different than the layer to presentation viewport.
    QDemonRenderRectF getLayerRenderViewport() const;
};
QT_END_NAMESPACE

#endif // QDEMON_RENDER_LAYER_HELPER_IMPL_H
