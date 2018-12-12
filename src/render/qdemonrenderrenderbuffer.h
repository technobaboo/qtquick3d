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
#ifndef QDEMON_RENDER__RENDER_RENDER_BUFFER_H
#define QDEMON_RENDER__RENDER_RENDER_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

struct QDemonRenderRenderBufferDimensions
{
    quint32 m_Width; ///< buffer width
    quint32 m_Height; ///< buffer height

    QDemonRenderRenderBufferDimensions(quint32 w, quint32 h)
        : m_Width(w)
        , m_Height(h)
    {
    }
    QDemonRenderRenderBufferDimensions()
        : m_Width(0)
        , m_Height(0)
    {
    }
};

class QDemonRenderRenderBuffer : public QDemonRenderImplemented, public QEnableSharedFromThis<QDemonRenderRenderBuffer>
{
private:
    QSharedPointer<QDemonRenderContextImpl> m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    quint32 m_Width; ///< buffer width
    quint32 m_Height; ///< buffer height
    QDemonRenderRenderBufferFormats::Enum m_StorageFormat; ///< buffer storage format

    QDemonRenderBackend::QDemonRenderBackendRenderbufferObject
    m_BufferHandle; ///< opaque backend handle

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
    QDemonRenderRenderBuffer(QSharedPointer<QDemonRenderContextImpl> context,
                             QDemonRenderRenderBufferFormats::Enum format, quint32 width, quint32 height);

    /// destructor
    virtual ~QDemonRenderRenderBuffer();

    /**
         * @brief query buffer format
         *
         *
         * @return buffer format
         */
    virtual QDemonRenderRenderBufferFormats::Enum GetStorageFormat() const
    {
        return m_StorageFormat;
    }

    /**
         * @brief query buffer dimension
         *
         *
         * @return QDemonRenderRenderBufferDimensions object
         */
    virtual QDemonRenderRenderBufferDimensions GetDimensions() const
    {
        return QDemonRenderRenderBufferDimensions(m_Width, m_Height);
    }

    /**
         * @brief constructor
         *
         * @param[in] inDimensions		A dimension object
         *
         * @return buffer format
         */
    virtual void SetDimensions(const QDemonRenderRenderBufferDimensions &inDimensions);

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
    static QSharedPointer<QDemonRenderRenderBuffer> Create(QSharedPointer<QDemonRenderContextImpl> context,
                                            QDemonRenderRenderBufferFormats::Enum format, quint32 width,
                                            quint32 height);

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    virtual QDemonRenderBackend::QDemonRenderBackendRenderbufferObject GetRenderBuffertHandle()
    {
        return m_BufferHandle;
    }

    // this will be obsolete
    const void *GetImplementationHandle() const override
    {
        return reinterpret_cast<void *>(m_BufferHandle);
    }
};

QT_END_NAMESPACE

#endif
