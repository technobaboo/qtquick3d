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

struct SResourceManager : public IResourceManager
{
    QSharedPointer<QDemonRenderContext> m_RenderContext;
    // Complete list of all allocated objects
    //    QVector<QSharedPointer<QDemonRefCounted>> m_AllocatedObjects;

    QVector<QSharedPointer<QDemonRenderFrameBuffer>> m_FreeFrameBuffers;
    QVector<QSharedPointer<QDemonRenderRenderBuffer>> m_FreeRenderBuffers;
    QVector<QSharedPointer<QDemonRenderTexture2D>> m_FreeTextures;
    QVector<QSharedPointer<QDemonRenderTexture2DArray>> m_FreeTexArrays;
    QVector<QSharedPointer<QDemonRenderTextureCube>> m_FreeTexCubes;
    QVector<QSharedPointer<QDemonRenderImage2D>> m_FreeImages;

    SResourceManager(QSharedPointer<QDemonRenderContext> ctx)
        : m_RenderContext(ctx)
    {

    }

    virtual ~SResourceManager() override
    {

    }

    QSharedPointer<QDemonRenderFrameBuffer> AllocateFrameBuffer() override
    {
        if (m_FreeFrameBuffers.empty() == true) {
            auto newBuffer = m_RenderContext->CreateFrameBuffer();
            m_FreeFrameBuffers.push_back(newBuffer);
        }
        auto retval = m_FreeFrameBuffers.back();
        m_FreeFrameBuffers.pop_back();
        return retval;
    }
    void Release(QSharedPointer<QDemonRenderFrameBuffer> inBuffer) override
    {
        if (inBuffer->HasAnyAttachment()) {
            // Ensure the framebuffer has no attachments.
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color1,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color2,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color3,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color4,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color5,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color6,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Color7,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Depth,
                             QDemonRenderTextureOrRenderBuffer());
            inBuffer->Attach(QDemonRenderFrameBufferAttachments::Stencil,
                             QDemonRenderTextureOrRenderBuffer());
            if (m_RenderContext->IsDepthStencilSupported())
                inBuffer->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 QDemonRenderTextureOrRenderBuffer());
        }
#ifdef _DEBUG
        auto theFind = std::find(m_FreeFrameBuffers.begin(), m_FreeFrameBuffers.end(), inBuffer);
        Q_ASSERT(theFind == m_FreeFrameBuffers.end());
