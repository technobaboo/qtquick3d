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
#ifndef QDEMON_RENDER_RASTERIZER_STATE_H
#define QDEMON_RENDER_RASTERIZER_STATE_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

// currently this handles only stencil state
class QDemonRenderRasterizerState
{

private:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject m_StateHandle; ///< opaque backend handle

public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to context
         * @param[in] fnd			Pointer to foundation
         * @param[in] depthBias		depth bias
         * @param[in] depthScale	depth multiplicator
         * @param[in] cullFace		which face to cull front or back
         *
         * @return No return.
         */
    QDemonRenderRasterizerState(QDemonRenderContextImpl &context, float depthBias, float depthScale, QDemonRenderFaces::Enum cullFace);

    virtual ~QDemonRenderRasterizerState();

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    virtual QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject GetRasterizerObjectHandle()
    {
        return m_StateHandle;
    }

    static QDemonRenderRasterizerState *Create(QDemonRenderContextImpl &context, float depthBias,
                                               float depthScale, QDemonRenderFaces::Enum cullFace);

};

QT_END_NAMESPACE

#endif
