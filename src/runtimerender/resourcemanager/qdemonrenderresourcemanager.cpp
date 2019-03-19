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
#include "qdemonrenderresourcemanager.h"

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendertexture2darray.h>
#include <QtDemonRender/qdemonrendertexturecube.h>

QT_BEGIN_NAMESPACE
namespace {

template<typename T>
void replaceWithLast(QVector<T> &vector, int index)
{
    vector[index] = vector.back();
    vector.pop_back();
}

struct QDemonResourceManager : public QDemonResourceManagerInterface
{
    QDemonRef<QDemonRenderContext> renderContext;
    // Complete list of all allocated objects
    //    QVector<QDemonRef<QDemonRefCounted>> m_allocatedObjects;

    QVector<QDemonRef<QDemonRenderFrameBuffer>> freeFrameBuffers;
    QVector<QDemonRef<QDemonRenderRenderBuffer>> freeRenderBuffers;
    QVector<QDemonRef<QDemonRenderTexture2D>> freeTextures;
    QVector<QDemonRef<QDemonRenderTexture2DArray>> freeTexArrays;
    QVector<QDemonRef<QDemonRenderTextureCube>> freeTexCubes;
    QVector<QDemonRef<QDemonRenderImage2D>> freeImages;

    QDemonResourceManager(const QDemonRef<QDemonRenderContext> &ctx) : renderContext(ctx) {}

    ~QDemonResourceManager() override = default;

    QDemonRef<QDemonRenderFrameBuffer> allocateFrameBuffer() override
    {
        if (freeFrameBuffers.empty() == true) {
            auto newBuffer = new QDemonRenderFrameBuffer(renderContext);
            freeFrameBuffers.push_back(newBuffer);
        }
        auto retval = freeFrameBuffers.back();
        freeFrameBuffers.pop_back();
        return retval;
    }
    void release(QDemonRef<QDemonRenderFrameBuffer> inBuffer) override
    {
        if (inBuffer->hasAnyAttachment()) {
            // Ensure the framebuffer has no attachments.
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color1, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color2, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color3, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color4, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color5, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color6, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Color7, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Depth, QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachment::Stencil, QDemonRenderTextureOrRenderBuffer());
            if (renderContext->supportsDepthStencil())
                inBuffer->attach(QDemonRenderFrameBufferAttachment::DepthStencil, QDemonRenderTextureOrRenderBuffer());
        }
#ifdef _DEBUG
        auto theFind = std::find(freeFrameBuffers.begin(), freeFrameBuffers.end(), inBuffer);
        Q_ASSERT(theFind == freeFrameBuffers.end());
#endif
        freeFrameBuffers.push_back(inBuffer);
    }

