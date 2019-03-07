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

class QDemonRenderContext;

class Q_DEMONRENDER_EXPORT QDemonRenderRenderBuffer
{
    struct Private {
        Q_DISABLE_COPY(Private);
        Private(const QDemonRef<QDemonRenderContext> &context,
                QDemonRenderRenderBufferFormat format);
        ~Private();

        QAtomicInt ref;

        QDemonRef<QDemonRenderContext> context;
        QDemonRef<QDemonRenderBackend> backend;
        QSize size;
        QDemonRenderRenderBufferFormat storageFormat;

        QDemonRenderBackend::QDemonRenderBackendRenderbufferObject handle;
    };
    QExplicitlySharedDataPointer<Private> d;

public:
    QDemonRenderRenderBuffer() = default;
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
                             QDemonRenderRenderBufferFormat format, const QSize &size);

    ~QDemonRenderRenderBuffer();

    /**
     * @brief query buffer format
     *
     *
     * @return buffer format
     */
    QDemonRenderRenderBufferFormat storageFormat() const { return d ? d->storageFormat : QDemonRenderRenderBufferFormat::Unknown; }

    /**
     * @brief query buffer dimension
     *
     *
     * @return QDemonRenderRenderBufferDimensions object
     */
    QSize size() const
    {
        return d ? d->size : QSize();
    }

    bool isNull() const { return !d; }

    void clear() { d = nullptr; }

    /**
     * @brief constructor
     *
     * @param[in] inDimensions		A dimension object
     *
     * @return buffer format
     */
    void setSize(const QSize &size);

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendRenderbufferObject handle()
    {
        return d ? d->handle : nullptr;
    }
};

QT_END_NAMESPACE

#endif
