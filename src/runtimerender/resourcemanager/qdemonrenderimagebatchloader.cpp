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
#include "qdemonrenderimagebatchloader.h"

#include <QtDemon/qdemoninvasivelinkedlist.h>

#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrenderthreadpool.h>
#include <QtDemonRuntimeRender/qdemonrenderimagescaler.h>
#include <QtDemonRuntimeRender/qdemonrenderloadedtexture.h>

#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

namespace {

struct SImageLoaderBatch;

struct SLoadingImage
{
    SImageLoaderBatch *m_Batch;
    QString m_SourcePath;
    quint64 m_TaskId;
    SLoadingImage *m_Tail;

    // Called from main thread
    SLoadingImage(QString inSourcePath)
        : m_Batch(nullptr)
        , m_SourcePath(inSourcePath)
        , m_TaskId(0)
        , m_Tail(nullptr)
    {
    }
    SLoadingImage()
        : m_Batch(nullptr)
        , m_TaskId(0)
        , m_Tail(nullptr)
    {
    }
    // Called from main thread
    void Setup(SImageLoaderBatch &inBatch);

    // Called from loader thread
    static void LoadImage(void *inImg);

    // Potentially called from loader thread
    static void TaskCancelled(void *inImg);
};

struct SLoadingImageTailOp
{
    SLoadingImage *get(SLoadingImage &inImg) { return inImg.m_Tail; }
    void set(SLoadingImage &inImg, SLoadingImage *inItem) { inImg.m_Tail = inItem; }
};

typedef InvasiveSingleLinkedList<SLoadingImage, SLoadingImageTailOp> TLoadingImageList;

struct SBatchLoader;

struct SImageLoaderBatch
{
    // All variables setup in main thread and constant from then on except
    // loaded image count.
    SBatchLoader &m_Loader;
    QSharedPointer<IImageLoadListener> m_LoadListener;
    QWaitCondition m_LoadEvent;
    QMutex m_LoadMutex;
    TLoadingImageList m_Images;

    TImageBatchId m_BatchId;
    // Incremented in main thread
    quint32 m_LoadedOrCanceledImageCount;
    quint32 m_FinalizedImageCount;
    quint32 m_NumImages;
    QDemonRenderContextType m_contextType;

    // Called from main thread
    static SImageLoaderBatch *CreateLoaderBatch(SBatchLoader &inLoader, TImageBatchId inBatchId,
                                                QDemonConstDataRef<QString> inSourcePaths,
                                                QString inImageTillLoaded,
                                                IImageLoadListener *inListener,
                                                QDemonRenderContextType contextType);

    // Called from main thread
    SImageLoaderBatch(SBatchLoader &inLoader,
                      IImageLoadListener *inLoadListener,
                      const TLoadingImageList &inImageList,
                      TImageBatchId inBatchId,
                      quint32 inImageCount,
                      QDemonRenderContextType contextType);

    // Called from main thread
    ~SImageLoaderBatch();

    // Called from main thread
    bool IsLoadingFinished()
    {
        QMutexLocker locker(&m_LoadMutex);
        return m_LoadedOrCanceledImageCount >= m_NumImages;
    }

    bool IsFinalizedFinished()
    {
        QMutexLocker locker(&m_LoadMutex);
        return m_FinalizedImageCount >= m_NumImages;
    }

    void IncrementLoadedImageCount()
    {
        QMutexLocker locker(&m_LoadMutex);
        ++m_LoadedOrCanceledImageCount;
    }
    void IncrementFinalizedImageCount()
    {
        QMutexLocker locker(&m_LoadMutex);
        ++m_FinalizedImageCount;
    }
    // Called from main thread
    void Cancel();
    void Cancel(QString inSourcePath);
};

struct SBatchLoadedImage
{
    QString m_SourcePath;
    QSharedPointer<SLoadedTexture> m_Texture;
    SImageLoaderBatch *m_Batch;
    SBatchLoadedImage()
        : m_Texture(nullptr)
        , m_Batch(nullptr)
    {
    }

    // Called from loading thread
    SBatchLoadedImage(QString inSourcePath, SLoadedTexture *inTexture,
                      SImageLoaderBatch &inBatch)
        : m_SourcePath(inSourcePath)
        , m_Texture(inTexture)
        , m_Batch(&inBatch)
    {
    }

    // Called from main thread
    bool Finalize(IBufferManager &inMgr);
};

struct SBatchLoader : public IImageBatchLoader
{
    typedef QHash<TImageBatchId, SImageLoaderBatch *> TImageLoaderBatchMap;
    typedef QHash<QString, TImageBatchId> TSourcePathToBatchMap;

