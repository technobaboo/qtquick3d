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
#ifndef QDEMON_RENDER_INPUT_ASSEMBLER_H
#define QDEMON_RENDER_INPUT_ASSEMBLER_H

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>
#include <QtDemonRender/qdemonrenderattriblayout.h>

QT_BEGIN_NAMESPACE

// forward declarations
class QDemonRenderContext;
class QDemonRenderBackend;
class QDemonRenderAttribLayout;

///< this class handles the vertex attribute layout setup
class Q_DEMONRENDER_EXPORT QDemonRenderInputAssembler
{
    Q_DISABLE_COPY(QDemonRenderInputAssembler)
public:
    QAtomicInt ref;

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
    QDemonRenderInputAssembler(const QDemonRef<QDemonRenderContext> &context,
                               const QDemonRenderAttribLayout &attribLayout,
                               QDemonConstDataRef<QDemonRef<QDemonRenderVertexBuffer>> buffers,
                               const QDemonRef<QDemonRenderIndexBuffer> &indexBuffer,
                               QDemonConstDataRef<quint32> strides,
                               QDemonConstDataRef<quint32> offsets,
                               QDemonRenderDrawMode primType = QDemonRenderDrawMode::Triangles,
                               quint32 patchVertexCount = 1);
    ///< destructor
    ~QDemonRenderInputAssembler();

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject getInputAssemblerHandle() const
    {
        return m_inputAssemblertHandle;
    }

    /**
     * @brief get the attached index buffer
     *
     * @return the index buffer
     */
    const QDemonRef<QDemonRenderIndexBuffer> getIndexBuffer();

    /**
     * @brief get the index count of the attached index buffer (if any)
     *
     * @return the index buffer count
     */
    quint32 getIndexCount() const;

    /**
     * @brief get the vertex count of the buffer
     *		  Note this makes only sense if we have a single
     *		  interleaved buffer
     *
     * @return the vertex buffer count
     */
    quint32 getVertexCount() const;

    /**
     * @brief get the primitive type used for drawing
     *
     * @return primitive type
     */
    QDemonRenderDrawMode getPrimitiveType() const { return m_primitiveType; }

    /**
     * @brief set the per vertex patch count
     *
     * @return none
     */
    void setPatchVertexCount(quint32 count)
    {
        if (count != m_patchVertexCount) {
            // clamp to 1;
            m_patchVertexCount = (count == 0) ? 1 : count;
            ;
            m_backend->setPatchVertexCount(m_inputAssemblertHandle, m_patchVertexCount);
        }
    }

private:
    QDemonRef<QDemonRenderContext> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend

    QDemonRenderAttribLayout m_attribLayout; ///< pointer to attribute layout
    QVector<QDemonRef<QDemonRenderVertexBuffer>> m_vertexBuffers; ///< vertex buffers
    const QDemonRef<QDemonRenderIndexBuffer> m_indexBuffer; ///< index buffer
    QDemonConstDataRef<QDemonRenderBackend::QDemonRenderBackendBufferObject> m_vertexbufferHandles; ///< opaque vertex buffer backend handles

    QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject m_inputAssemblertHandle; ///< opaque backend handle
    QDemonRenderDrawMode m_primitiveType; ///< primitive type used for drawing
    quint32 m_patchVertexCount; ///< vertex count if primitive type is patch
};

QT_END_NAMESPACE

#endif
