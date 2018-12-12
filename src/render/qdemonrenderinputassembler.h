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
#ifndef QDEMON_RENDER_INPUT_ASSEMBLER_H
#define QDEMON_RENDER_INPUT_ASSEMBLER_H

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

// forward declarations
class QDemonRenderContextImpl;
class QDemonRenderBackend;
class QDemonRenderAttribLayout;

///< this class handles the vertex attribute layout setup
class Q_DEMONRENDER_EXPORT QDemonRenderInputAssembler
{
public:
    /**
         * @brief constructor
         *
         *	NOTE: The limit for buffers count is currently 16
         *
         * @param[in] context			Pointer to context
         * @param[in] attribLayout		Pointer to QDemonRenderAttribLayout object
         * @param[in] buffers			list of vertex buffers
         * @param[in] indexBuffer		pointer to index buffer. Can be nullptr
         * @param[in] strides			list of strides of the buffer
         * @param[in] offsets			list of offsets into the buffer
         * @param[in] primType			primitive type used for drawing
         * @param[in] patchVertexCount	if primitive is "Patch" this is the vertex count for a
         *single patch
         *
         * @return No return.
         */
    QDemonRenderInputAssembler(QDemonRenderContextImpl &context, QSharedPointer<QDemonRenderAttribLayout> attribLayout,
                               QDemonConstDataRef<QSharedPointer<QDemonRenderVertexBuffer>> buffers,
                               const QSharedPointer<QDemonRenderIndexBuffer> indexBuffer,
                               QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets,
                               QDemonRenderDrawMode::Enum primType = QDemonRenderDrawMode::Triangles,
                               quint32 patchVertexCount = 1);
    ///< destructor
    ~QDemonRenderInputAssembler();

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject GetInputAssemblerHandle() const
    {
        return m_InputAssemblertHandle;
    }

    /**
         * @brief get the attached index buffer
         *
         * @return the index buffer
         */
    const QSharedPointer<QDemonRenderIndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

    /**
         * @brief get the index count of the attached index buffer (if any)
         *
         * @return the index buffer count
         */
    quint32 GetIndexCount() const;

    /**
         * @brief get the vertex count of the buffer
         *		  Note this makes only sense if we have a single
         *		  interleaved buffer
         *
         * @return the vertex buffer count
         */
    quint32 GetVertexCount() const;

    /**
         * @brief get the primitive type used for drawing
         *
         * @return primitive type
         */
    QDemonRenderDrawMode::Enum GetPrimitiveType() const { return m_PrimitiveType; }

    /**
         * @brief set the per vertex patch count
         *
         * @return none
         */
    void SetPatchVertexCount(quint32 count)
    {
        if (count != m_PatchVertexCount) {
            // clamp to 1;
            m_PatchVertexCount = (count == 0) ? 1 : count;
            ;
            m_Backend->SetPatchVertexCount(m_InputAssemblertHandle, m_PatchVertexCount);
        }
    }

private:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend

    QSharedPointer<QDemonRenderAttribLayout> m_AttribLayout; ///< pointer to attribute layout
    QVector<QSharedPointer<QDemonRenderVertexBuffer>> m_VertexBuffers; ///< vertex buffers
    const QSharedPointer<QDemonRenderIndexBuffer> m_IndexBuffer; ///< index buffer
    QDemonConstDataRef<QDemonRenderBackend::QDemonRenderBackendBufferObject> m_VertexbufferHandles; ///< opaque vertex buffer backend handles

    QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject m_InputAssemblertHandle; ///< opaque backend handle
    QDemonRenderDrawMode::Enum m_PrimitiveType; ///< primitive type used for drawing
    quint32 m_PatchVertexCount; ///< vertex count if primitive type is patch
};

QT_END_NAMESPACE

#endif
