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

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderpathrender.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderpathspecification.h>

QT_BEGIN_NAMESPACE

QDemonRenderPathRender::QDemonRenderPathRender(const QDemonRef<QDemonRenderContext> &context, size_t range)
    : m_context(context), m_backend(context->backend()), m_strokeWidth(0.0f)
{
    m_range = range;
    m_pathRenderHandle = m_backend->createPathNVObject(range);
}

QDemonRenderPathRender::~QDemonRenderPathRender()
{
    if (m_pathRenderHandle) {
        m_backend->releasePathNVObject(m_pathRenderHandle, m_range);
    }
}

void QDemonRenderPathRender::setPathSpecification(const QDemonRef<QDemonRenderPathSpecification> &inCommandBuffer)
{
    m_backend->setPathSpecification(m_pathRenderHandle,
                                    toDataView(inCommandBuffer->getPathCommands()),
                                    toDataView(inCommandBuffer->getPathCoords()));
}

QDemonBounds3 QDemonRenderPathRender::getPathObjectBoundingBox()
{
    return m_backend->getPathObjectBoundingBox(m_pathRenderHandle);
}

QDemonBounds3 QDemonRenderPathRender::getPathObjectFillBox()
{
    return m_backend->getPathObjectFillBox(m_pathRenderHandle);
}

QDemonBounds3 QDemonRenderPathRender::getPathObjectStrokeBox()
{
    return m_backend->getPathObjectStrokeBox(m_pathRenderHandle);
}

void QDemonRenderPathRender::setStrokeWidth(float inStrokeWidth)
{
    if (inStrokeWidth != m_strokeWidth) {
        m_strokeWidth = inStrokeWidth;
        m_backend->setStrokeWidth(m_pathRenderHandle, inStrokeWidth);
    }
}

float QDemonRenderPathRender::getStrokeWidth() const
{
    return m_strokeWidth;
}

void QDemonRenderPathRender::stencilStroke()
{
    m_backend->stencilStrokePath(m_pathRenderHandle);
}

void QDemonRenderPathRender::stencilFill()
{
    m_backend->stencilFillPath(m_pathRenderHandle);
}

QDemonRef<QDemonRenderPathRender> QDemonRenderPathRender::create(const QDemonRef<QDemonRenderContext> &context, size_t range)
{
    if (!context->supportsPathRendering())
        return nullptr;

    return QDemonRef<QDemonRenderPathRender>(new QDemonRenderPathRender(context, range));
}

QT_END_NAMESPACE
