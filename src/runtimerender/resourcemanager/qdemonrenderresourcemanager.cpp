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
void replaceWithLast(QVector<T> &vector, int index) {
    vector[index] = vector.back();
    vector.pop_back();
}

struct QDemonResourceManager : public QDemonResourceManagerInterface
{
    QSharedPointer<QDemonRenderContext> renderContext;
    // Complete list of all allocated objects
    //    QVector<QSharedPointer<QDemonRefCounted>> m_allocatedObjects;

    QVector<QSharedPointer<QDemonRenderFrameBuffer>> freeFrameBuffers;
    QVector<QSharedPointer<QDemonRenderRenderBuffer>> freeRenderBuffers;
    QVector<QSharedPointer<QDemonRenderTexture2D>> freeTextures;
    QVector<QSharedPointer<QDemonRenderTexture2DArray>> freeTexArrays;
    QVector<QSharedPointer<QDemonRenderTextureCube>> freeTexCubes;
    QVector<QSharedPointer<QDemonRenderImage2D>> freeImages;

    QDemonResourceManager(QSharedPointer<QDemonRenderContext> ctx)
        : renderContext(ctx)
    {

    }

    virtual ~QDemonResourceManager() override
    {

    }

    QSharedPointer<QDemonRenderFrameBuffer> allocateFrameBuffer() override
    {
        if (freeFrameBuffers.empty() == true) {
            auto newBuffer = renderContext->createFrameBuffer();
            freeFrameBuffers.push_back(newBuffer);
        }
        auto retval = freeFrameBuffers.back();
        freeFrameBuffers.pop_back();
        return retval;
    }
    void release(QSharedPointer<QDemonRenderFrameBuffer> inBuffer) override
    {
        if (inBuffer->hasAnyAttachment()) {
            // Ensure the framebuffer has no attachments.
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color0,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color1,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color2,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color3,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color4,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color5,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color6,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Color7,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Depth,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->attach(QDemonRenderFrameBufferAttachments::Stencil,
                             QDemonRenderTextureOrRenderBuffer());
            if (renderContext->isDepthStencilSupported())
                inBuffer->attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 QDemonRenderTextureOrRenderBuffer());
        }
#ifdef _DEBUG
        auto theFind = std::find(freeFrameBuffers.begin(), freeFrameBuffers.end(), inBuffer);
        Q_ASSERT(theFind == freeFrameBuffers.end());
#endif
        freeFrameBuffers.push_back(inBuffer);
    }

    virtual QSharedPointer<QDemonRenderRenderBuffer> allocateRenderBuffer(quint32 inWidth,
                                                                          quint32 inHeight,
                                                                          QDemonRenderRenderBufferFormats::Enum inBufferFormat) override
    {
        // Look for one of this specific size and format.
        int existingMatchIdx = freeRenderBuffers.size();
        for (int idx = 0, end = existingMatchIdx; idx < end; ++idx) {
            auto theBuffer = freeRenderBuffers[idx];
            QDemonRenderRenderBufferDimensions theDims = theBuffer->getDimensions();
            QDemonRenderRenderBufferFormats::Enum theFormat = theBuffer->getStorageFormat();
            if (theDims.m_width == inWidth && theDims.m_height == inHeight
                    && theFormat == inBufferFormat) {
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
            theBuffer->setDimensions(QDemonRenderRenderBufferDimensions(inWidth, inHeight));
            return theBuffer;
        }

        auto theBuffer = renderContext->createRenderBuffer(inBufferFormat, inWidth, inHeight);
        return theBuffer;
    }
    void release(QSharedPointer<QDemonRenderRenderBuffer> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeRenderBuffers.begin(), freeRenderBuffers.end(), inBuffer);
        Q_ASSERT(theFind == freeRenderBuffers.end());
#endif
        freeRenderBuffers.push_back(inBuffer);
    }
    QSharedPointer<QDemonRenderTexture2D> setupAllocatedTexture(QSharedPointer<QDemonRenderTexture2D> inTexture)
    {
        inTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        inTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return inTexture;
    }
    QSharedPointer<QDemonRenderTexture2D> allocateTexture2D(quint32 inWidth, quint32 inHeight,
                                                            QDemonRenderTextureFormats::Enum inTextureFormat,
                                                            quint32 inSampleCount, bool immutable) override
    {
        bool inMultisample =
                inSampleCount > 1 && renderContext->areMultisampleTexturesSupported();
        for (quint32 idx = 0, end = freeTextures.size(); idx < end; ++idx) {
            auto theTexture = freeTextures[idx];
            QDemonTextureDetails theDetails = theTexture->getTextureDetails();
            if (theDetails.width == inWidth && theDetails.height == inHeight
                    && inTextureFormat == theDetails.format
                    && theTexture->getSampleCount() == inSampleCount) {
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
                theTexture->SetTextureData( QDemonDataRef<quint8>(), 0, inWidth, inHeight, inTextureFormat
        );

                return SetupAllocatedTexture( *theTexture );
        }*/
        // else create a new texture.
        auto theTexture = renderContext->createTexture2D();

        if (inMultisample)
            theTexture->setTextureDataMultisample(inSampleCount, inWidth, inHeight,
                                                  inTextureFormat);
        else if (immutable)
            theTexture->setTextureStorage(1, inWidth, inHeight, inTextureFormat);
        else
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inTextureFormat);

        return setupAllocatedTexture(theTexture);
    }

    void release(QSharedPointer<QDemonRenderTexture2D> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeTextures.begin(), freeTextures.end(), inBuffer);
        Q_ASSERT(theFind == freeTextures.end());
