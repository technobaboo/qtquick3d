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
#ifndef QDEMON_RENDER_THREADED_IMAGE_LOADER_H
#define QDEMON_RENDER_THREADED_IMAGE_LOADER_H
#include <Qt3DSRender.h>
#include <QDemonRefCounted>
#include <Qt3DSDataRef.h>
#include <qdemonrenderbasetypes.h>

namespace qt3ds {
namespace render {
    struct ImageLoadResult
    {
        enum Enum {
            Succeeded,
            Failed,
        };
    };

    class IImageLoadListener : public QDemonRefCounted
    {
    protected:
        virtual ~IImageLoadListener() {}

    public:
        virtual void OnImageLoadComplete(CRegisteredString inPath,
                                         ImageLoadResult::Enum inResult) = 0;
        virtual void OnImageBatchComplete(quint64 inBatch) = 0;
    };

    typedef quint32 TImageBatchId;

    class IImageBatchLoader : public QDemonRefCounted
    {
    protected:
        virtual ~IImageBatchLoader() {}

    public:
        // Returns an ID to the load request.  Request a block of images to be loaded.
        // Also takes an image that the buffer system will return when requested for the given
        // source paths
        // until said path is loaded.
        // An optional listener can be passed in to get callbacks about the batch.
        virtual TImageBatchId LoadImageBatch(QDemonConstDataRef<CRegisteredString> inSourcePaths,
                                             CRegisteredString inImageTillLoaded,
                                             IImageLoadListener *inListener,
                                             QDemonRenderContextType type) = 0;
        // Blocks if any of the images in the batch are in flight
        virtual void CancelImageBatchLoading(TImageBatchId inBatchId) = 0;
        // Blocks if the image is currently in-flight
        virtual void CancelImageLoading(CRegisteredString inSourcePath) = 0;
        // Block until every image in the batch is loaded.
        virtual void BlockUntilLoaded(TImageBatchId inId) = 0;

        // These are called by the render context, users don't need to call this.
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        static IImageBatchLoader &CreateBatchLoader(NVFoundationBase &inFoundation,
                                                    IInputStreamFactory &inFactory,
                                                    IBufferManager &inBufferManager,
                                                    IThreadPool &inThreadPool, IPerfTimer &inTimer);
    };
}
}

#endif
