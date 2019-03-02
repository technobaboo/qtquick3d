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

QDemonRenderSync::QDemonRenderSync(const QDemonRef<QDemonRenderContextImpl> &context)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_syncHandle(nullptr)
{
}

QDemonRenderSync::~QDemonRenderSync()
{
    if (m_syncHandle)
        m_backend->releaseSync(m_syncHandle);
}

void QDemonRenderSync::sync()
{
    // On every sync call we need to create a new sync object
    // A sync object can only be used once

    // First delete the old object
    // We can safely do this because it is actually not deleted until
    // it is unused
    if (m_syncHandle)
        m_backend->releaseSync(m_syncHandle);

    m_syncHandle =
            m_backend->createSync(QDemonRenderSyncType::GpuCommandsComplete, QDemonRenderSyncFlags());
}

void QDemonRenderSync::wait()
{
    // wait until the sync object is signaled or a timeout happens
    if (m_syncHandle)
        m_backend->waitSync(m_syncHandle, QDemonRenderCommandFlushFlags(), 0);
}

QDemonRef<QDemonRenderSync> QDemonRenderSync::create(const QDemonRef<QDemonRenderContextImpl> &context)
{
    if (!context->isCommandSyncSupported())
        return nullptr;

    return QDemonRef<QDemonRenderSync>(new QDemonRenderSync(context));
}

QT_END_NAMESPACE