#endif
        freeTextures.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderTexture2DArray> allocateTexture2DArray(quint32 inWidth, quint32 inHeight, quint32 inSlices,
                                                                      QDemonRenderTextureFormats::Enum inTextureFormat,
                                                                      quint32 inSampleCount) override
    {
        bool inMultisample = inSampleCount > 1 && renderContext->areMultisampleTexturesSupported();
        for (int idx = 0, end = freeTexArrays.size(); idx < end; ++idx) {
            auto theTexture = freeTexArrays[idx];
            QDemonTextureDetails theDetails = theTexture->getTextureDetails();
            if (theDetails.width == inWidth && theDetails.height == inHeight
                    && theDetails.depth == inSlices && inTextureFormat == theDetails.format
                    && theTexture->getSampleCount() == inSampleCount) {
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
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inSlices,
                                       inTextureFormat);
            theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            return theTexture;
        }

        // else create a new texture.
        QSharedPointer<QDemonRenderTexture2DArray> theTexture = nullptr;

        if (!inMultisample) {
            theTexture = renderContext->createTexture2DArray();
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inSlices,
                                       inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void release(QSharedPointer<QDemonRenderTexture2DArray> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeTexArrays.begin(), freeTexArrays.end(), inBuffer);
        Q_ASSERT(theFind == freeTexArrays.end());
#endif
        freeTexArrays.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderTextureCube> allocateTextureCube(quint32 inWidth, quint32 inHeight,
                                                                QDemonRenderTextureFormats::Enum inTextureFormat,
                                                                quint32 inSampleCount) override
    {
        bool inMultisample =
                inSampleCount > 1 && renderContext->areMultisampleTexturesSupported();
        for (int idx = 0, end = freeTexCubes.size(); idx < end; ++idx) {
            auto theTexture = freeTexCubes[idx];
            QDemonTextureDetails theDetails = theTexture->getTextureDetails();
            if (theDetails.width == inWidth && theDetails.height == inHeight
                    && inTextureFormat == theDetails.format
                    && theTexture->getSampleCount() == inSampleCount) {
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
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosZ,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegZ,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            return theTexture;
        }

        // else create a new texture.
        QSharedPointer<QDemonRenderTextureCube> theTexture = nullptr;

        if (!inMultisample) {
            theTexture = renderContext->createTextureCube();
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosZ,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->setTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegZ,
                                       inWidth, inHeight, inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        theTexture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void release(QSharedPointer<QDemonRenderTextureCube> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeTexCubes.begin(), freeTexCubes.end(), inBuffer);
        Q_ASSERT(theFind == freeTexCubes.end());
#endif
        freeTexCubes.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderImage2D> allocateImage2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                                                        QDemonRenderImageAccessType::Enum inAccess) override
    {
        if (freeImages.empty() == true) {
            auto newImage = renderContext->createImage2D(inTexture, inAccess);
            if (newImage) {
                freeImages.push_back(newImage);
            }
        }

        auto retval = freeImages.back();
        freeImages.pop_back();

        return retval;
    }

    void release(QSharedPointer<QDemonRenderImage2D> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(freeImages.begin(), freeImages.end(), inBuffer);
        Q_ASSERT(theFind == freeImages.end());
#endif
        freeImages.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderContext> getRenderContext() override { return renderContext; }

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

QSharedPointer<QDemonResourceManagerInterface> QDemonResourceManagerInterface::createResourceManager(QSharedPointer<QDemonRenderContext> inContext)
{
    return QSharedPointer<QDemonResourceManagerInterface>(new QDemonResourceManager(inContext));
}

QT_END_NAMESPACE
