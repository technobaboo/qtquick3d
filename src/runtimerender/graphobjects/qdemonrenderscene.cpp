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

#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

QDemonRenderScene::QDemonRenderScene()
    : QDemonGraphObject(QDemonGraphObjectTypes::Scene)
    , presentation(nullptr)
    , firstChild(nullptr)
    , clearColor(0, 0, 0)
    , useClearColor(true)
    , dirty(true)
{
}

void QDemonRenderScene::addChild(QDemonRenderLayer &inLayer)
{
    if (firstChild == nullptr)
        firstChild = &inLayer;
    else
        getLastChild()->nextSibling = &inLayer;
    inLayer.scene = this;
}

void QDemonRenderScene:: removeChild(QDemonRenderLayer &inLayer)
{
    QDemonRenderLayer *child;
    for (child = firstChild; child && child->nextSibling; child = static_cast<QDemonRenderLayer *>(child->nextSibling)) {
        if (child == &inLayer) {
            if (child->previousSibling)
                child->previousSibling->nextSibling = child->nextSibling;
            if (child->nextSibling)
                child->nextSibling->previousSibling = child->previousSibling;
            child->parent = nullptr;
            if (firstChild == child)
                firstChild = static_cast<QDemonRenderLayer *>(child->nextSibling);
            child->nextSibling = nullptr;
            child->previousSibling = nullptr;
            return;
        }
    }
}

QDemonRenderLayer *QDemonRenderScene::getLastChild()
{
    // empty loop intentional
    QDemonRenderLayer *child;
    for (child = firstChild; child && child->nextSibling; child = static_cast<QDemonRenderLayer *>(child->nextSibling)) {
    }

    return child;
}

bool QDemonRenderScene::prepareForRender(const QVector2D &inViewportDimensions,
                                         QDemonRenderContextInterface *inContext,
                                         const QDemonRenderInstanceId id)
{
    // We need to iterate through the layers in reverse order and ask them to render.
    bool wasDirty = dirty;
    dirty = false;
    if (firstChild)
        wasDirty |= inContext->getRenderer()->prepareLayerForRender(*firstChild, inViewportDimensions, true, id);
    return wasDirty;
}

void QDemonRenderScene::render(const QVector2D &inViewportDimensions,
                               QDemonRenderContextInterface *inContext,
                               RenderClearCommand inClearColorBuffer,
                               const QDemonRenderInstanceId id)
{
    if ((inClearColorBuffer == QDemonRenderScene::ClearIsOptional && useClearColor)
        || inClearColorBuffer == QDemonRenderScene::AlwaysClear) {
        float clearColorAlpha = inContext->isInSubPresentation() && !useClearColor ? 0.0f : 1.0f;
        QVector4D clearColor(0.0f, 0.0f, 0.0f, clearColorAlpha);
        if (useClearColor) {
            clearColor.setX(clearColor.x());
            clearColor.setY(clearColor.y());
            clearColor.setZ(clearColor.z());
        }
        // Maybe clear and reset to previous clear color after we leave.
        QDemonRenderContextScopedProperty<QVector4D> __clearColor(*inContext->getRenderContext(),
                                                                  &QDemonRenderContext::getClearColor,
                                                                  &QDemonRenderContext::setClearColor,
                                                                  clearColor);
        inContext->getRenderContext()->clear(QDemonRenderClearValues::Color);
    }
    if (firstChild)
        inContext->getRenderer()->renderLayer(*firstChild, inViewportDimensions, useClearColor, clearColor, true, id);
}
void QDemonRenderScene::renderWithClear(const QVector2D &inViewportDimensions,
                                        QDemonRenderContextInterface *inContext,
                                        RenderClearCommand inClearColorBuffer,
                                        QVector3D inClearColor,
                                        const QDemonRenderInstanceId id)
{
    // If this scene is not using clear color, we set the color
    // to background color from parent layer. This allows
    // fully transparent subpresentations (both scene and layer(s) transparent)
    // to inherit color from the layer that contains them.
    if (!useClearColor) {
        clearColor = inClearColor;
        useClearColor = true;
    }
    render(inViewportDimensions, inContext, inClearColorBuffer, id);
}

QT_END_NAMESPACE
