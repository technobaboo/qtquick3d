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
#ifndef QDEMON_RENDER_BACKEND_INPUT_ASSEMBLER_GL_H
#define QDEMON_RENDER_BACKEND_INPUT_ASSEMBLER_GL_H

#include <QtDemon/QDemonOption>
#include <QtDemon/qdemonutils.h>
#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

struct QDemonRenderBackendLayoutEntryGL
{
    QString m_attribName; ///< must be the same name as used in the vertex shader
    quint8 m_normalize; ///< normalize parameter
    quint32 m_attribIndex; ///< attribute index
    quint32 m_type; ///< GL vertex format type @sa GL_FLOAT, GL_INT
    quint32 m_numComponents; ///< component count. max 4
    quint32 m_inputSlot; ///< Input slot where to fetch the data from
    quint32 m_offset; ///< offset in byte
};

///< this class handles the vertex attribute layout setup
class QDemonRenderBackendAttributeLayoutGL
{
public:
    ///< constructor
    QDemonRenderBackendAttributeLayoutGL(QDemonDataRef<QDemonRenderBackendLayoutEntryGL> entries,
                                         quint32 maxInputSlot)
        : m_layoutAttribEntries(entries)
        , m_maxInputSlot(maxInputSlot)
    {
    }
    ///< destructor
    ~QDemonRenderBackendAttributeLayoutGL(){}

    QDemonRenderBackendLayoutEntryGL *getEntryByName(const QString &entryName) const
    {
        QDEMON_FOREACH(idx, m_layoutAttribEntries.size())
        {
            if (m_layoutAttribEntries[idx].m_attribName == entryName)
                return &m_layoutAttribEntries.mData[idx];
        }
        return nullptr;
    }

    QDemonOption<QDemonRenderBackendLayoutEntryGL> getEntryByAttribIndex(quint32 attribIndex) const
    {
        QDEMON_FOREACH(idx, m_layoutAttribEntries.size())
        {
            if (m_layoutAttribEntries[idx].m_attribIndex == attribIndex)
                return m_layoutAttribEntries[idx];
        }
        return QDemonEmpty();
    }

    QDemonDataRef<QDemonRenderBackendLayoutEntryGL> m_layoutAttribEntries; ///< vertex attribute layout entries
    qint32 m_maxInputSlot; ///< max used input slot
};

///< this class handles the input assembler setup
class QDemonRenderBackendInputAssemblerGL
{
public:
    ///< constructor
    QDemonRenderBackendInputAssemblerGL(
            QDemonRenderBackendAttributeLayoutGL *attribLayout,
            QDemonConstDataRef<QDemonRenderBackend::QDemonRenderBackendBufferObject> buffers,
            const QDemonRenderBackend::QDemonRenderBackendBufferObject indexBuffer,
            QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets, quint32 patchVertexCount)
        : m_attribLayout(attribLayout)
        , m_vertexbufferHandles(buffers)
        , m_indexbufferHandle(indexBuffer)
        , m_vaoID(0)
        , m_cachedShaderHandle(0)
        , m_patchVertexCount(patchVertexCount)
    {
        quint32 *strideMem = static_cast<quint32 *>(::malloc(strides.mSize * sizeof(quint32)));
        quint32 *offsetMem = static_cast<quint32 *>(::malloc(strides.mSize * sizeof(quint32)));
        // copy offsets and strides
        QDEMON_FOREACH(idx, strides.size())
        {
            strideMem[idx] = strides.mData[idx];
            offsetMem[idx] = offsets.mData[idx];
        }

        m_strides = toDataRef(strideMem, strides.size());
        m_offsets = toDataRef(offsetMem, offsets.size());
    }
    ///< destructor
    ~QDemonRenderBackendInputAssemblerGL()
    {
        ::free(m_strides.mData);
        ::free(m_offsets.mData);
    };

    QDemonRenderBackendAttributeLayoutGL *m_attribLayout; ///< pointer to attribute layout
    QDemonConstDataRef<QDemonRenderBackend::QDemonRenderBackendBufferObject> m_vertexbufferHandles; ///< opaque vertex buffer backend handles
    QDemonRenderBackend::QDemonRenderBackendBufferObject m_indexbufferHandle; ///< opaque index buffer backend handles
    quint32 m_vaoID; ///< this is only used if GL version is greater or equal 3
    quint32 m_cachedShaderHandle; ///< this is the shader id which was last used with this object
    quint32 m_patchVertexCount; ///< vertex count for a single patch primitive
    QDemonDataRef<quint32> m_strides; ///< buffer strides
    QDemonDataRef<quint32> m_offsets; ///< buffer offsets
};

QT_END_NAMESPACE

#endif
