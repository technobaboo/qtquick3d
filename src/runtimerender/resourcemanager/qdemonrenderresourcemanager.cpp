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
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendertexture2darray.h>
#include <qdemonrendertexturecube.h>

QT_BEGIN_NAMESPACE
namespace {

struct SResourceManager : public IResourceManager
{
    QSharedPointer<QDemonRenderContext> m_RenderContext;
    // Complete list of all allocated objects
    QVector<QSharedPointer<QDemonRefCounted>> m_AllocatedObjects;

    QVector<QDemonRenderFrameBuffer *> m_FreeFrameBuffers;
    QVector<QDemonRenderRenderBuffer *> m_FreeRenderBuffers;
    QVector<QDemonRenderTexture2D *> m_FreeTextures;
    QVector<QDemonRenderTexture2DArray *> m_FreeTexArrays;
    QVector<QDemonRenderTextureCube *> m_FreeTexCubes;
    QVector<QDemonRenderImage2D *> m_FreeImages;

    SResourceManager(QDemonRenderContext &ctx)
        : m_RenderContext(ctx)
    {
    }
    virtual ~SResourceManager() {}

    QDemonRenderFrameBuffer *AllocateFrameBuffer() override
    {
        if (m_FreeFrameBuffers.empty() == true) {
            QDemonRenderFrameBuffer *newBuffer = m_RenderContext->CreateFrameBuffer();
            m_AllocatedObjects.push_back(newBuffer);
            m_FreeFrameBuffers.push_back(newBuffer);
        }
        QDemonRenderFrameBuffer *retval = m_FreeFrameBuffers.back();
        m_FreeFrameBuffers.pop_back();
        return retval;
    }
    void Release(QDemonRenderFrameBuffer &inBuffer) override
    {
        if (inBuffer.HasAnyAttachment()) {
            // Ensure the framebuffer has no attachments.
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color0,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color1,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color2,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color3,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color4,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color5,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color6,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Color7,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Depth,
                            QDemonRenderTextureOrRenderBuffer());
            inBuffer.Attach(QDemonRenderFrameBufferAttachments::Stencil,
                            QDemonRenderTextureOrRenderBuffer());
            if (m_RenderContext->IsDepthStencilSupported())
                inBuffer.Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                QDemonRenderTextureOrRenderBuffer());
        }
#ifdef _DEBUG
        QVector<QDemonRenderFrameBuffer *>::iterator theFind =
                eastl::find(m_FreeFrameBuffers.begin(), m_FreeFrameBuffers.end(), &inBuffer);
        Q_ASSERT(theFind == m_FreeFrameBuffers.end());
#endif
        m_FreeFrameBuffers.push_back(&inBuffer);
    }

    virtual QDemonRenderRenderBuffer *
    AllocateRenderBuffer(quint32 inWidth, quint32 inHeight,
                         QDemonRenderRenderBufferFormats::Enum inBufferFormat) override
    {
        // Look for one of this specific size and format.
        quint32 existingMatchIdx = m_FreeRenderBuffers.size();
        for (quint32 idx = 0, end = existingMatchIdx; idx < end; ++idx) {
            QDemonRenderRenderBuffer *theBuffer = m_FreeRenderBuffers[idx];
            QDemonRenderRenderBufferDimensions theDims = theBuffer->GetDimensions();
            QDemonRenderRenderBufferFormats::Enum theFormat = theBuffer->GetStorageFormat();
            if (theDims.m_Width == inWidth && theDims.m_Height == inHeight
                    && theFormat == inBufferFormat) {
                // Replace idx with last for efficient erasure (that reorders the vector).
                m_FreeRenderBuffers.replace_with_last(idx);
                return theBuffer;
            } else if (theFormat == inBufferFormat)
                existingMatchIdx = idx;
        }
        // If a specific exact match couldn't be found, just use the buffer with
        // the same format and resize it.
        if (existingMatchIdx < m_FreeRenderBuffers.size()) {
            QDemonRenderRenderBuffer *theBuffer = m_FreeRenderBuffers[existingMatchIdx];
            m_FreeRenderBuffers.replace_with_last(existingMatchIdx);
            theBuffer->SetDimensions(QDemonRenderRenderBufferDimensions(inWidth, inHeight));
            return theBuffer;
        }

        QDemonRenderRenderBuffer *theBuffer =
                m_RenderContext->CreateRenderBuffer(inBufferFormat, inWidth, inHeight);
        m_AllocatedObjects.push_back(theBuffer);
        return theBuffer;
    }
    void Release(QDemonRenderRenderBuffer &inBuffer) override
    {
#ifdef _DEBUG
        QVector<QDemonRenderRenderBuffer *>::iterator theFind =
                eastl::find(m_FreeRenderBuffers.begin(), m_FreeRenderBuffers.end(), &inBuffer);
        Q_ASSERT(theFind == m_FreeRenderBuffers.end());
#endif
        m_FreeRenderBuffers.push_back(&inBuffer);
    }
    QDemonRenderTexture2D *SetupAllocatedTexture(QDemonRenderTexture2D &inTexture)
    {
        inTexture.SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        inTexture.SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return &inTexture;
    }
    QDemonRenderTexture2D *AllocateTexture2D(quint32 inWidth, quint32 inHeight,
                                             QDemonRenderTextureFormats::Enum inTextureFormat,
                                             quint32 inSampleCount, bool immutable) override
    {
        bool inMultisample =
                inSampleCount > 1 && m_RenderContext->AreMultisampleTexturesSupported();
        for (quint32 idx = 0, end = m_FreeTextures.size(); idx < end; ++idx) {
            QDemonRenderTexture2D *theTexture = m_FreeTextures[idx];
            STextureDetails theDetails = theTexture->GetTextureDetails();
            if (theDetails.m_Width == inWidth && theDetails.m_Height == inHeight
                    && inTextureFormat == theDetails.m_Format
                    && theTexture->GetSampleCount() == inSampleCount) {
                m_FreeTextures.replace_with_last(idx);
                return SetupAllocatedTexture(*theTexture);
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
        QDemonRenderTexture2D *theTexture = m_RenderContext->CreateTexture2D();

        if (inMultisample)
            theTexture->SetTextureDataMultisample(inSampleCount, inWidth, inHeight,
                                                  inTextureFormat);
        else if (immutable)
            theTexture->SetTextureStorage(1, inWidth, inHeight, inTextureFormat);
        else
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inTextureFormat);

        m_AllocatedObjects.push_back(theTexture);
        return SetupAllocatedTexture(*theTexture);
    }
    void Release(QDemonRenderTexture2D &inBuffer) override
    {
#ifdef _DEBUG
        QVector<QDemonRenderTexture2D *>::iterator theFind =
                eastl::find(m_FreeTextures.begin(), m_FreeTextures.end(), &inBuffer);
        Q_ASSERT(theFind == m_FreeTextures.end());
#endif
        m_FreeTextures.push_back(&inBuffer);
    }

    QDemonRenderTexture2DArray *AllocateTexture2DArray(quint32 inWidth, quint32 inHeight, quint32 inSlices,
                                                       QDemonRenderTextureFormats::Enum inTextureFormat,
                                                       quint32 inSampleCount) override
    {
        bool inMultisample =
                inSampleCount > 1 && m_RenderContext->AreMultisampleTexturesSupported();
        for (quint32 idx = 0, end = m_FreeTexArrays.size(); idx < end; ++idx) {
            QDemonRenderTexture2DArray *theTexture = m_FreeTexArrays[idx];
            STextureDetails theDetails = theTexture->GetTextureDetails();
            if (theDetails.m_Width == inWidth && theDetails.m_Height == inHeight
                    && theDetails.m_Depth == inSlices && inTextureFormat == theDetails.m_Format
                    && theTexture->GetSampleCount() == inSampleCount) {
                m_FreeTexArrays.replace_with_last(idx);
                theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                return theTexture;
            }
        }

        // else resize an existing texture.  This should be fairly quick at the driver level.
        // note that MSAA textures are not resizable ( in GLES )
        if (!m_FreeTexArrays.empty() && !inMultisample) {
            QDemonRenderTexture2DArray *theTexture = m_FreeTexArrays.back();
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
        QDemonRenderTexture2DArray *theTexture = nullptr;

        if (!inMultisample) {
            theTexture = m_RenderContext->CreateTexture2DArray();
            theTexture->SetTextureData(QDemonDataRef<quint8>(), 0, inWidth, inHeight, inSlices,
                                       inTextureFormat);
        } else {
            // Not supported yet
            return nullptr;
        }

        m_AllocatedObjects.push_back(theTexture);
        theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void Release(QDemonRenderTexture2DArray &inBuffer) override
    {
#ifdef _DEBUG
        QVector<QDemonRenderTexture2DArray *>::iterator theFind =
                eastl::find(m_FreeTexArrays.begin(), m_FreeTexArrays.end(), &inBuffer);
        Q_ASSERT(theFind == m_FreeTexArrays.end());
#endif
        m_FreeTexArrays.push_back(&inBuffer);
    }

    QDemonRenderTextureCube *AllocateTextureCube(quint32 inWidth, quint32 inHeight,
                                                 QDemonRenderTextureFormats::Enum inTextureFormat,
                                                 quint32 inSampleCount) override
    {
        bool inMultisample =
                inSampleCount > 1 && m_RenderContext->AreMultisampleTexturesSupported();
        for (quint32 idx = 0, end = m_FreeTexCubes.size(); idx < end; ++idx) {
            QDemonRenderTextureCube *theTexture = m_FreeTexCubes[idx];
            STextureDetails theDetails = theTexture->GetTextureDetails();
            if (theDetails.m_Width == inWidth && theDetails.m_Height == inHeight
                    && inTextureFormat == theDetails.m_Format
                    && theTexture->GetSampleCount() == inSampleCount) {
                m_FreeTexCubes.replace_with_last(idx);

                theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                return theTexture;
            }
        }

        // else resize an existing texture.  This should be fairly quick at the driver level.
        // note that MSAA textures are not resizable ( in GLES )
        if (!m_FreeTexCubes.empty() && !inMultisample) {
            QDemonRenderTextureCube *theTexture = m_FreeTexCubes.back();
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
        QDemonRenderTextureCube *theTexture = nullptr;

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

        m_AllocatedObjects.push_back(theTexture);
        theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        theTexture->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        return theTexture;
    }

    void Release(QDemonRenderTextureCube &inBuffer) override
    {
#ifdef _DEBUG
        QVector<QDemonRenderTextureCube *>::iterator theFind =
                eastl::find(m_FreeTexCubes.begin(), m_FreeTexCubes.end(), &inBuffer);
        Q_ASSERT(theFind == m_FreeTexCubes.end());
#endif
        m_FreeTexCubes.push_back(&inBuffer);
    }

    QDemonRenderImage2D *AllocateImage2D(QDemonRenderTexture2D *inTexture,
                                         QDemonRenderImageAccessType::Enum inAccess) override
    {
        if (m_FreeImages.empty() == true) {
            QDemonRenderImage2D *newImage = m_RenderContext->CreateImage2D(inTexture, inAccess);
            if (newImage) {
                m_AllocatedObjects.push_back(newImage);
                m_FreeImages.push_back(newImage);
            }
        }

        QDemonRenderImage2D *retval = m_FreeImages.back();
        m_FreeImages.pop_back();

        return retval;
    }

    void Release(QDemonRenderImage2D &inBuffer) override
    {
#ifdef _DEBUG
        QVector<QDemonRenderImage2D *>::iterator theFind =
                eastl::find(m_FreeImages.begin(), m_FreeImages.end(), &inBuffer);
        Q_ASSERT(theFind == m_FreeImages.end());
#endif
        m_FreeImages.push_back(&inBuffer);
    }

    QDemonRenderContext &GetRenderContext() override { return *m_RenderContext; }

    void RemoveObjectAllocation(QDemonRefCounted *obj) {
        for (quint32 idx = 0, end = m_AllocatedObjects.size(); idx < end; ++idx) {
            if (obj == m_AllocatedObjects[idx]) {
                m_AllocatedObjects.replace_with_last(idx);
                break;
            }
        }
    }

    void DestroyFreeSizedResources()
    {
        for (int idx = m_FreeRenderBuffers.size() - 1; idx >= 0; --idx) {
            QDemonRenderRenderBuffer *obj = m_FreeRenderBuffers[idx];
            m_FreeRenderBuffers.replace_with_last(idx);
            RemoveObjectAllocation(obj);
        }
        for (int idx = m_FreeTextures.size() - 1; idx >= 0; --idx) {
            QDemonRenderTexture2D *obj = m_FreeTextures[idx];
            m_FreeTextures.replace_with_last(idx);
            RemoveObjectAllocation(obj);
        }
        for (int idx = m_FreeTexArrays.size() - 1; idx >= 0; --idx) {
            QDemonRenderTexture2DArray *obj = m_FreeTexArrays[idx];
            m_FreeTexArrays.replace_with_last(idx);
            RemoveObjectAllocation(obj);
        }
        for (int idx = m_FreeTexCubes.size() - 1; idx >= 0; --idx) {
            QDemonRenderTextureCube *obj = m_FreeTexCubes[idx];
            m_FreeTexCubes.replace_with_last(idx);
            RemoveObjectAllocation(obj);
        }
    }
};
}

IResourceManager &IResourceManager::CreateResourceManager(QDemonRenderContext &inContext)
{
    return *new SResourceManager(inContext);
}

QT_END_NAMESPACE