#endif
        m_FreeFrameBuffers.push_back(inBuffer);
    }

    virtual QSharedPointer<QDemonRenderRenderBuffer>
    AllocateRenderBuffer(quint32 inWidth, quint32 inHeight, QDemonRenderRenderBufferFormats::Enum inBufferFormat) override
    {
        // Look for one of this specific size and format.
        int existingMatchIdx = m_FreeRenderBuffers.size();
        for (int idx = 0, end = existingMatchIdx; idx < end; ++idx) {
            auto theBuffer = m_FreeRenderBuffers[idx];
            QDemonRenderRenderBufferDimensions theDims = theBuffer->GetDimensions();
            QDemonRenderRenderBufferFormats::Enum theFormat = theBuffer->GetStorageFormat();
            if (theDims.m_Width == inWidth && theDims.m_Height == inHeight
                    && theFormat == inBufferFormat) {
                // Replace idx with last for efficient erasure (that reorders the vector).
                replaceWithLast(m_FreeRenderBuffers, idx);
                return theBuffer;
            } else if (theFormat == inBufferFormat)
                existingMatchIdx = idx;
        }
        // If a specific exact match couldn't be found, just use the buffer with
        // the same format and resize it.
        if (existingMatchIdx < m_FreeRenderBuffers.size()) {
            auto theBuffer = m_FreeRenderBuffers[existingMatchIdx];
            replaceWithLast(m_FreeRenderBuffers, existingMatchIdx);
            theBuffer->SetDimensions(QDemonRenderRenderBufferDimensions(inWidth, inHeight));
            return theBuffer;
        }

        auto theBuffer = m_RenderContext->CreateRenderBuffer(inBufferFormat, inWidth, inHeight);
        return theBuffer;
    }
    void Release(QSharedPointer<QDemonRenderRenderBuffer> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(m_FreeRenderBuffers.begin(), m_FreeRenderBuffers.end(), inBuffer);
        Q_ASSERT(theFind == m_FreeRenderBuffers.end());
#endif
        m_FreeRenderBuffers.push_back(inBuffer);
    }
    QSharedPointer<QDemonRenderTexture2D> SetupAllocatedTexture(QSharedPointer<QDemonRenderTexture2D> inTexture)
    {
        inTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        inTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return inTexture;
    }
    QSharedPointer<QDemonRenderTexture2D> AllocateTexture2D(quint32 inWidth, quint32 inHeight,
                                                            QDemonRenderTextureFormats::Enum inTextureFormat,
                                                            quint32 inSampleCount, bool immutable) override
    {
        bool inMultisample =
                inSampleCount > 1 && m_RenderContext->AreMultisampleTexturesSupported();
        for (quint32 idx = 0, end = m_FreeTextures.size(); idx < end; ++idx) {
            auto theTexture = m_FreeTextures[idx];
            STextureDetails theDetails = theTexture->GetTextureDetails();
            if (theDetails.m_Width == inWidth && theDetails.m_Height == inHeight
                    && inTextureFormat == theDetails.m_Format
                    && theTexture->GetSampleCount() == inSampleCount) {
                replaceWithLast(m_FreeTextures, idx);
                return SetupAllocatedTexture(theTexture);
            }
        }
        // else resize an existing texture.  This is very expensive
        // note that MSAA textures are not resizable ( in GLES )
        /*
        if ( !m_FreeTextures.empty() && !inMultisample )
        {
                QDemonRenderTexture2D* theTexture = m_FreeTextures.back();
                m_FreeTextures.pop_back();

                // note we could re-use a former MSAA texture
                // this causes a entiere destroy of the previous texture object
                theTexture->SetTextureData( QDemonDataRef<quint8>(), 0, inWidth, inHeight, inTextureFormat
        );

                return SetupAllocatedTexture( *theTexture );
        }*/
        // else create a new texture.
        auto theTexture = m_RenderContext->CreateTexture2D();

        if (inMultisample)
            theTexture->SetTextureDataMultisample(inSampleCount, inWidth, inHeight,
                                                  inTextureFormat);
        else if (immutable)
            theTexture->SetTextureStorage(1, inWidth, inHeight, inTextureFormat);
        else
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inTextureFormat);

        return SetupAllocatedTexture(theTexture);
    }

    void Release(QSharedPointer<QDemonRenderTexture2D> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(m_FreeTextures.begin(), m_FreeTextures.end(), inBuffer);
        Q_ASSERT(theFind == m_FreeTextures.end());
#endif
        m_FreeTextures.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderTexture2DArray> AllocateTexture2DArray(quint32 inWidth, quint32 inHeight, quint32 inSlices,
                                                                      QDemonRenderTextureFormats::Enum inTextureFormat,
                                                                      quint32 inSampleCount) override
    {
        bool inMultisample = inSampleCount > 1 && m_RenderContext->AreMultisampleTexturesSupported();
        for (int idx = 0, end = m_FreeTexArrays.size(); idx < end; ++idx) {
            auto theTexture = m_FreeTexArrays[idx];
            STextureDetails theDetails = theTexture->GetTextureDetails();
            if (theDetails.m_Width == inWidth && theDetails.m_Height == inHeight
                    && theDetails.m_Depth == inSlices && inTextureFormat == theDetails.m_Format
                    && theTexture->GetSampleCount() == inSampleCount) {
                replaceWithLast(m_FreeTexArrays, idx);
                theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                return theTexture;
            }
        }

        // else resize an existing texture.  This should be fairly quick at the driver level.
        // note that MSAA textures are not resizable ( in GLES )
        if (!m_FreeTexArrays.empty() && !inMultisample) {
            auto theTexture = m_FreeTexArrays.back();
            m_FreeTexArrays.pop_back();

            // note we could re-use a former MSAA texture
            // this causes a entiere destroy of the previous texture object
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inSlices,
                                       inTextureFormat);
            theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            return theTexture;
        }

        // else create a new texture.
        QSharedPointer<QDemonRenderTexture2DArray> theTexture = nullptr;

        if (!inMultisample) {
            theTexture = m_RenderContext->CreateTexture2DArray();
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inSlices,
                                       inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void Release(QSharedPointer<QDemonRenderTexture2DArray> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(m_FreeTexArrays.begin(), m_FreeTexArrays.end(), inBuffer);
        Q_ASSERT(theFind == m_FreeTexArrays.end());
#endif
        m_FreeTexArrays.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderTextureCube> AllocateTextureCube(quint32 inWidth, quint32 inHeight,
                                                                QDemonRenderTextureFormats::Enum inTextureFormat,
                                                                quint32 inSampleCount) override
    {
        bool inMultisample =
                inSampleCount > 1 && m_RenderContext->AreMultisampleTexturesSupported();
        for (int idx = 0, end = m_FreeTexCubes.size(); idx < end; ++idx) {
            auto theTexture = m_FreeTexCubes[idx];
            STextureDetails theDetails = theTexture->GetTextureDetails();
            if (theDetails.m_Width == inWidth && theDetails.m_Height == inHeight
                    && inTextureFormat == theDetails.m_Format
                    && theTexture->GetSampleCount() == inSampleCount) {
                replaceWithLast(m_FreeTexCubes, idx);

                theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                return theTexture;
            }
        }

        // else resize an existing texture.  This should be fairly quick at the driver level.
        // note that MSAA textures are not resizable ( in GLES )
        if (!m_FreeTexCubes.empty() && !inMultisample) {
            auto theTexture = m_FreeTexCubes.back();
            m_FreeTexCubes.pop_back();

            // note we could re-use a former MSAA texture
            // this causes a entire destroy of the previous texture object
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosZ,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegZ,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            return theTexture;
        }

        // else create a new texture.
        QSharedPointer<QDemonRenderTextureCube> theTexture = nullptr;

        if (!inMultisample) {
            theTexture = m_RenderContext->CreateTextureCube();
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegX,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegY,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubePosZ,
                                       inWidth, inHeight, inTextureFormat);
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, QDemonRenderTextureCubeFaces::CubeNegZ,
                                       inWidth, inHeight, inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void Release(QSharedPointer<QDemonRenderTextureCube> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(m_FreeTexCubes.begin(), m_FreeTexCubes.end(), inBuffer);
        Q_ASSERT(theFind == m_FreeTexCubes.end());
#endif
        m_FreeTexCubes.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderImage2D> AllocateImage2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                                                        QDemonRenderImageAccessType::Enum inAccess) override
    {
        if (m_FreeImages.empty() == true) {
            auto newImage = m_RenderContext->CreateImage2D(inTexture, inAccess);
            if (newImage) {
                m_FreeImages.push_back(newImage);
            }
        }

        auto retval = m_FreeImages.back();
        m_FreeImages.pop_back();

        return retval;
    }

    void Release(QSharedPointer<QDemonRenderImage2D> inBuffer) override
    {
#ifdef _DEBUG
        auto theFind = std::find(m_FreeImages.begin(), m_FreeImages.end(), inBuffer);
        Q_ASSERT(theFind == m_FreeImages.end());
#endif
        m_FreeImages.push_back(inBuffer);
    }

    QSharedPointer<QDemonRenderContext> GetRenderContext() override { return m_RenderContext; }

    void DestroyFreeSizedResources()
    {
        for (int idx = m_FreeRenderBuffers.size() - 1; idx >= 0; --idx) {
            auto obj = m_FreeRenderBuffers[idx];
            replaceWithLast(m_FreeRenderBuffers, idx);
        }
        for (int idx = m_FreeTextures.size() - 1; idx >= 0; --idx) {
            auto obj = m_FreeTextures[idx];
            replaceWithLast(m_FreeTextures, idx);
        }
        for (int idx = m_FreeTexArrays.size() - 1; idx >= 0; --idx) {
            auto obj = m_FreeTexArrays[idx];
            replaceWithLast(m_FreeTexArrays, idx);
        }
        for (int idx = m_FreeTexCubes.size() - 1; idx >= 0; --idx) {
            auto obj = m_FreeTexCubes[idx];
            replaceWithLast(m_FreeTexCubes, idx);
        }
    }
};
}

QSharedPointer<IResourceManager> IResourceManager::CreateResourceManager(QSharedPointer<QDemonRenderContext> inContext)
{
    return QSharedPointer<IResourceManager>(new SResourceManager(inContext));
}

QT_END_NAMESPACE