    // Accessed from loader thread
    QSharedPointer<IInputStreamFactory> m_InputStreamFactory;
    //!!Not threadsafe!  accessed only from main thread
    QSharedPointer<IBufferManager> m_BufferManager;
    // Accessed from main thread
    QSharedPointer<IThreadPool> m_ThreadPool;
    // Accessed from both threads
    QSharedPointer<IPerfTimer> m_PerfTimer;
    // main thread
    TImageBatchId m_NextBatchId;
    // main thread
    TImageLoaderBatchMap m_Batches;
    // main thread
    QMutex m_LoaderMutex;

    // Both loader and main threads
    QVector<SBatchLoadedImage> m_LoadedImages;
    // main thread
    QVector<TImageBatchId> m_FinishedBatches;
    // main thread
    TSourcePathToBatchMap m_SourcePathToBatches;
    // main thread
    QVector<SLoadingImage> m_LoaderBuilderWorkspace;

    SBatchLoader(QSharedPointer<IInputStreamFactory> inFactory,
                 QSharedPointer<IBufferManager> inBufferManager,
                 QSharedPointer<IThreadPool> inThreadPool,
                 QSharedPointer<IPerfTimer> inTimer)
        : m_InputStreamFactory(inFactory)
        , m_BufferManager(inBufferManager)
        , m_ThreadPool(inThreadPool)
        , m_PerfTimer(inTimer)
        , m_NextBatchId(1)
    {
    }

    virtual ~SBatchLoader()
    {
        QVector<TImageBatchId> theCancelledBatches;
        for (TImageLoaderBatchMap::iterator theIter = m_Batches.begin(), theEnd = m_Batches.end();
             theIter != theEnd; ++theIter) {
            theIter.value()->Cancel();
            theCancelledBatches.push_back(theIter.value()->m_BatchId);
        }
        for (quint32 idx = 0, end = theCancelledBatches.size(); idx < end; ++idx)
            BlockUntilLoaded(theCancelledBatches[idx]);

        Q_ASSERT(m_Batches.size() == 0);
    }

    // Returns an ID to the load request.  Request a block of images to be loaded.
    // Also takes an image that the buffer system will return when requested for the given source
    // paths
    // until said path is loaded.
    // An optional listener can be passed in to get callbacks about the batch.
    TImageBatchId LoadImageBatch(QDemonConstDataRef<QString> inSourcePaths,
                                 QString inImageTillLoaded,
                                 IImageLoadListener *inListener,
                                 QDemonRenderContextType contextType) override
    {
        if (inSourcePaths.size() == 0)
            return 0;

        QMutexLocker loaderLock(&m_LoaderMutex);

        TImageBatchId theBatchId = 0;

        // Empty loop intentional to find an unused batch id.
        for (theBatchId = m_NextBatchId; m_Batches.find(theBatchId) != m_Batches.end();
             ++m_NextBatchId, theBatchId = m_NextBatchId) {
        }

        SImageLoaderBatch *theBatch(SImageLoaderBatch::CreateLoaderBatch(
                                        *this, theBatchId, inSourcePaths, inImageTillLoaded, inListener, contextType));
        if (theBatch) {
            m_Batches.insert(theBatchId, theBatch);
            return theBatchId;
        }
        return 0;
    }

    void CancelImageBatchLoading(TImageBatchId inBatchId) override
    {
        SImageLoaderBatch *theBatch(GetBatch(inBatchId));
        if (theBatch)
            theBatch->Cancel();
    }

    // Blocks if the image is currently in-flight
    void CancelImageLoading(QString inSourcePath) override
    {
        QMutexLocker loaderLock(&m_LoaderMutex);
        TSourcePathToBatchMap::iterator theIter = m_SourcePathToBatches.find(inSourcePath);
        if (theIter != m_SourcePathToBatches.end()) {
            TImageBatchId theBatchId = theIter.value();
            TImageLoaderBatchMap::iterator theBatchIter = m_Batches.find(theBatchId);
            if (theBatchIter != m_Batches.end())
                theBatchIter.value()->Cancel(inSourcePath);
        }
    }

    SImageLoaderBatch *GetBatch(TImageBatchId inId)
    {
        QMutexLocker loaderLock(&m_LoaderMutex);
        TImageLoaderBatchMap::iterator theIter = m_Batches.find(inId);
        if (theIter != m_Batches.end())
            return theIter.value();
        return nullptr;
    }

