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
#ifndef QDEMON_RENDER_RESOURCE_MANAGER_H
#define QDEMON_RENDER_RESOURCE_MANAGER_H

#include <QtDemon/qdemonrefcounted.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE
/**
     *	Implements simple pooling of render resources
     */
class IResourceManager : public QDemonRefCounted
{
protected:
    virtual ~IResourceManager() {}

public:
    virtual QDemonRenderFrameBuffer *AllocateFrameBuffer() = 0;
    virtual void Release(QDemonRenderFrameBuffer &inBuffer) = 0;
    virtual QDemonRenderRenderBuffer *
    AllocateRenderBuffer(quint32 inWidth, quint32 inHeight,
                         QDemonRenderRenderBufferFormats::Enum inBufferFormat) = 0;
    virtual void Release(QDemonRenderRenderBuffer &inBuffer) = 0;
    virtual QDemonRenderTexture2D *AllocateTexture2D(quint32 inWidth, quint32 inHeight,
                                                     QDemonRenderTextureFormats::Enum inTextureFormat,
                                                     quint32 inSampleCount = 1,
                                                     bool immutable = false) = 0;
    virtual void Release(QDemonRenderTexture2D &inBuffer) = 0;
    virtual QDemonRenderTexture2DArray *
    AllocateTexture2DArray(quint32 inWidth, quint32 inHeight, quint32 inSlices,
                           QDemonRenderTextureFormats::Enum inTextureFormat,
                           quint32 inSampleCount = 1) = 0;
    virtual void Release(QDemonRenderTexture2DArray &inBuffer) = 0;
    virtual QDemonRenderTextureCube *
    AllocateTextureCube(quint32 inWidth, quint32 inHeight,
                        QDemonRenderTextureFormats::Enum inTextureFormat,
                        quint32 inSampleCount = 1) = 0;
    virtual void Release(QDemonRenderTextureCube &inBuffer) = 0;
    virtual QDemonRenderImage2D *AllocateImage2D(QDemonRenderTexture2D *inTexture,
                                                 QDemonRenderImageAccessType::Enum inAccess) = 0;
    virtual void Release(QDemonRenderImage2D &inBuffer) = 0;

    virtual QDemonRenderContext &GetRenderContext() = 0;
    virtual void DestroyFreeSizedResources() = 0;

    static IResourceManager &CreateResourceManager(QDemonRenderContext &inContext);
};
QT_END_NAMESPACE

#endif
