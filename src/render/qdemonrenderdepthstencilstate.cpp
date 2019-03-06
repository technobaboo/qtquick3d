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

#include <QtDemonRender/qdemonrendercontext.h>
#include <qdemonrenderdepthstencilstate.h>

QT_BEGIN_NAMESPACE

QDemonRenderDepthStencilState::QDemonRenderDepthStencilState(const QDemonRef<QDemonRenderContextImpl> &context,
                                                             bool enableDepth,
                                                             bool depthMask,
                                                             QDemonRenderBoolOp depthFunc,
                                                             bool enableStencil,
                                                             QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                                             QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                                             QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                                             QDemonRenderStencilOperationArgument &depthStencilOpBack)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_depthEnabled(enableDepth)
    , m_depthMask(depthMask)
    , m_depthFunc(depthFunc)
    , m_stencilEnabled(enableStencil)
    , m_stencilFuncFront(stencilFuncFront)
    , m_stencilFuncBack(stencilFuncBack)
    , m_depthStencilOpFront(depthStencilOpFront)
    , m_depthStencilOpBack(depthStencilOpBack)
{
    // create backend handle
    m_stateHandle = m_backend->createDepthStencilState(enableDepth, depthMask, depthFunc, enableStencil, stencilFuncFront, stencilFuncBack, depthStencilOpFront, depthStencilOpBack);
}

QDemonRenderDepthStencilState::~QDemonRenderDepthStencilState()
{
    if (m_stateHandle) {
        m_context->stateDestroyed(this);
        m_backend->releaseDepthStencilState(m_stateHandle);
    }
}

QDemonRef<QDemonRenderDepthStencilState> QDemonRenderDepthStencilState::create(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                               bool enableDepth,
                                                                               bool depthMask,
                                                                               QDemonRenderBoolOp depthFunc,
                                                                               bool enableStencil,
                                                                               QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                                                               QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                                                               QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                                                               QDemonRenderStencilOperationArgument &depthStencilOpBack)
{
    return QDemonRef<QDemonRenderDepthStencilState>(
            new QDemonRenderDepthStencilState(context, enableDepth, depthMask, depthFunc, enableStencil, stencilFuncFront, stencilFuncBack, depthStencilOpFront, depthStencilOpBack));
}
QT_END_NAMESPACE
