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
#ifndef QDEMON_RENDER_RESOURCE_BUFFER_OBJECTS_H
#define QDEMON_RENDER_RESOURCE_BUFFER_OBJECTS_H
#include <qdemonrender.h>
#include <qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <qdemonrenderframebuffer.h>
#include <qdemonrenderrenderbuffer.h>

QT_BEGIN_NAMESPACE
class CResourceFrameBuffer
{
protected:
    IResourceManager &m_ResourceManager;
    QDemonRenderFrameBuffer *m_FrameBuffer;

public:
    CResourceFrameBuffer(IResourceManager &mgr);
    ~CResourceFrameBuffer();
    bool EnsureFrameBuffer();
    void ReleaseFrameBuffer();

    IResourceManager &GetResourceManager() { return m_ResourceManager; }
    operator QDemonRenderFrameBuffer *() { return m_FrameBuffer; }
    QDemonRenderFrameBuffer *operator->()
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
    IResourceManager &m_ResourceManager;
    QDemonRenderRenderBuffer *m_RenderBuffer;
    QDemonRenderRenderBufferFormats::Enum m_StorageFormat;
    QDemonRenderRenderBufferDimensions m_Dimensions;

public:
    CResourceRenderBuffer(IResourceManager &mgr);
    ~CResourceRenderBuffer();
    bool EnsureRenderBuffer(quint32 width, quint32 height,
                            QDemonRenderRenderBufferFormats::Enum storageFormat);
    void ReleaseRenderBuffer();

    operator QDemonRenderRenderBuffer *() { return m_RenderBuffer; }
    QDemonRenderRenderBuffer *operator->()
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
