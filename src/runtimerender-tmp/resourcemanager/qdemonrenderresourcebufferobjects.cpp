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
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>

QT_BEGIN_NAMESPACE

/*
        IResourceManager&		m_ResourceManager;
        QDemonRenderFrameBuffer*	m_FrameBuffer;
        */

CResourceFrameBuffer::CResourceFrameBuffer(IResourceManager &mgr)
    : m_ResourceManager(mgr)
    , m_FrameBuffer(nullptr)
{
}

CResourceFrameBuffer::~CResourceFrameBuffer()
{
    ReleaseFrameBuffer();
}

bool CResourceFrameBuffer::EnsureFrameBuffer()
{
    if (!m_FrameBuffer) {
        m_FrameBuffer = m_ResourceManager.AllocateFrameBuffer();
        return true;
    }
    return false;
}

void CResourceFrameBuffer::ReleaseFrameBuffer()
{
    if (m_FrameBuffer) {
        m_ResourceManager.Release(*m_FrameBuffer);
    }
}

CResourceRenderBuffer::CResourceRenderBuffer(IResourceManager &mgr)
    : m_ResourceManager(mgr)
    , m_RenderBuffer(nullptr)
{
}

CResourceRenderBuffer::~CResourceRenderBuffer()
{
    ReleaseRenderBuffer();
}

bool CResourceRenderBuffer::EnsureRenderBuffer(quint32 width, quint32 height,
                                               QDemonRenderRenderBufferFormats::Enum storageFormat)
{
    if (m_RenderBuffer == nullptr || m_Dimensions.m_Width != width || m_Dimensions.m_Height != height
            || m_StorageFormat != storageFormat) {
        if (m_RenderBuffer == nullptr || m_StorageFormat != storageFormat) {
            ReleaseRenderBuffer();
            m_RenderBuffer = m_ResourceManager.AllocateRenderBuffer(width, height, storageFormat);
        } else
            m_RenderBuffer->SetDimensions(
                        QDemonRenderRenderBufferDimensions(width, height));
        m_Dimensions = m_RenderBuffer->GetDimensions();
        m_StorageFormat = m_RenderBuffer->GetStorageFormat();
        return true;
    }
    return false;
}

void CResourceRenderBuffer::ReleaseRenderBuffer()
{
    if (m_RenderBuffer) {
        m_ResourceManager.Release(*m_RenderBuffer);
        m_RenderBuffer = nullptr;
    }
}

QT_END_NAMESPACE
