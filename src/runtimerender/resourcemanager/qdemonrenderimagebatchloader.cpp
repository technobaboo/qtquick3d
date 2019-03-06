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

struct QDemonImageLoaderBatch;

struct QDemonLoadingImage
{
    QDemonImageLoaderBatch *batch = nullptr;
    QString sourcePath;
    quint64 taskId = 0;
    QDemonLoadingImage *tail = nullptr;

    // Called from main thread
    QDemonLoadingImage(QString inSourcePath) : batch(nullptr), sourcePath(inSourcePath), taskId(0), tail(nullptr) {}

    QDemonLoadingImage() = default;

    // Called from main thread
    void setup(QDemonImageLoaderBatch &inBatch);

    // Called from loader thread
    static void loadImage(void *inImg);

    // Potentially called from loader thread
    static void taskCancelled(void *inImg);
};

struct QDemonLoadingImageTailOp
{
    QDemonLoadingImage *get(QDemonLoadingImage &inImg) { return inImg.tail; }
    void set(QDemonLoadingImage &inImg, QDemonLoadingImage *inItem) { inImg.tail = inItem; }
};

typedef QDemonInvasiveSingleLinkedList<QDemonLoadingImage, QDemonLoadingImageTailOp> TLoadingImageList;

struct QDemonBatchLoader;

struct QDemonImageLoaderBatch
{
    // All variables setup in main thread and constant from then on except
    // loaded image count.
    QDemonBatchLoader &loader;
    QDemonRef<IImageLoadListener> loadListener;
    QWaitCondition loadEvent;
    QMutex loadMutex;
    TLoadingImageList images;

    TImageBatchId batchId;
    // Incremented in main thread
    quint32 loadedOrCanceledImageCount;
    quint32 finalizedImageCount;
    quint32 numImages;
    QDemonRenderContextType contextType;

    // Called from main thread
    static QDemonImageLoaderBatch *createLoaderBatch(QDemonBatchLoader &inLoader,
                                                     TImageBatchId inBatchId,
                                                     QDemonConstDataRef<QString> inSourcePaths,
                                                     QString inImageTillLoaded,
                                                     IImageLoadListener *inListener,
                                                     QDemonRenderContextType contextType);

    // Called from main thread
    QDemonImageLoaderBatch(QDemonBatchLoader &inLoader,
                           IImageLoadListener *inLoadListener,
                           const TLoadingImageList &inImageList,
                           TImageBatchId inBatchId,
                           quint32 inImageCount,
                           QDemonRenderContextType contextType);

    // Called from main thread
    ~QDemonImageLoaderBatch();

    // Called from main thread
    bool isLoadingFinished()
    {
        QMutexLocker locker(&loadMutex);
        return loadedOrCanceledImageCount >= numImages;
    }

    bool isFinalizedFinished()
    {
        QMutexLocker locker(&loadMutex);
        return finalizedImageCount >= numImages;
    }

    void incrementLoadedImageCount()
    {
        QMutexLocker locker(&loadMutex);
        ++loadedOrCanceledImageCount;
    }
    void incrementFinalizedImageCount()
    {
        QMutexLocker locker(&loadMutex);
        ++finalizedImageCount;
    }
    // Called from main thread
    void cancel();
    void cancel(QString inSourcePath);
};

struct QDemonBatchLoadedImage
{
    QString sourcePath;
    QDemonRef<QDemonLoadedTexture> texture;
    QDemonImageLoaderBatch *batch = nullptr;
    QDemonBatchLoadedImage() = default;

    // Called from loading thread
    QDemonBatchLoadedImage(QString inSourcePath, QDemonLoadedTexture *inTexture, QDemonImageLoaderBatch &inBatch)
        : sourcePath(inSourcePath), texture(inTexture), batch(&inBatch)
    {
    }

    // Called from main thread
    bool finalize(QDemonBufferManager &inMgr);
};

struct QDemonBatchLoader : public IImageBatchLoader
{
    typedef QHash<TImageBatchId, QDemonImageLoaderBatch *> TImageLoaderBatchMap;
    typedef QHash<QString, TImageBatchId> TSourcePathToBatchMap;