    QDemonRef<QDemonRenderRenderBuffer> allocateRenderBuffer(qint32 inWidth,
                                                             qint32 inHeight,
                                                             QDemonRenderRenderBufferFormat inBufferFormat) override
    {
        Q_ASSERT(inWidth >= 0 && inHeight >= 0);
        // Look for one of this specific size and format.
        int existingMatchIdx = freeRenderBuffers.size();
        for (int idx = 0, end = existingMatchIdx; idx < end; ++idx) {
            auto theBuffer = freeRenderBuffers[idx];
            QSize theDims = theBuffer->size();
            QDemonRenderRenderBufferFormat theFormat = theBuffer->storageFormat();
            if (theDims.width() == inWidth && theDims.height() == inHeight && theFormat == inBufferFormat) {
                // Replace idx with last for efficient erasure (that reorders the vector).
                replaceWithLast(freeRenderBuffers, idx);
                return theBuffer;
            } else if (theFormat == inBufferFormat)
                existingMatchIdx = idx;
        }
        // If a specific exact match couldn't be found, just use the buffer with
        // the same format and resize it.
        if (existingMatchIdx < freeRenderBuffers.size()) {
            auto theBuffer = freeRenderBuffers[existingMatchIdx];
            replaceWithLast(freeRenderBuffers, existingMatchIdx);
            theBuffer->setSize(QSize(inWidth, inHeight));
            return theBuffer;
        }

        auto theBuffer = new QDemonRenderRenderBuffer(renderContext, inBufferFormat, inWidth, inHeight);
        return theBuffer;
    }
    void release(QDemonRef<QDemonRenderRenderBuffer> inBuffer) override
    {
        freeRenderBuffers.push_back(inBuffer);
    }
    QDemonRef<QDemonRenderTexture2D> setupAllocatedTexture(QDemonRef<QDemonRenderTexture2D> inTexture)
    {
        inTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        inTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return inTexture;
    }
    QDemonRef<QDemonRenderTexture2D> allocateTexture2D(qint32 inWidth,
                                                       qint32 inHeight,
                                                       QDemonRenderTextureFormat inTextureFormat,
                                                       qint32 inSampleCount,
                                                       bool immutable) override
    {
        Q_ASSERT(inWidth >= 0 && inHeight >= 0 && inSampleCount >= 0);
        bool inMultisample = inSampleCount > 1 && renderContext->supportsMultisampleTextures();
        for (qint32 idx = 0, end = freeTextures.size(); idx < end; ++idx) {
            auto theTexture = freeTextures[idx];
            QDemonTextureDetails theDetails = theTexture->textureDetails();
            if (theDetails.width == inWidth && theDetails.height == inHeight && inTextureFormat == theDetails.format
                && theTexture->sampleCount() == inSampleCount) {
                replaceWithLast(freeTextures, idx);
                return setupAllocatedTexture(theTexture);
            }
        }
        // else resize an existing texture.  This is very expensive
        // note that MSAA textures are not resizable ( in GLES )
        /*
        if ( !freeTextures.empty() && !inMultisample )
        {
                QDemonRenderTexture2D* theTexture = freeTextures.back();
                freeTextures.pop_back();

                // note we could re-use a former MSAA texture
                // this causes a entiere destroy of the previous texture object
                theTexture->SetTextureData( QDemonByteRef(), 0, inWidth, inHeight, inTextureFormat
        );

                return SetupAllocatedTexture( *theTexture );
        }*/
        // else create a new texture.
        auto theTexture = new QDemonRenderTexture2D(renderContext);

        if (inMultisample)
            theTexture->setTextureDataMultisample(inSampleCount, inWidth, inHeight, inTextureFormat);
        else if (immutable)
            theTexture->setTextureStorage(1, inWidth, inHeight, inTextureFormat);
        else
            theTexture->setTextureData(QDemonByteView(), 0, inWidth, inHeight, inTextureFormat);

        return setupAllocatedTexture(theTexture);
    }

