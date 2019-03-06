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
#ifndef QDEMON_RENDER_RESOURCE_BUFFER_OBJECTS_H
#define QDEMON_RENDER_RESOURCE_BUFFER_OBJECTS_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>

#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>

QT_BEGIN_NAMESPACE
class QDemonResourceFrameBuffer
{
protected:
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QDemonRef<QDemonRenderFrameBuffer> m_frameBuffer;

public:
    QDemonResourceFrameBuffer(const QDemonRef<QDemonResourceManagerInterface> &mgr);
    ~QDemonResourceFrameBuffer();
    bool ensureFrameBuffer();
    void releaseFrameBuffer();

    QDemonRef<QDemonResourceManagerInterface> getResourceManager() { return m_resourceManager; }
    QDemonRef<QDemonRenderFrameBuffer> getFrameBuffer() { return m_frameBuffer; }
    operator QDemonRef<QDemonRenderFrameBuffer>() { return m_frameBuffer; }
    QDemonRef<QDemonRenderFrameBuffer> operator->()
    {
        Q_ASSERT(m_frameBuffer);
        return m_frameBuffer;
    }
    QDemonRenderFrameBuffer &operator*()
    {
        Q_ASSERT(m_frameBuffer);
        return *m_frameBuffer;
    }
};

class QDemonResourceRenderBuffer
{
protected:
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QDemonRef<QDemonRenderRenderBuffer> m_renderBuffer;
    QDemonRenderRenderBufferFormat m_storageFormat;
    QDemonRenderRenderBufferDimensions m_dimensions;

public:
    QDemonResourceRenderBuffer(const QDemonRef<QDemonResourceManagerInterface> &mgr);
    ~QDemonResourceRenderBuffer();
    bool ensureRenderBuffer(qint32 width, qint32 height, QDemonRenderRenderBufferFormat storageFormat);
    void releaseRenderBuffer();

    operator QDemonRef<QDemonRenderRenderBuffer>() { return m_renderBuffer; }
    QDemonRef<QDemonRenderRenderBuffer> operator->()
    {
        Q_ASSERT(m_renderBuffer);
        return m_renderBuffer;
    }
    QDemonRenderRenderBuffer &operator*()
    {
        Q_ASSERT(m_renderBuffer);
        return *m_renderBuffer;
    }
};
QT_END_NAMESPACE
#endif
