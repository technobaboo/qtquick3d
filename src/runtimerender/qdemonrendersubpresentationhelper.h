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
#pragma once
#ifndef QDEMON_RENDER_SUB_PRESENTATION_HELPER_H
#define QDEMON_RENDER_SUB_PRESENTATION_HELPER_H
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

// Small helper object to setup the state needed to render a sub presentation
// correctly.  Sub presentations may have transparency, and if they do then
// then need to be rendered with pre multiple alpha disabled.  If they don't,
// then they need to be rendered with pre-multiply alpha enabled (and have the alpha channel
// set to 1
struct SSubPresentationHelper
{
    QSharedPointer<IQDemonRenderContext> m_RenderContext;
    QSize m_PreviousPresentationDimensions;

    bool m_WasInSubPresentation;

    SSubPresentationHelper(QSharedPointer<IQDemonRenderContext> inContext, const QSize &inPresDimensions)
        : m_RenderContext(inContext)
        , m_PreviousPresentationDimensions(inContext->GetCurrentPresentationDimensions())
        , m_WasInSubPresentation(inContext->IsInSubPresentation())
    {
        m_RenderContext->SetInSubPresentation(true);
        m_RenderContext->SetPresentationDimensions(inPresDimensions);
    }
    ~SSubPresentationHelper()
    {
        m_RenderContext->SetInSubPresentation(m_WasInSubPresentation);
        m_RenderContext->SetPresentationDimensions(m_PreviousPresentationDimensions);
    }
};
QT_END_NAMESPACE
#endif