    void BlockUntilLoaded(TImageBatchId inId) override
    {
        // TODO: This is not sane
        QMutexLocker locker(&m_LoaderMutex);
        for (SImageLoaderBatch *theBatch = GetBatch(inId); theBatch; theBatch = GetBatch(inId)) {
            // Only need to block if images aren't loaded.  Don't need to block if they aren't
            // finalized.
            if (!theBatch->IsLoadingFinished()) {
                theBatch->m_LoadEvent.wait(&m_LoaderMutex, 200);
//                theBatch->m_LoadEvent.reset(); ???
            }
            BeginFrame();
        }
    }
    void ImageLoaded(SLoadingImage &inImage, SLoadedTexture *inTexture)
    {
        QMutexLocker loaderLock(&m_LoaderMutex);
        m_LoadedImages.push_back(
                    SBatchLoadedImage(inImage.m_SourcePath, inTexture, *inImage.m_Batch));
        inImage.m_Batch->IncrementLoadedImageCount();
        inImage.m_Batch->m_LoadEvent.wakeAll();
    }
    // These are called by the render context, users don't need to call this.
    void BeginFrame() override
    {
        QMutexLocker loaderLock(&m_LoaderMutex);
        // Pass 1 - send out all image loaded signals
        for (quint32 idx = 0, end = m_LoadedImages.size(); idx < end; ++idx) {

            m_SourcePathToBatches.remove(m_LoadedImages[idx].m_SourcePath);
            m_LoadedImages[idx].Finalize(*m_BufferManager);
            m_LoadedImages[idx].m_Batch->IncrementFinalizedImageCount();
            if (m_LoadedImages[idx].m_Batch->IsFinalizedFinished())
                m_FinishedBatches.push_back(m_LoadedImages[idx].m_Batch->m_BatchId);
        }
        m_LoadedImages.clear();
        // pass 2 - clean up any existing batches.
        for (quint32 idx = 0, end = m_FinishedBatches.size(); idx < end; ++idx) {
            TImageLoaderBatchMap::iterator theIter = m_Batches.find(m_FinishedBatches[idx]);
            if (theIter != m_Batches.end()) {
                SImageLoaderBatch *theBatch = theIter.value();
                if (theBatch->m_LoadListener)
                    theBatch->m_LoadListener->OnImageBatchComplete(theBatch->m_BatchId);
                m_Batches.remove(m_FinishedBatches[idx]);
                theBatch->~SImageLoaderBatch();
            }
        }
        m_FinishedBatches.clear();
    }

