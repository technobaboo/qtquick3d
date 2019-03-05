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

#include "qdemonrenderresourcebufferobjects.h"

QT_BEGIN_NAMESPACE

QDemonResourceFrameBuffer::QDemonResourceFrameBuffer(const QDemonRef<QDemonResourceManagerInterface> &mgr)
    : m_resourceManager(mgr)
{
}

QDemonResourceFrameBuffer::~QDemonResourceFrameBuffer()
{
    releaseFrameBuffer();
}

bool QDemonResourceFrameBuffer::ensureFrameBuffer()
{
    if (!m_frameBuffer) {
        m_frameBuffer = m_resourceManager->allocateFrameBuffer();
        return true;
    }
    return false;
}

void QDemonResourceFrameBuffer::releaseFrameBuffer()
{
    if (m_frameBuffer) {
        m_resourceManager->release(m_frameBuffer);
    }
}

QDemonResourceRenderBuffer::QDemonResourceRenderBuffer(const QDemonRef<QDemonResourceManagerInterface> &mgr)
    : m_resourceManager(mgr)
{
}

QDemonResourceRenderBuffer::~QDemonResourceRenderBuffer()
{
    releaseRenderBuffer();
}

bool QDemonResourceRenderBuffer::ensureRenderBuffer(quint32 width, quint32 height, QDemonRenderRenderBufferFormats::Enum storageFormat)
{
    if (m_renderBuffer == nullptr || m_dimensions.m_width != width || m_dimensions.m_height != height || m_storageFormat != storageFormat) {
        if (m_renderBuffer == nullptr || m_storageFormat != storageFormat) {
            releaseRenderBuffer();
            m_renderBuffer = m_resourceManager->allocateRenderBuffer(width, height, storageFormat);
        } else
            m_renderBuffer->setDimensions(QDemonRenderRenderBufferDimensions(width, height));
        m_dimensions = m_renderBuffer->getDimensions();
        m_storageFormat = m_renderBuffer->getStorageFormat();
        return true;
    }
    return false;
}

void QDemonResourceRenderBuffer::releaseRenderBuffer()
{
    if (m_renderBuffer) {
        m_resourceManager->release(m_renderBuffer);
        m_renderBuffer = nullptr;
    }
}

QT_END_NAMESPACE
