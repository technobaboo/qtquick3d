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
class CResourceFrameBuffer
{
protected:
    QSharedPointer<IResourceManager> m_ResourceManager;
    QSharedPointer<QDemonRenderFrameBuffer> m_FrameBuffer;

public:
    CResourceFrameBuffer(QSharedPointer<IResourceManager> mgr);
    ~CResourceFrameBuffer();
    bool EnsureFrameBuffer();
    void ReleaseFrameBuffer();

    QSharedPointer<IResourceManager> GetResourceManager() { return m_ResourceManager; }
    QSharedPointer<QDemonRenderFrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
    operator QSharedPointer<QDemonRenderFrameBuffer> () { return m_FrameBuffer; }
    QSharedPointer<QDemonRenderFrameBuffer> operator->()
    {
        Q_ASSERT(m_FrameBuffer);
        return m_FrameBuffer;
    }
    QDemonRenderFrameBuffer &operator*()
    {
        Q_ASSERT(m_FrameBuffer);
        return *m_FrameBuffer;
    }
};

class CResourceRenderBuffer
{
protected:
    QSharedPointer<IResourceManager> m_ResourceManager;
    QSharedPointer<QDemonRenderRenderBuffer> m_RenderBuffer;
    QDemonRenderRenderBufferFormats::Enum m_StorageFormat;
    QDemonRenderRenderBufferDimensions m_Dimensions;

public:
    CResourceRenderBuffer(QSharedPointer<IResourceManager> mgr);
    ~CResourceRenderBuffer();
    bool EnsureRenderBuffer(quint32 width, quint32 height,
                            QDemonRenderRenderBufferFormats::Enum storageFormat);
    void ReleaseRenderBuffer();

    operator QSharedPointer<QDemonRenderRenderBuffer> () { return m_RenderBuffer; }
    QSharedPointer<QDemonRenderRenderBuffer> operator->()
    {
        Q_ASSERT(m_RenderBuffer);
        return m_RenderBuffer;
    }
    QDemonRenderRenderBuffer &operator*()
    {
        Q_ASSERT(m_RenderBuffer);
        return *m_RenderBuffer;
    }
};
QT_END_NAMESPACE
#endif