    // Accessed from loader thread
    QDemonRef<QDemonInputStreamFactoryInterface> inputStreamFactory;
    //!!Not threadsafe!  accessed only from main thread
    QDemonBufferManager bufferManager;
    // Accessed from main thread
    QDemonRef<QDemonAbstractThreadPool> threadPool;
    // Accessed from both threads
    QDemonRef<QDemonPerfTimerInterface> perfTimer;
    // main thread
    TImageBatchId nextBatchId;
    // main thread
    TImageLoaderBatchMap batches;
    // main thread
    QMutex loaderMutex;

    // Both loader and main threads
    QVector<QDemonBatchLoadedImage> loadedImages;
    // main thread
    QVector<TImageBatchId> finishedBatches;
    // main thread
    TSourcePathToBatchMap sourcePathToBatches;
    // main thread
    QVector<QDemonLoadingImage> loaderBuilderWorkspace;

    QDemonBatchLoader(QDemonRef<QDemonInputStreamFactoryInterface> inFactory,
                      QDemonBufferManager inBufferManager,
                      QDemonRef<QDemonAbstractThreadPool> inThreadPool,
                      QDemonRef<QDemonPerfTimerInterface> inTimer)
        : inputStreamFactory(inFactory), bufferManager(inBufferManager), threadPool(inThreadPool), perfTimer(inTimer), nextBatchId(1)
    {
    }

    virtual ~QDemonBatchLoader() override;

    // Returns an ID to the load request.  Request a block of images to be loaded.
    // Also takes an image that the buffer system will return when requested for the given source
    // paths
    // until said path is loaded.
    // An optional listener can be passed in to get callbacks about the batch.
    TImageBatchId loadImageBatch(QDemonConstDataRef<QString> inSourcePaths,
                                 QString inImageTillLoaded,
                                 IImageLoadListener *inListener,
                                 QDemonRenderContextType contextType) override
    {
        if (inSourcePaths.size() == 0)
            return 0;

        QMutexLocker loaderLock(&loaderMutex);

        TImageBatchId theBatchId = 0;

        // Empty loop intentional to find an unused batch id.
        for (theBatchId = nextBatchId; batches.find(theBatchId) != batches.end(); ++nextBatchId, theBatchId = nextBatchId) {
        }

        QDemonImageLoaderBatch *theBatch(
                QDemonImageLoaderBatch::createLoaderBatch(*this, theBatchId, inSourcePaths, inImageTillLoaded, inListener, contextType));
        if (theBatch) {
            batches.insert(theBatchId, theBatch);
            return theBatchId;
        }
        return 0;
    }

    void cancelImageBatchLoading(TImageBatchId inBatchId) override
    {
        QDemonImageLoaderBatch *theBatch(GetBatch(inBatchId));
        if (theBatch)
            theBatch->cancel();
    }

    // Blocks if the image is currently in-flight
    void cancelImageLoading(QString inSourcePath) override
    {
        QMutexLocker loaderLock(&loaderMutex);
        TSourcePathToBatchMap::iterator theIter = sourcePathToBatches.find(inSourcePath);
        if (theIter != sourcePathToBatches.end()) {
            TImageBatchId theBatchId = theIter.value();
            TImageLoaderBatchMap::iterator theBatchIter = batches.find(theBatchId);
            if (theBatchIter != batches.end())
                theBatchIter.value()->cancel(inSourcePath);
        }
    }

    QDemonImageLoaderBatch *GetBatch(TImageBatchId inId)
    {
        QMutexLocker loaderLock(&loaderMutex);
        TImageLoaderBatchMap::iterator theIter = batches.find(inId);
        if (theIter != batches.end())
            return theIter.value();
        return nullptr;
    }

