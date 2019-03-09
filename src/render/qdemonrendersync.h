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
#ifndef QDEMON_RENDER_SYNC_H
#define QDEMON_RENDER_SYNC_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContext;
class QDemonRenderBackend;

///< Base class
class Q_DEMONRENDER_EXPORT QDemonRenderSync
{
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendSyncObject m_handle; ///< opaque backend handle

    explicit QDemonRenderSync(const QDemonRef<QDemonRenderContext> &context);
public:
    ~QDemonRenderSync();

    /**
     * @brief Get sync type
     *
     * @return Return query type
     */
    QDemonRenderSyncType syncType() const { return QDemonRenderSyncType::GpuCommandsComplete; }

    /**
     * @brief Create a sync object and place it in command stream.
     *		  Note every syncobject can only be used once.
     *		  This function creates a new sync object on ever call
     *		  and deletes the previous one
     *
     * @return no return.
     */
    void sync();

    /**
     * @brief Wait for a sync to be signaled
     *		  Note this blocks until the sync is signaled
     *
     * @return no return.
     */
    void wait();

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendSyncObject handle() const { return m_handle; }

    /*
     * @brief static creation function
     *
     * @return a sync object on success
     */
    static QDemonRef<QDemonRenderSync> create(const QDemonRef<QDemonRenderContext> &context);
};

QT_END_NAMESPACE

#endif
