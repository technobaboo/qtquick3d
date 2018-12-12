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

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendersync.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

QDemonRenderSync::QDemonRenderSync(QDemonRenderContextImpl &context)
    : m_Context(context)
    , m_Backend(context.GetBackend())
    , m_SyncHandle(nullptr)
{
}

QDemonRenderSync::~QDemonRenderSync()
{
    if (m_SyncHandle)
        m_Backend->ReleaseSync(m_SyncHandle);
}

void QDemonRenderSync::Sync()
{
    // On every sync call we need to create a new sync object
    // A sync object can only be used once

    // First delete the old object
    // We can safely do this because it is actually not deleted until
    // it is unused
    if (m_SyncHandle)
        m_Backend->ReleaseSync(m_SyncHandle);

    m_SyncHandle =
            m_Backend->CreateSync(QDemonRenderSyncType::GpuCommandsComplete, QDemonRenderSyncFlags());
}

void QDemonRenderSync::Wait()
{
    // wait until the sync object is signaled or a timeout happens
    if (m_SyncHandle)
        m_Backend->WaitSync(m_SyncHandle, QDemonRenderCommandFlushFlags(), 0);
}

QSharedPointer<QDemonRenderSync> QDemonRenderSync::Create(QDemonRenderContextImpl &context)
{
    if (!context.IsCommandSyncSupported())
        return nullptr;

    return QSharedPointer<QDemonRenderSync>(new QDemonRenderSync(context));
}

QT_END_NAMESPACE