    void blockUntilLoaded(TImageBatchId inId) override
    {
        // TODO: This is not sane
        QMutexLocker locker(&loaderMutex);
        for (QDemonImageLoaderBatch *theBatch = GetBatch(inId); theBatch; theBatch = GetBatch(inId)) {
            // Only need to block if images aren't loaded.  Don't need to block if they aren't
            // finalized.
            if (!theBatch->isLoadingFinished()) {
                theBatch->loadEvent.wait(&loaderMutex, 200);
                //                theBatch->m_LoadEvent.reset(); ???
            }
            beginFrame();
        }
    }
    void imageLoaded(QDemonLoadingImage &inImage, QDemonLoadedTexture *inTexture)
    {
        QMutexLocker loaderLock(&loaderMutex);
        loadedImages.push_back(QDemonBatchLoadedImage(inImage.sourcePath, inTexture, *inImage.batch));
        inImage.batch->incrementLoadedImageCount();
        inImage.batch->loadEvent.wakeAll();
    }
    // These are called by the render context, users don't need to call this.
    void beginFrame() override
    {
        QMutexLocker loaderLock(&loaderMutex);
        // Pass 1 - send out all image loaded signals
        for (int idx = 0, end = loadedImages.size(); idx < end; ++idx) {

            sourcePathToBatches.remove(loadedImages[idx].sourcePath);
            loadedImages[idx].finalize(bufferManager);
            loadedImages[idx].batch->incrementFinalizedImageCount();
            if (loadedImages[idx].batch->isFinalizedFinished())
                finishedBatches.push_back(loadedImages[idx].batch->batchId);
        }
        loadedImages.clear();
        // pass 2 - clean up any existing batches.
        for (int idx = 0, end = finishedBatches.size(); idx < end; ++idx) {
            TImageLoaderBatchMap::iterator theIter = batches.find(finishedBatches[idx]);
            if (theIter != batches.end()) {
                QDemonImageLoaderBatch *theBatch = theIter.value();
                if (theBatch->loadListener)
                    theBatch->loadListener->OnImageBatchComplete(theBatch->batchId);
                batches.remove(finishedBatches[idx]);
                // TODO:
                theBatch->~QDemonImageLoaderBatch();
            }
        }
        finishedBatches.clear();
    }

    void endFrame() override {}
};

void QDemonLoadingImage::setup(QDemonImageLoaderBatch &inBatch)
{
    batch = &inBatch;
    taskId = inBatch.loader.threadPool->addTask(this, loadImage, taskCancelled);
}

void QDemonLoadingImage::loadImage(void *inImg)
{
    QDemonLoadingImage *theThis = reinterpret_cast<QDemonLoadingImage *>(inImg);
    //    SStackPerfTimer theTimer(theThis->m_Batch->m_Loader.m_PerfTimer, "Image Decompression");
    if (theThis->batch->loader.bufferManager.isImageLoaded(theThis->sourcePath) == false) {
        QDemonRef<QDemonLoadedTexture> theTexture = QDemonLoadedTexture::load(theThis->sourcePath,
                                                                              *theThis->batch->loader.inputStreamFactory,
                                                                              true,
                                                                              theThis->batch->contextType);
        // if ( theTexture )
        //	theTexture->EnsureMultiplerOfFour( theThis->m_Batch->m_Loader.m_Foundation,
        // theThis->m_SourcePath.c_str() );

        theThis->batch->loader.imageLoaded(*theThis, theTexture.data());
    } else {
        theThis->batch->loader.imageLoaded(*theThis, nullptr);
    }
}

void QDemonLoadingImage::taskCancelled(void *inImg)
{
    QDemonLoadingImage *theThis = reinterpret_cast<QDemonLoadingImage *>(inImg);
    theThis->batch->loader.imageLoaded(*theThis, nullptr);
}

bool QDemonBatchLoadedImage::finalize(QDemonBufferManager &inMgr)
{
    if (texture) {
        // PKC : We'll look at the path location to see if the image is in the standard
        // location for IBL light probes or a standard hdr format and decide to generate BSDF
        // miplevels (if the image doesn't have
        // mipmaps of its own that is).
        QString thepath(sourcePath);
        bool isIBL = (thepath.contains(QLatin1String(".hdr"))) || (thepath.contains(QLatin1String("\\IBL\\"))) ||
                     (thepath.contains(QLatin1String("/IBL/")));
        inMgr.loadRenderImage(sourcePath, texture, false, isIBL);
        inMgr.unaliasImagePath(sourcePath);
    }
    if (batch->loadListener)
        batch->loadListener->OnImageLoadComplete(sourcePath, texture ? ImageLoadResult::Succeeded : ImageLoadResult::Failed);

    if (texture) {
        // m_Texture->release();
        return true;
    }

    return false;
}

