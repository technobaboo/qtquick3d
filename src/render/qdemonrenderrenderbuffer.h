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

#ifndef QDEMON_RENDER__RENDER_RENDER_BUFFER_H
#define QDEMON_RENDER__RENDER_RENDER_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContext;

class Q_DEMONRENDER_EXPORT QDemonRenderRenderBuffer
{
    Q_DISABLE_COPY(QDemonRenderRenderBuffer)
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderContext> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    qint32 m_width; ///< buffer width
    qint32 m_height; ///< buffer height
    QDemonRenderRenderBufferFormat m_storageFormat; ///< buffer storage format

    QDemonRenderBackend::QDemonRenderBackendRenderbufferObject m_handle; ///< opaque backend handle

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
    QDemonRenderRenderBuffer(const QDemonRef<QDemonRenderContext> &context,
                             QDemonRenderRenderBufferFormat format,
                             quint32 width,
                             quint32 height);

    /// destructor
    ~QDemonRenderRenderBuffer();

    /**
     * @brief query buffer format
     *
     *
     * @return buffer format
     */
    QDemonRenderRenderBufferFormat storageFormat() const { return m_storageFormat; }

    /**
     * @brief query buffer dimension
     *
     *
     * @return QDemonRenderRenderBufferDimensions object
     */
    QSize size() const
    {
        return QSize(m_width, m_height);
    }

    /**
     * @brief constructor
     *
     * @param[in] inDimensions		A dimension object
     *
     * @return buffer format
     */
    void setSize(const QSize &inDimensions);

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendRenderbufferObject handle()
    {
        return m_handle;
    }
};

QT_END_NAMESPACE

#endif
