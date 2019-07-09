/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#ifndef QDEMON_RENDER_RESOURCE_MANAGER_H
#define QDEMON_RENDER_RESOURCE_MANAGER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

QT_BEGIN_NAMESPACE
/**
 *	Implements simple pooling of render resources
 */
class Q_DEMONRUNTIMERENDER_EXPORT QDemonResourceManager
{
    Q_DISABLE_COPY(QDemonResourceManager)
public:
    QAtomicInt ref;
private:
    QDemonRef<QDemonRenderContext> renderContext;
    // Complete list of all allocated objects
    //    QVector<QDemonRef<QDemonRefCounted>> m_allocatedObjects;

    QVector<QDemonRef<QDemonRenderFrameBuffer>> freeFrameBuffers;
    QVector<QDemonRef<QDemonRenderRenderBuffer>> freeRenderBuffers;
    QVector<QDemonRef<QDemonRenderTexture2D>> freeTextures;
    QVector<QDemonRef<QDemonRenderTexture2DArray>> freeTexArrays;
    QVector<QDemonRef<QDemonRenderTextureCube>> freeTexCubes;
    QVector<QDemonRef<QDemonRenderImage2D>> freeImages;

    QDemonRef<QDemonRenderTexture2D> setupAllocatedTexture(QDemonRef<QDemonRenderTexture2D> inTexture);

public:
    QDemonResourceManager(const QDemonRef<QDemonRenderContext> &ctx);
    ~QDemonResourceManager();

    QDemonRef<QDemonRenderFrameBuffer> allocateFrameBuffer();
    void release(QDemonRef<QDemonRenderFrameBuffer> inBuffer);
    QDemonRef<QDemonRenderRenderBuffer> allocateRenderBuffer(qint32 inWidth,
                                                                     qint32 inHeight,
                                                                     QDemonRenderRenderBufferFormat inBufferFormat);
    void release(QDemonRef<QDemonRenderRenderBuffer> inBuffer);
    QDemonRef<QDemonRenderTexture2D> allocateTexture2D(qint32 inWidth,
                                                               qint32 inHeight,
                                                               QDemonRenderTextureFormat inTextureFormat,
                                                               qint32 inSampleCount = 1,
                                                               bool immutable = false);
    void release(QDemonRef<QDemonRenderTexture2D> inBuffer);
    QDemonRef<QDemonRenderTexture2DArray> allocateTexture2DArray(qint32 inWidth,
                                                                         qint32 inHeight,
                                                                         qint32 inSlices,
                                                                         QDemonRenderTextureFormat inTextureFormat,
                                                                         qint32 inSampleCount = 1);
    void release(QDemonRef<QDemonRenderTexture2DArray> inBuffer);
    QDemonRef<QDemonRenderTextureCube> allocateTextureCube(qint32 inWidth,
                                                                   qint32 inHeight,
                                                                   QDemonRenderTextureFormat inTextureFormat,
                                                                   qint32 inSampleCount = 1);
    void release(QDemonRef<QDemonRenderTextureCube> inBuffer);
    QDemonRef<QDemonRenderImage2D> allocateImage2D(QDemonRef<QDemonRenderTexture2D> inTexture,
                                                           QDemonRenderImageAccessType inAccess);
    void release(QDemonRef<QDemonRenderImage2D> inBuffer);

    QDemonRef<QDemonRenderContext> getRenderContext();
    void destroyFreeSizedResources();
};

QT_END_NAMESPACE

#endif
