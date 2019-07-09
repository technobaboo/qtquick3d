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

#ifndef QDEMON_RENDER_DEPTH_STENCIL_STATE_H
#define QDEMON_RENDER_DEPTH_STENCIL_STATE_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContext;

// currently this handles only stencil state
class Q_DEMONRENDER_EXPORT QDemonRenderDepthStencilState
{
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject m_handle; ///< opaque backend handle

public:
    /**
     * @brief constructor
     *
     * @param[in] context				Pointer to context
     * @param[in] fnd					Pointer to foundation
     * @param[in] enableDepth			enable depth test
     * @param[in] depthMask				enable depth writes
     * @param[in] depthFunc				depth compare function
     * @param[in] enableStencil			enable stencil test
     * @param[in] stencilFuncFront		stencil setup front faces
     * @param[in] stencilFuncBack		stencil setup back faces
     * @param[in] depthStencilOpFront	depth/stencil operations front faces
     * @param[in] depthStencilOpBack	depth/stencil operations back faces
     *
     * @return No return.
     */
    QDemonRenderDepthStencilState(const QDemonRef<QDemonRenderContext> &context,
                                  bool enableDepth,
                                  bool depthMask,
                                  QDemonRenderBoolOp depthFunc,
                                  bool enableStencil,
                                  QDemonRenderStencilFunction &stencilFuncFront,
                                  QDemonRenderStencilFunction &stencilFuncBack,
                                  QDemonRenderStencilOperation &depthStencilOpFront,
                                  QDemonRenderStencilOperation &depthStencilOpBack);

    ~QDemonRenderDepthStencilState();

    ///< various get functions
    const QDemonRenderStencilFunction stencilFunction(QDemonRenderFace face) const
    {
        return (face == QDemonRenderFace::Back) ? m_stencilFuncBack : m_stencilFuncFront;
    }
    const QDemonRenderStencilOperation stencilOperation(QDemonRenderFace face) const
    {
        return (face == QDemonRenderFace::Back) ? m_depthStencilOpBack : m_depthStencilOpFront;
    }
    QDemonRenderBoolOp depthFunction() const { return m_depthFunc; }
    bool depthEnabled() const { return m_depthEnabled; }
    bool stencilEnabled() const { return m_stencilEnabled; }
    bool depthMask() const { return m_depthMask; }

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject handle()
    {
        return m_handle;
    }

private:
    bool m_depthEnabled; ///< depth test enabled
    bool m_depthMask; ///< depth writes enabled
    bool m_stencilEnabled; ///< stencil test enabled
    QDemonRenderBoolOp m_depthFunc; ///< depth comparison func
    QDemonRenderStencilFunction m_stencilFuncFront; ///< stencil setup front faces
    QDemonRenderStencilFunction m_stencilFuncBack; ///< stencil setup back faces
    QDemonRenderStencilOperation m_depthStencilOpFront; ///< depth stencil operation front faces
    QDemonRenderStencilOperation m_depthStencilOpBack; ///< depth stencil operation back faces
};

QT_END_NAMESPACE

#endif
