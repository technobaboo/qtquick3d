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

#include <qdemonrendercontext.h>
#include <qdemonrenderdepthstencilstate.h>

QT_BEGIN_NAMESPACE

QDemonRenderDepthStencilState::QDemonRenderDepthStencilState(
        QDemonRenderContextImpl &context, bool enableDepth, bool depthMask,
        QDemonRenderBoolOp::Enum depthFunc, bool enableStencil,
        QDemonRenderStencilFunctionArgument &stencilFuncFront,
        QDemonRenderStencilFunctionArgument &stencilFuncBack,
        QDemonRenderStencilOperationArgument &depthStencilOpFront,
        QDemonRenderStencilOperationArgument &depthStencilOpBack)
    : m_Context(context)
    , m_Backend(context.GetBackend())
    , m_DepthEnabled(enableDepth)
    , m_DepthMask(depthMask)
    , m_DepthFunc(depthFunc)
    , m_StencilEnabled(enableStencil)
    , m_StencilFuncFront(stencilFuncFront)
    , m_StencilFuncBack(stencilFuncBack)
    , m_DepthStencilOpFront(depthStencilOpFront)
    , m_DepthStencilOpBack(depthStencilOpBack)
{
    // create backend handle
    m_StateHandle = m_Backend->CreateDepthStencilState(
                enableDepth, depthMask, depthFunc, enableStencil, stencilFuncFront, stencilFuncBack,
                depthStencilOpFront, depthStencilOpBack);
}

QDemonRenderDepthStencilState::~QDemonRenderDepthStencilState()
{
    if (m_StateHandle) {
        m_Context.StateDestroyed(*this);
        m_Backend->ReleaseDepthStencilState(m_StateHandle);
    }
}

QDemonRenderDepthStencilState *
QDemonRenderDepthStencilState::Create(QDemonRenderContextImpl &context, bool enableDepth,
                                      bool depthMask, QDemonRenderBoolOp::Enum depthFunc,
                                      bool enableStencil,
                                      QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                      QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                      QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                      QDemonRenderStencilOperationArgument &depthStencilOpBack)
{
    return new QDemonRenderDepthStencilState(context, enableDepth, depthMask, depthFunc, enableStencil,
                stencilFuncFront, stencilFuncBack, depthStencilOpFront, depthStencilOpBack);
}
QT_END_NAMESPACE
