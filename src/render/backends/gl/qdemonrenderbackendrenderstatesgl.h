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
#ifndef QDEMON_RENDER_BACKEND_RENDER_STATE_OBJECTS_GL_H
#define QDEMON_RENDER_BACKEND_RENDER_STATE_OBJECTS_GL_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

///< this class handles the shader input variables
class QDemonRenderBackendDepthStencilStateGL
{
public:
    ///< constructor
    QDemonRenderBackendDepthStencilStateGL(bool enableDepth, bool depthMask,
                                           QDemonRenderBoolOp::Enum depthFunc, bool enableStencil,
                                           QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                           QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                           QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                           QDemonRenderStencilOperationArgument &depthStencilOpBack)
        : m_DepthEnable(enableDepth)
        , m_DepthMask(depthMask)
        , m_DepthFunc(depthFunc)
        , m_StencilEnable(enableStencil)
        , m_StencilFuncFront(stencilFuncFront)
        , m_StencilFuncBack(stencilFuncBack)
        , m_DepthStencilOpFront(depthStencilOpFront)
        , m_DepthStencilOpBack(depthStencilOpBack)
    {
    }

    ///< constructor
    QDemonRenderBackendDepthStencilStateGL()
        : m_DepthEnable(true)
        , m_DepthMask(true)
        , m_DepthFunc(QDemonRenderBoolOp::LessThanOrEqual)
        , m_StencilEnable(false)
    {
    }

    ///< destructor
    ~QDemonRenderBackendDepthStencilStateGL(){}

    ///< assignement
    QDemonRenderBackendDepthStencilStateGL &operator=(const QDemonRenderBackendDepthStencilStateGL &rhs)
    {
        // Check for self-assignment!
        if (this == &rhs)
            return *this;

        m_DepthEnable = rhs.m_DepthEnable;
        m_DepthMask = rhs.m_DepthMask;
        m_DepthFunc = rhs.m_DepthFunc;
        m_StencilEnable = rhs.m_StencilEnable;
        m_StencilFuncFront = rhs.m_StencilFuncFront;
        m_StencilFuncBack = rhs.m_StencilFuncBack;
        m_DepthStencilOpFront = rhs.m_DepthStencilOpFront;
        m_DepthStencilOpBack = rhs.m_DepthStencilOpBack;

        return *this;
    }

    bool operator==(const QDemonRenderBackendDepthStencilStateGL &other) const
    {
        return (m_DepthEnable == other.m_DepthEnable && m_DepthMask == other.m_DepthMask
                && m_DepthFunc == other.m_DepthFunc && m_StencilEnable == other.m_StencilEnable
                && m_StencilFuncFront == other.m_StencilFuncFront
                && m_StencilFuncBack == other.m_StencilFuncBack
                && m_DepthStencilOpFront == other.m_DepthStencilOpFront
                && m_DepthStencilOpBack == other.m_DepthStencilOpBack);
    }

    bool m_DepthEnable; ///< depth test enabled
    bool m_DepthMask; ///< enable / disable depth writes
    QDemonRenderBoolOp::Enum m_DepthFunc; ///< depth comparison func
    bool m_StencilEnable; ///< enable disable stencil test
    QDemonRenderStencilFunctionArgument m_StencilFuncFront; ///< stencil setup for front faces
    QDemonRenderStencilFunctionArgument m_StencilFuncBack; ///< stencil setup for back faces
    QDemonRenderStencilOperationArgument
    m_DepthStencilOpFront; ///< depth stencil operation for front faces
    QDemonRenderStencilOperationArgument
    m_DepthStencilOpBack; ///< depth stencil operation for back faces
};

class QDemonRenderBackendMiscStateGL
{
public:
    ///< constructor
    QDemonRenderBackendMiscStateGL()
        : m_PatchVertexCount(1)
    {
    }

    quint32 m_PatchVertexCount; ///< vertex count for a single patch primitive
};

class QDemonRenderBackendRasterizerStateGL
{
public:
    ///< constructor
    QDemonRenderBackendRasterizerStateGL(float depthBias, float depthScale,
                                         QDemonRenderFaces::Enum cullFace)
        : m_DepthBias(depthBias)
        , m_DepthScale(depthScale)
        , m_CullFace(cullFace)
    {
    }
    ///< constructor
    QDemonRenderBackendRasterizerStateGL()
        : m_DepthBias(0.0)
        , m_DepthScale(0.0)
        , m_CullFace(QDemonRenderFaces::Back)
    {
    }

    QDemonRenderBackendRasterizerStateGL &operator=(const QDemonRenderBackendRasterizerStateGL &rhs)
    {
        // Check for self-assignment!
        if (this == &rhs)
            return *this;

        m_DepthBias = rhs.m_DepthBias;
        m_DepthScale = rhs.m_DepthScale;
        m_CullFace = rhs.m_CullFace;

        return *this;
    }

    bool operator==(const QDemonRenderBackendRasterizerStateGL &other) const
    {
        // TODO: Added fuzzy compare to hide warning, but we should make sure if we actuall want this behavior
        // and disable the warning instead.
        return (qFuzzyCompare(m_DepthBias, other.m_DepthBias) && qFuzzyCompare(m_DepthScale, other.m_DepthScale)
                && m_CullFace == other.m_CullFace);
    }

    float m_DepthBias; ///< depth bias
    float m_DepthScale; ///< mulitply constant
    QDemonRenderFaces::Enum m_CullFace; ///< cull face front or back
};

QT_END_NAMESPACE

#endif