    void EndFrame() override {}
};

void SLoadingImage::Setup(SImageLoaderBatch &inBatch)
{
    m_Batch = &inBatch;
    m_TaskId = inBatch.m_Loader.m_ThreadPool->AddTask(this, LoadImage, TaskCancelled);
}

void SLoadingImage::LoadImage(void *inImg)
{
    SLoadingImage *theThis = reinterpret_cast<SLoadingImage *>(inImg);
//    SStackPerfTimer theTimer(theThis->m_Batch->m_Loader.m_PerfTimer, "Image Decompression");
    if (theThis->m_Batch->m_Loader.m_BufferManager->IsImageLoaded(theThis->m_SourcePath) == false) {
        QSharedPointer<SLoadedTexture> theTexture = SLoadedTexture::Load(theThis->m_SourcePath,
                                                                         *theThis->m_Batch->m_Loader.m_InputStreamFactory,
                                                                         true,
                                                                         theThis->m_Batch->m_contextType);
        // if ( theTexture )
        //	theTexture->EnsureMultiplerOfFour( theThis->m_Batch->m_Loader.m_Foundation,
        //theThis->m_SourcePath.c_str() );

        theThis->m_Batch->m_Loader.ImageLoaded(*theThis, theTexture.data());
    } else {
        theThis->m_Batch->m_Loader.ImageLoaded(*theThis, nullptr);
    }
}

void SLoadingImage::TaskCancelled(void *inImg)
{
    SLoadingImage *theThis = reinterpret_cast<SLoadingImage *>(inImg);
    theThis->m_Batch->m_Loader.ImageLoaded(*theThis, nullptr);
}

bool SBatchLoadedImage::Finalize(IBufferManager &inMgr)
{
    if (m_Texture) {
        // PKC : We'll look at the path location to see if the image is in the standard
        // location for IBL light probes or a standard hdr format and decide to generate BSDF
        // miplevels (if the image doesn't have
        // mipmaps of its own that is).
        QString thepath(m_SourcePath);
        bool isIBL = (thepath.contains(".hdr"))
                || (thepath.contains("\\IBL\\"))
                || (thepath.contains("/IBL/"));
        inMgr.LoadRenderImage(m_SourcePath, m_Texture, false, isIBL);
        inMgr.UnaliasImagePath(m_SourcePath);
    }
    if (m_Batch->m_LoadListener)
        m_Batch->m_LoadListener->OnImageLoadComplete(
                    m_SourcePath, m_Texture ? ImageLoadResult::Succeeded : ImageLoadResult::Failed);

    if (m_Texture) {
        //m_Texture->release();
        return true;
    }

    return false;
}

SImageLoaderBatch *
SImageLoaderBatch::CreateLoaderBatch(SBatchLoader &inLoader, TImageBatchId inBatchId,
                                     QDemonConstDataRef<QString> inSourcePaths,
                                     QString inImageTillLoaded,
                                     IImageLoadListener *inListener,
                                     QDemonRenderContextType contextType)
{
    TLoadingImageList theImages;
    quint32 theLoadingImageCount = 0;
    for (quint32 idx = 0, end = inSourcePaths.size(); idx < end; ++idx) {
        QString theSourcePath(inSourcePaths[idx]);

        // TODO: What was the meaning of isValid() (now isEmpty())??
        if (theSourcePath.isEmpty() == false)
            continue;

        if (inLoader.m_BufferManager->IsImageLoaded(theSourcePath))
            continue;

        const auto foundIt = inLoader.m_SourcePathToBatches.find(inSourcePaths[idx]);

        // TODO: This is a bit funky, check if we really need to update the inBatchId...
        inLoader.m_SourcePathToBatches.insert(inSourcePaths[idx], inBatchId);

        // If the loader has already seen this image.
        if (foundIt != inLoader.m_SourcePathToBatches.constEnd())
            continue;

        if (inImageTillLoaded.isEmpty()) {
            // Alias the image so any further requests for this source path will result in
            // the default images (image till loaded).
            bool aliasSuccess =
                    inLoader.m_BufferManager->AliasImagePath(theSourcePath, inImageTillLoaded, true);
            (void)aliasSuccess;
            Q_ASSERT(aliasSuccess);
        }

        // TODO: Yeah... make sure this is cleaned up correctly.
        SLoadingImage *sli = new SLoadingImage(theSourcePath);
        theImages.push_front(*sli);
        ++theLoadingImageCount;
    }
    if (theImages.empty() == false) {
        SImageLoaderBatch *theBatch = new SImageLoaderBatch(inLoader, inListener, theImages, inBatchId, theLoadingImageCount, contextType);
        return theBatch;
    }
    return nullptr;
}

SImageLoaderBatch::SImageLoaderBatch(SBatchLoader &inLoader, IImageLoadListener *inLoadListener,
                                     const TLoadingImageList &inImageList, TImageBatchId inBatchId,
                                     quint32 inImageCount, QDemonRenderContextType contextType)
    : m_Loader(inLoader)
    , m_LoadListener(inLoadListener)
    , m_Images(inImageList)
    , m_BatchId(inBatchId)
    , m_LoadedOrCanceledImageCount(0)
    , m_FinalizedImageCount(0)
    , m_NumImages(inImageCount)
    , m_contextType(contextType)
{
    for (TLoadingImageList::iterator iter = m_Images.begin(), end = m_Images.end(); iter != end;
         ++iter) {
        iter->Setup(*this);
    }
}

SImageLoaderBatch::~SImageLoaderBatch()
{
    for (TLoadingImageList::iterator iter = m_Images.begin(), end = m_Images.end(); iter != end;
         ++iter) {
        TLoadingImageList::iterator temp(iter);
        ++iter;
        delete temp.m_Obj;
    }
}

void SImageLoaderBatch::Cancel()
{
    for (TLoadingImageList::iterator iter = m_Images.begin(), end = m_Images.end(); iter != end;
         ++iter)
        m_Loader.m_ThreadPool->CancelTask(iter->m_TaskId);
}

void SImageLoaderBatch::Cancel(QString inSourcePath)
{
    for (TLoadingImageList::iterator iter = m_Images.begin(), end = m_Images.end(); iter != end;
         ++iter) {
        if (iter->m_SourcePath == inSourcePath) {
            m_Loader.m_ThreadPool->CancelTask(iter->m_TaskId);
            break;
        }
    }
}
}

QSharedPointer<IImageBatchLoader> IImageBatchLoader::CreateBatchLoader(QSharedPointer<IInputStreamFactory> inFactory,
                                                        QSharedPointer<IBufferManager> inBufferManager,
                                                        QSharedPointer<IThreadPool> inThreadPool,
                                                        QSharedPointer<IPerfTimer> inTimer)
{
    return QSharedPointer<IImageBatchLoader>(new SBatchLoader(inFactory, inBufferManager, inThreadPool, inTimer));
}

QT_END_NAMESPACE