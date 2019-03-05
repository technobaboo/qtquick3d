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
#ifndef QDEMON_RENDER__RENDER_RENDER_BUFFER_H
#define QDEMON_RENDER__RENDER_RENDER_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

struct QDemonRenderRenderBufferDimensions
{
    quint32 m_width = 0; ///< buffer width
    quint32 m_height = 0; ///< buffer height

    QDemonRenderRenderBufferDimensions(quint32 w, quint32 h) : m_width(w), m_height(h) {}
    QDemonRenderRenderBufferDimensions() = default;
};

class QDemonRenderRenderBuffer : public QDemonRenderImplemented
{
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    quint32 m_width; ///< buffer width
    quint32 m_height; ///< buffer height
    QDemonRenderRenderBufferFormats::Enum m_storageFormat; ///< buffer storage format

    QDemonRenderBackend::QDemonRenderBackendRenderbufferObject m_bufferHandle; ///< opaque backend handle

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] fnd			Pointer to foundation
     * @param[in] format		Renderbuffer format
     * @param[in] width			Renderbuffer width
     * @param[in] height		Renderbuffer height
     *
     * @return No return.
     */
    QDemonRenderRenderBuffer(const QDemonRef<QDemonRenderContextImpl> &context,
                             QDemonRenderRenderBufferFormats::Enum format,
                             quint32 width,
                             quint32 height);

    /// destructor
    virtual ~QDemonRenderRenderBuffer();

    /**
     * @brief query buffer format
     *
     *
     * @return buffer format
     */
    virtual QDemonRenderRenderBufferFormats::Enum getStorageFormat() const { return m_storageFormat; }

    /**
     * @brief query buffer dimension
     *
     *
     * @return QDemonRenderRenderBufferDimensions object
     */
    virtual QDemonRenderRenderBufferDimensions getDimensions() const
    {
        return QDemonRenderRenderBufferDimensions(m_width, m_height);
    }

    /**
     * @brief constructor
     *
     * @param[in] inDimensions		A dimension object
     *
     * @return buffer format
     */
    virtual void setDimensions(const QDemonRenderRenderBufferDimensions &inDimensions);

    /**
     * @brief static creator function
     *
     * @param[in] context		Pointer to context
     * @param[in] format		Renderbuffer format
     * @param[in] width			Renderbuffer width
     * @param[in] height		Renderbuffer height
     *
     * @return No return.
     */
    static QDemonRef<QDemonRenderRenderBuffer> create(const QDemonRef<QDemonRenderContextImpl> &context,
                                                      QDemonRenderRenderBufferFormats::Enum format,
                                                      quint32 width,
                                                      quint32 height);

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    virtual QDemonRenderBackend::QDemonRenderBackendRenderbufferObject getRenderBuffertHandle()
    {
        return m_bufferHandle;
    }

    // this will be obsolete
    const void *getImplementationHandle() const override { return reinterpret_cast<void *>(m_bufferHandle); }
};

QT_END_NAMESPACE

#endif
