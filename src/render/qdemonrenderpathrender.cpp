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
#include <QtDemonRender/qdemonrenderpathfontspecification.h>

QT_BEGIN_NAMESPACE

QDemonRenderPathRender::QDemonRenderPathRender(QSharedPointer<QDemonRenderContextImpl> context, size_t range)
    : m_Context(context)
    , m_Backend(context->GetBackend())
    , m_StrokeWidth(0.0f)
{
    m_Range = range;
    m_PathRenderHandle = m_Backend->CreatePathNVObject(range);
}

QDemonRenderPathRender::~QDemonRenderPathRender()
{
    if (m_PathRenderHandle) {
        m_Backend->ReleasePathNVObject(m_PathRenderHandle, m_Range);
    }
}

void QDemonRenderPathRender::SetPathSpecification(QSharedPointer<QDemonRenderPathSpecification> inCommandBuffer)
{
    m_Backend->SetPathSpecification(m_PathRenderHandle,
                                    toConstDataRef(inCommandBuffer->GetPathCommands().constData(), inCommandBuffer->GetPathCommands().size()),
                                    toConstDataRef(inCommandBuffer->GetPathCoords().constData(), inCommandBuffer->GetPathCoords().size()));
}

QDemonBounds3 QDemonRenderPathRender::GetPathObjectBoundingBox()
{
    return m_Backend->GetPathObjectBoundingBox(m_PathRenderHandle);
}

QDemonBounds3 QDemonRenderPathRender::GetPathObjectFillBox()
{
    return m_Backend->GetPathObjectFillBox(m_PathRenderHandle);
}

QDemonBounds3 QDemonRenderPathRender::GetPathObjectStrokeBox()
{
    return m_Backend->GetPathObjectStrokeBox(m_PathRenderHandle);
}

void QDemonRenderPathRender::SetStrokeWidth(float inStrokeWidth)
{
    if (inStrokeWidth != m_StrokeWidth) {
        m_StrokeWidth = inStrokeWidth;
        m_Backend->SetStrokeWidth(m_PathRenderHandle, inStrokeWidth);
    }
}

float QDemonRenderPathRender::GetStrokeWidth() const { return m_StrokeWidth; }

void QDemonRenderPathRender::StencilStroke() { m_Backend->StencilStrokePath(m_PathRenderHandle); }

void QDemonRenderPathRender::StencilFill() { m_Backend->StencilFillPath(m_PathRenderHandle); }

QSharedPointer<QDemonRenderPathRender> QDemonRenderPathRender::Create(QSharedPointer<QDemonRenderContextImpl> context, size_t range)
{
    if (!context->IsPathRenderingSupported())
        return nullptr;

    return QSharedPointer<QDemonRenderPathRender>(new QDemonRenderPathRender(context, range));
}

QT_END_NAMESPACE
