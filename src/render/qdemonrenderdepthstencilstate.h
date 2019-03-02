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
#ifndef QDEMON_RENDER_DEPTH_STENCIL_STATE_H
#define QDEMON_RENDER_DEPTH_STENCIL_STATE_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

// currently this handles only stencil state
class QDemonRenderDepthStencilState
{

private:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject m_stateHandle; ///< opaque backend handle

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
    QDemonRenderDepthStencilState(const QDemonRef<QDemonRenderContextImpl> &context,
                                  bool enableDepth,
                                  bool depthMask,
                                  QDemonRenderBoolOp::Enum depthFunc,
                                  bool enableStencil,
                                  QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                  QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                  QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                  QDemonRenderStencilOperationArgument &depthStencilOpBack);

    virtual ~QDemonRenderDepthStencilState();

    ///< various get functions
    const QDemonRenderStencilFunctionArgument getStencilFunc(QDemonRenderFaces::Enum face) const
    {
        return (face == QDemonRenderFaces::Back) ? m_stencilFuncBack : m_stencilFuncFront;
    }
    const QDemonRenderStencilOperationArgument getStencilOp(QDemonRenderFaces::Enum face) const
    {
        return (face == QDemonRenderFaces::Back) ? m_depthStencilOpBack : m_depthStencilOpFront;
    }
    QDemonRenderBoolOp::Enum getDepthFunc() const { return m_depthFunc; }
    bool getDepthEnabled() const { return m_depthEnabled; }
    bool getStencilEnabled() const { return m_stencilEnabled; }
    bool getDepthMask() const { return m_depthMask; }

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    virtual QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject getDepthStencilObjectHandle()
    {
        return m_stateHandle;
    }

    static QDemonRef<QDemonRenderDepthStencilState> create(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                bool enableDepth,
                                                                bool depthMask,
                                                                QDemonRenderBoolOp::Enum depthFunc,
                                                                bool enableStencil,
                                                                QDemonRenderStencilFunctionArgument &stencilFuncFront,
                                                                QDemonRenderStencilFunctionArgument &stencilFuncBack,
                                                                QDemonRenderStencilOperationArgument &depthStencilOpFront,
                                                                QDemonRenderStencilOperationArgument &depthStencilOpBack);

private:
    bool m_depthEnabled; ///< depth test enabled
    bool m_depthMask; ///< depth writes enabled
    QDemonRenderBoolOp::Enum m_depthFunc; ///< depth comparison func
    bool m_stencilEnabled; ///< stencil test enabled
    QDemonRenderStencilFunctionArgument m_stencilFuncFront; ///< stencil setup front faces
    QDemonRenderStencilFunctionArgument m_stencilFuncBack; ///< stencil setup back faces
    QDemonRenderStencilOperationArgument m_depthStencilOpFront; ///< depth stencil operation front faces
    QDemonRenderStencilOperationArgument m_depthStencilOpBack; ///< depth stencil operation back faces
};

QT_END_NAMESPACE

#endif
