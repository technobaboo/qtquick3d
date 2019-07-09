/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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
    QDemonRenderBackendDepthStencilStateGL(bool enableDepth,
                                           bool depthMask,
                                           QDemonRenderBoolOp depthFunc,
                                           bool enableStencil,
                                           QDemonRenderStencilFunction &stencilFuncFront,
                                           QDemonRenderStencilFunction &stencilFuncBack,
                                           QDemonRenderStencilOperation &depthStencilOpFront,
                                           QDemonRenderStencilOperation &depthStencilOpBack)
        : m_depthEnable(enableDepth)
        , m_depthMask(depthMask)
        , m_depthFunc(depthFunc)
        , m_stencilEnable(enableStencil)
        , m_stencilFuncFront(stencilFuncFront)
        , m_stencilFuncBack(stencilFuncBack)
        , m_depthStencilOpFront(depthStencilOpFront)
        , m_depthStencilOpBack(depthStencilOpBack)
    {
    }

    ///< constructor
    QDemonRenderBackendDepthStencilStateGL()
        : m_depthEnable(true), m_depthMask(true), m_depthFunc(QDemonRenderBoolOp::LessThanOrEqual), m_stencilEnable(false)
    {
    }

    ///< destructor
    ~QDemonRenderBackendDepthStencilStateGL() {}

    ///< assignement
    QDemonRenderBackendDepthStencilStateGL &operator=(const QDemonRenderBackendDepthStencilStateGL &rhs)
    {
        // Check for self-assignment!
        if (this == &rhs)
            return *this;

        m_depthEnable = rhs.m_depthEnable;
        m_depthMask = rhs.m_depthMask;
        m_depthFunc = rhs.m_depthFunc;
        m_stencilEnable = rhs.m_stencilEnable;
        m_stencilFuncFront = rhs.m_stencilFuncFront;
        m_stencilFuncBack = rhs.m_stencilFuncBack;
        m_depthStencilOpFront = rhs.m_depthStencilOpFront;
        m_depthStencilOpBack = rhs.m_depthStencilOpBack;

        return *this;
    }

    bool operator==(const QDemonRenderBackendDepthStencilStateGL &other) const
    {
        return (m_depthEnable == other.m_depthEnable && m_depthMask == other.m_depthMask && m_depthFunc == other.m_depthFunc
                && m_stencilEnable == other.m_stencilEnable && m_stencilFuncFront == other.m_stencilFuncFront
                && m_stencilFuncBack == other.m_stencilFuncBack && m_depthStencilOpFront == other.m_depthStencilOpFront
                && m_depthStencilOpBack == other.m_depthStencilOpBack);
    }

    bool m_depthEnable; ///< depth test enabled
    bool m_depthMask; ///< enable / disable depth writes
    QDemonRenderBoolOp m_depthFunc; ///< depth comparison func
    bool m_stencilEnable; ///< enable disable stencil test
    QDemonRenderStencilFunction m_stencilFuncFront; ///< stencil setup for front faces
    QDemonRenderStencilFunction m_stencilFuncBack; ///< stencil setup for back faces
    QDemonRenderStencilOperation m_depthStencilOpFront; ///< depth stencil operation for front faces
    QDemonRenderStencilOperation m_depthStencilOpBack; ///< depth stencil operation for back faces
};

class QDemonRenderBackendMiscStateGL
{
public:
    ///< constructor
    QDemonRenderBackendMiscStateGL() : m_patchVertexCount(1) {}

    quint32 m_patchVertexCount; ///< vertex count for a single patch primitive
};

class QDemonRenderBackendRasterizerStateGL
{
public:
    ///< constructor
    QDemonRenderBackendRasterizerStateGL(float depthBias, float depthScale, QDemonRenderFace cullFace)
        : m_depthBias(depthBias), m_depthScale(depthScale), m_cullFace(cullFace)
    {
    }
    ///< constructor
    QDemonRenderBackendRasterizerStateGL() : m_depthBias(0.0), m_depthScale(0.0), m_cullFace(QDemonRenderFace::Back) {}

    QDemonRenderBackendRasterizerStateGL &operator=(const QDemonRenderBackendRasterizerStateGL &rhs)
    {
        // Check for self-assignment!
        if (this == &rhs)
            return *this;

        m_depthBias = rhs.m_depthBias;
        m_depthScale = rhs.m_depthScale;
        m_cullFace = rhs.m_cullFace;

        return *this;
    }

    bool operator==(const QDemonRenderBackendRasterizerStateGL &other) const
    {
        // TODO: Added fuzzy compare to hide warning, but we should make sure if we actuall want this behavior
        // and disable the warning instead.
        return (qFuzzyCompare(m_depthBias, other.m_depthBias) && qFuzzyCompare(m_depthScale, other.m_depthScale)
                && m_cullFace == other.m_cullFace);
    }

    float m_depthBias; ///< depth bias
    float m_depthScale; ///< mulitply constant
    QDemonRenderFace m_cullFace; ///< cull face front or back
};

QT_END_NAMESPACE

#endif