    void release(QDemonRef<QDemonRenderTexture2D> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeTextures.begin(), freeTextures.end(), inBuffer);
        Q_ASSERT(theFind == freeTextures.end());
#endif
        freeTextures.push_back(inBuffer);
    }

    QDemonRef<QDemonRenderTexture2DArray> allocateTexture2DArray(qint32 inWidth,
                                                                 qint32 inHeight,
                                                                 qint32 inSlices,
                                                                 QDemonRenderTextureFormat inTextureFormat,
                                                                 qint32 inSampleCount) override
    {
        Q_ASSERT(inWidth >= 0 && inHeight >= 0 && inSlices >= 0 && inSampleCount >= 0);
        bool inMultisample = inSampleCount > 1 && renderContext->supportsMultisampleTextures();
        for (int idx = 0, end = freeTexArrays.size(); idx < end; ++idx) {
            auto theTexture = freeTexArrays[idx];
            QDemonTextureDetails theDetails = theTexture->textureDetails();
            if (theDetails.width == inWidth && theDetails.height == inHeight && theDetails.depth == inSlices
                && inTextureFormat == theDetails.format && theTexture->sampleCount() == inSampleCount) {
                replaceWithLast(freeTexArrays, idx);
                theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                return theTexture;
            }
        }

        // else resize an existing texture.  This should be fairly quick at the driver level.
        // note that MSAA textures are not resizable ( in GLES )
        if (!freeTexArrays.empty() && !inMultisample) {
            auto theTexture = freeTexArrays.back();
            freeTexArrays.pop_back();

            // note we could re-use a former MSAA texture
            // this causes a entiere destroy of the previous texture object
            theTexture->setTextureData(QDemonByteRef(), 0, inWidth, inHeight, inSlices, inTextureFormat);
            theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            return theTexture;
        }

        // else create a new texture.
        QDemonRef<QDemonRenderTexture2DArray> theTexture = nullptr;

        if (!inMultisample) {
            theTexture = new QDemonRenderTexture2DArray(renderContext);
            theTexture->setTextureData(QDemonByteRef(), 0, inWidth, inHeight, inSlices, inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void release(QDemonRef<QDemonRenderTexture2DArray> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeTexArrays.begin(), freeTexArrays.end(), inBuffer);
        Q_ASSERT(theFind == freeTexArrays.end());
#endif
        freeTexArrays.push_back(inBuffer);
    }

    QDemonRef<QDemonRenderTextureCube> allocateTextureCube(qint32 inWidth,
                                                           qint32 inHeight,
                                                           QDemonRenderTextureFormat inTextureFormat,
                                                           qint32 inSampleCount) override
    {
        bool inMultisample = inSampleCount > 1 && renderContext->supportsMultisampleTextures();
        for (int idx = 0, end = freeTexCubes.size(); idx < end; ++idx) {
            auto theTexture = freeTexCubes[idx];
            QDemonTextureDetails theDetails = theTexture->textureDetails();
            if (theDetails.width == inWidth && theDetails.height == inHeight && inTextureFormat == theDetails.format
                && theTexture->sampleCount() == inSampleCount) {
                replaceWithLast(freeTexCubes, idx);

                theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                return theTexture;
            }
        }

        // else resize an existing texture.  This should be fairly quick at the driver level.
        // note that MSAA textures are not resizable ( in GLES )
        if (!freeTexCubes.empty() && !inMultisample) {
            auto theTexture = freeTexCubes.back();
            freeTexCubes.pop_back();

            // note we could re-use a former MSAA texture
            // this causes a entire destroy of the previous texture object
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubePosX, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubeNegX, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubePosY, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubeNegY, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubePosZ, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubeNegZ, inWidth, inHeight, inTextureFormat);
            theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            return theTexture;
        }

        // else create a new texture.
        QDemonRef<QDemonRenderTextureCube> theTexture = nullptr;

        if (!inMultisample) {
            theTexture = new QDemonRenderTextureCube(renderContext);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubePosX, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubeNegX, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubePosY, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubeNegY, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubePosZ, inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonByteRef(), 0, QDemonRenderTextureCubeFace::CubeNegZ, inWidth, inHeight, inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void release(QDemonRef<QDemonRenderTextureCube> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeTexCubes.begin(), freeTexCubes.end(), inBuffer);
        Q_ASSERT(theFind == freeTexCubes.end());
#endif
        freeTexCubes.push_back(inBuffer);
    }

    QDemonRef<QDemonRenderImage2D> allocateImage2D(QDemonRef<QDemonRenderTexture2D> inTexture,
                                                   QDemonRenderImageAccessType inAccess) override
    {
        if (freeImages.empty() == true) {
            auto newImage = new QDemonRenderImage2D(renderContext, inTexture, inAccess);
            if (newImage) {
                freeImages.push_back(newImage);
            }
        }

        auto retval = freeImages.back();
        freeImages.pop_back();

        return retval;
    }

    void release(QDemonRef<QDemonRenderImage2D> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeImages.begin(), freeImages.end(), inBuffer);
        Q_ASSERT(theFind == freeImages.end());
#endif
        freeImages.push_back(inBuffer);
    }

    QDemonRef<QDemonRenderContext> getRenderContext() override { return renderContext; }

    void destroyFreeSizedResources()
    {
        for (int idx = freeRenderBuffers.size() - 1; idx >= 0; --idx) {
            auto obj = freeRenderBuffers[idx];
            replaceWithLast(freeRenderBuffers, idx);
        }
        for (int idx = freeTextures.size() - 1; idx >= 0; --idx) {
            auto obj = freeTextures[idx];
            replaceWithLast(freeTextures, idx);
        }
        for (int idx = freeTexArrays.size() - 1; idx >= 0; --idx) {
            auto obj = freeTexArrays[idx];
            replaceWithLast(freeTexArrays, idx);
        }
        for (int idx = freeTexCubes.size() - 1; idx >= 0; --idx) {
            auto obj = freeTexCubes[idx];
            replaceWithLast(freeTexCubes, idx);
        }
    }
};
}

QDemonRef<QDemonResourceManagerInterface> QDemonResourceManagerInterface::createResourceManager(const QDemonRef<QDemonRenderContext> &inContext)
{
    return QDemonRef<QDemonResourceManagerInterface>(new QDemonResourceManager(inContext));
}

QT_END_NAMESPACE