QDemonImageLoaderBatch *QDemonImageLoaderBatch::createLoaderBatch(QDemonBatchLoader &inLoader,
                                                                  TImageBatchId inBatchId,
                                                                  QDemonConstDataRef<QString> inSourcePaths,
                                                                  QString inImageTillLoaded,
                                                                  IImageLoadListener *inListener,
                                                                  QDemonRenderContextType contextType)
{
    TLoadingImageList theImages;
    quint32 theLoadingImageCount = 0;
    for (int idx = 0, end = inSourcePaths.size(); idx < end; ++idx) {
        QString theSourcePath(inSourcePaths[idx]);

        // TODO: What was the meaning of isValid() (now isEmpty())??
        if (theSourcePath.isEmpty() == false)
            continue;

        if (inLoader.bufferManager.isImageLoaded(theSourcePath))
            continue;

        const auto foundIt = inLoader.sourcePathToBatches.find(inSourcePaths[idx]);

        // TODO: This is a bit funky, check if we really need to update the inBatchId...
        inLoader.sourcePathToBatches.insert(inSourcePaths[idx], inBatchId);

        // If the loader has already seen this image.
        if (foundIt != inLoader.sourcePathToBatches.constEnd())
            continue;

        if (inImageTillLoaded.isEmpty()) {
            // Alias the image so any further requests for this source path will result in
            // the default images (image till loaded).
            bool aliasSuccess = inLoader.bufferManager.aliasImagePath(theSourcePath, inImageTillLoaded, true);
            (void)aliasSuccess;
            Q_ASSERT(aliasSuccess);
        }

        // TODO: Yeah... make sure this is cleaned up correctly.
        QDemonLoadingImage *sli = new QDemonLoadingImage(theSourcePath);
        theImages.push_front(*sli);
        ++theLoadingImageCount;
    }
    if (theImages.empty() == false) {
        QDemonImageLoaderBatch *theBatch = new QDemonImageLoaderBatch(inLoader, inListener, theImages, inBatchId, theLoadingImageCount, contextType);
        return theBatch;
    }
    return nullptr;
}

QDemonImageLoaderBatch::QDemonImageLoaderBatch(QDemonBatchLoader &inLoader,
                                               IImageLoadListener *inLoadListener,
                                               const TLoadingImageList &inImageList,
                                               TImageBatchId inBatchId,
                                               quint32 inImageCount,
                                               QDemonRenderContextType contextType)
    : loader(inLoader)
    , loadListener(inLoadListener)
    , images(inImageList)
    , batchId(inBatchId)
    , loadedOrCanceledImageCount(0)
    , finalizedImageCount(0)
    , numImages(inImageCount)
    , contextType(contextType)
{
    for (TLoadingImageList::iterator iter = images.begin(), end = images.end(); iter != end; ++iter) {
        iter->setup(*this);
    }
}

QDemonImageLoaderBatch::~QDemonImageLoaderBatch()
{
    auto iter = images.begin();
    const auto end = images.end();
    while (iter != end) {
        auto temp(iter);
        ++iter;
        delete temp.m_obj;
    }
}

void QDemonImageLoaderBatch::cancel()
{
    for (TLoadingImageList::iterator iter = images.begin(), end = images.end(); iter != end; ++iter)
        loader.threadPool->cancelTask(iter->taskId);
}

void QDemonImageLoaderBatch::cancel(QString inSourcePath)
{
    for (TLoadingImageList::iterator iter = images.begin(), end = images.end(); iter != end; ++iter) {
        if (iter->sourcePath == inSourcePath) {
            loader.threadPool->cancelTask(iter->taskId);
            break;
        }
    }
}
}

QDemonRef<IImageBatchLoader> IImageBatchLoader::createBatchLoader(QDemonRef<QDemonInputStreamFactoryInterface> inFactory,
                                                                  QDemonBufferManager inBufferManager,
                                                                  QDemonRef<QDemonAbstractThreadPool> inThreadPool,
                                                                  QDemonRef<QDemonPerfTimerInterface> inTimer)
{
    return QDemonRef<IImageBatchLoader>(new QDemonBatchLoader(inFactory, inBufferManager, inThreadPool, inTimer));
}

QDemonBatchLoader::~QDemonBatchLoader()
{
    QVector<TImageBatchId> theCancelledBatches;
    auto theIter = batches.begin();
    const auto theEnd = batches.end();
    while (theIter != theEnd) {
        theIter.value()->cancel();
        theCancelledBatches.push_back(theIter.value()->batchId);
        ++theIter;
    }
    for (int idx = 0, end = theCancelledBatches.size(); idx < end; ++idx)
        blockUntilLoaded(theCancelledBatches[idx]);

    Q_ASSERT(batches.size() == 0);
}

QT_END_NAMESPACE
