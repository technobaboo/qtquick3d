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

#include "qdemonrenderbufferloader.h"

#include <QtDemon/qdemoninvasivelinkedlist.h>
#include <QtDemon/qdemonutils.h>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QWaitCondition>

QT_BEGIN_NAMESPACE

namespace {
struct QDemonBufferLoader;
class QDemonBufferLoadResult : public QDemonLoadedBufferInterface
{
    QString m_path;
    QDemonRef<QDemonBufferLoaderCallbackInterface> m_userData;
    QDemonDataRef<quint8> m_data;
public:
    QDemonBufferLoadResult(QString p,
                           QDemonRef<QDemonBufferLoaderCallbackInterface> ud,
                           QDemonDataRef<quint8> inData)
        : m_path(p)
        , m_userData(ud)
        , m_data(inData)
    {
    }

    QString path() override { return m_path; }
    // Data is released when the buffer itself is released.
    QDemonDataRef<quint8> data() override { return m_data; }
    QDemonRef<QDemonBufferLoaderCallbackInterface> userData() override { return m_userData; }

    static QDemonRef<QDemonBufferLoadResult> Allocate(quint32 inBufferSize,
                                       QString p,
                                       QDemonRef<QDemonBufferLoaderCallbackInterface> ud)
    {
        size_t allocSize = sizeof(QDemonBufferLoadResult) + inBufferSize;
        quint8 *allocMem = static_cast<quint8 *>(::malloc(allocSize));
        if (allocMem == nullptr)
            return nullptr;
        quint8 *bufferStart = allocMem + sizeof(QDemonBufferLoadResult);
        QDemonDataRef<quint8> dataBuffer = toDataRef(bufferStart, inBufferSize);
        return QDemonRef<QDemonBufferLoadResult>(new (allocMem) QDemonBufferLoadResult(p, ud, dataBuffer));
    }
};
struct QDemonLoadedBufferImpl
{
    QDemonBufferLoader &loader;
    quint64 jobId;
    quint64 loadId;
    QString path;
    QDemonRef<QDemonBufferLoaderCallbackInterface> userData;
    bool quiet;
    volatile bool cancel;
    QDemonRef<QDemonBufferLoadResult> result;
    QDemonLoadedBufferImpl *nextBuffer;
    QDemonLoadedBufferImpl *previousBuffer;

    QDemonLoadedBufferImpl(QDemonBufferLoader &l,
                           QString inPath,
                           QDemonRef<QDemonBufferLoaderCallbackInterface> ud,
                           bool inQuiet,
                           quint64 loadId)
        : loader(l)
        , jobId(0)
        , loadId(loadId)
        , path(inPath)
        , userData(ud)
        , quiet(inQuiet)
        , cancel(false)
        , nextBuffer(nullptr)
        , previousBuffer(nullptr)
    {
    }
};

DEFINE_INVASIVE_LIST(QDemonLoadedBufferImpl);
IMPLEMENT_INVASIVE_LIST(QDemonLoadedBufferImpl, previousBuffer, nextBuffer);

struct QDemonBufferLoader : public QDemonBufferLoaderInterface
{
    QDemonRef<QDemonInputStreamFactoryInterface> factory;
    QDemonRef<QDemonAbstractThreadPool> threadPool;

    QMutex buffersToLoadMutex;
    QDemonLoadedBufferImplList buffersToLoad;

    QMutex buffersLoadingMutex;
    QDemonLoadedBufferImplList buffersLoading;

    QMutex loadedBuffersMutex;
    QDemonLoadedBufferImplList loadedBuffers;

    QWaitCondition bufferLoadedEvent;

    quint64 nextBufferId;

    QDemonBufferLoader(QDemonRef<QDemonInputStreamFactoryInterface> fac, QDemonRef<QDemonAbstractThreadPool> tp)
        : factory(fac)
        , threadPool(tp)
        , buffersToLoadMutex()
        , buffersLoadingMutex()
        , loadedBuffersMutex()
        , bufferLoadedEvent()
        , nextBufferId(1)
    {
        bufferLoadedEvent.wakeAll();
    }

    virtual ~QDemonBufferLoader() override;

    static void initializeActiveLoadingBuffer(QDemonLoadedBufferImpl &theBuffer)
    {
        QMutexLocker theLocker(&theBuffer.loader.buffersToLoadMutex);
        QMutexLocker theSecondLocker(&theBuffer.loader.buffersLoadingMutex);
        theBuffer.loader.buffersToLoad.remove(theBuffer);
        theBuffer.loader.buffersLoading.push_back(theBuffer);
    }

    static void setBufferAsLoaded(QDemonLoadedBufferImpl &theBuffer)
    {
        QMutexLocker theSecondLocker(&theBuffer.loader.buffersLoadingMutex);
        QMutexLocker theLocker(&theBuffer.loader.loadedBuffersMutex);
        theBuffer.loader.buffersLoading.remove(theBuffer);
        theBuffer.loader.loadedBuffers.push_back(theBuffer);
        theBuffer.loader.bufferLoadedEvent.wakeAll();
    }

    static void loadNextBuffer(void *loader)
    {
        QDemonLoadedBufferImpl &theBuffer = *reinterpret_cast<QDemonLoadedBufferImpl *>(loader);

        initializeActiveLoadingBuffer(theBuffer);
        QDemonRef<QIODevice> theStream =
                theBuffer.loader.factory->getStreamForFile(theBuffer.path, theBuffer.quiet);
        if (theStream && theBuffer.cancel == false) {
            theStream->seek(IOStream::positionHelper(*theStream.data(), 0, IOStream::SeekPosition::End));
            qint64 theFileLen = theStream->pos();
            if (theFileLen > 0 && theFileLen < (quint32)std::numeric_limits<quint32>::max()) {
                quint64 required = theFileLen;
                theBuffer.result = QDemonBufferLoadResult::Allocate(required, theBuffer.path, theBuffer.userData);
                quint64 amountRead = 0;
                quint64 total = amountRead;
                if (theBuffer.result && theBuffer.cancel == false) {
                    QDemonDataRef<quint8> theDataBuffer(theBuffer.result->data());
                    theStream->seek(0);
                    amountRead = theStream->read(reinterpret_cast<char *>(theDataBuffer.mData), theDataBuffer.mSize);
                    total += amountRead;
                    // Ensure we keep trying, not all file systems allow huge reads.
                    while (total < required && amountRead > 0 && theBuffer.cancel == false) {
                        QDemonDataRef<quint8> newBuffer(theDataBuffer.mData + total, required - total);
                        amountRead = theStream->read(reinterpret_cast<char *>(newBuffer.mData), newBuffer.mSize);
                        total += amountRead;
                    }
                }
                if (theBuffer.cancel || total != required) {
                    theBuffer.result->data() = QDemonDataRef<quint8>();
                }
            }
        }

        // only callback if the file was successfully loaded.
        if (theBuffer.userData) {
            if (theBuffer.cancel == false && theBuffer.result
                    && theBuffer.result->data().size()) {
                theBuffer.userData->onBufferLoaded(*theBuffer.result);
            } else {
                if (theBuffer.cancel)
                    theBuffer.userData->onBufferLoadCancelled(theBuffer.path);
                else
                    theBuffer.userData->onBufferLoadFailed(theBuffer.path);
            }
        }

        setBufferAsLoaded(theBuffer);
    }
    static void cancelNextBuffer(void *loader)
    {
        QDemonLoadedBufferImpl &theBuffer = *reinterpret_cast<QDemonLoadedBufferImpl *>(loader);
        theBuffer.cancel = true;
        initializeActiveLoadingBuffer(theBuffer);

        if (theBuffer.userData)
            theBuffer.userData->onBufferLoadCancelled(theBuffer.path);

        setBufferAsLoaded(theBuffer);
    }

    // nonblocking.  Quiet failure is passed to the input stream factory.
    quint64 queueForLoading(QString inPath,
                            QDemonRef<QDemonBufferLoaderCallbackInterface> inUserData = nullptr,
                            bool inQuietFailure = false) override
    {
        QDemonLoadedBufferImpl &theBuffer = *new QDemonLoadedBufferImpl(*this, inPath, inUserData, inQuietFailure, nextBufferId);
        ++nextBufferId;
        {
            QMutexLocker theLocker(&buffersToLoadMutex);
            buffersToLoad.push_back(theBuffer);
        }
        theBuffer.jobId = threadPool->addTask(&theBuffer, loadNextBuffer, cancelNextBuffer);
        return theBuffer.loadId;
    }

    void cancelBufferLoad(quint64 inBufferId) override
    {
        {
            QMutexLocker theLocker(&buffersToLoadMutex);
            QDemonLoadedBufferImpl *theLoadedBuffer = nullptr;
            auto iter = buffersToLoad.begin();
            const auto end = buffersToLoad.end();
            while (iter != end && theLoadedBuffer == nullptr) {
                if (iter->loadId == inBufferId) {
                    theLoadedBuffer = &(*iter);
                    // both cancellation attempts are necessary.  The user will still get
                    // a load result, it will just have no data.
                    theLoadedBuffer->cancel = true;
                    threadPool->cancelTask(theLoadedBuffer->jobId);
                }
                ++iter;
            }
        }
    }

    // If we were will to wait, will we ever get another buffer
    bool willLoadedBuffersBeAvailable() override
    {
        QMutexLocker theLocker(&buffersToLoadMutex);
        QMutexLocker theSecondLocker(&buffersLoadingMutex);
        return areLoadedBuffersAvailable() || buffersToLoad.empty() == false
                || buffersLoading.empty() == false;
    }
    // Will nextLoadedBuffer block or not?
    bool areLoadedBuffersAvailable() override
    {
        QMutexLocker theLocker(&loadedBuffersMutex);
        return loadedBuffers.empty() == false;
    }

    // blocking, be careful with this.  No order guarantees here.
    QDemonRef<QDemonLoadedBufferInterface> nextLoadedBuffer() override
    {
        while (!areLoadedBuffersAvailable()) {
            bufferLoadedEvent.wait(&loadedBuffersMutex);
        }
        QDemonLoadedBufferImpl *theBuffer;
        {
            QMutexLocker theLocker(&loadedBuffersMutex);
            theBuffer = loadedBuffers.back_ptr();
            loadedBuffers.remove(*theBuffer);
        }
        QDemonRef<QDemonLoadedBufferInterface> retval(theBuffer->result);
        if (retval == nullptr) {
            retval = QDemonBufferLoadResult::Allocate(0, theBuffer->path, theBuffer->userData);
        }
        delete theBuffer;
        return retval;
    }
};
}

QDemonLoadedBufferInterface::~QDemonLoadedBufferInterface()
{

}

QDemonBufferLoaderCallbackInterface::~QDemonBufferLoaderCallbackInterface()
{

}

QDemonBufferLoaderInterface::~QDemonBufferLoaderInterface()
{

}

QDemonRef<QDemonBufferLoaderInterface> QDemonBufferLoaderInterface::create(QDemonRef<QDemonInputStreamFactoryInterface> &inFactory, QDemonRef<QDemonAbstractThreadPool> inThreadPool)
{
    return QDemonRef<QDemonBufferLoaderInterface>(new QDemonBufferLoader(inFactory, inThreadPool));
}

QDemonBufferLoader::~QDemonBufferLoader()
{
    {
        QMutexLocker locker(&buffersToLoadMutex);
        auto iter = buffersToLoad.begin();
        const auto end = buffersToLoad.end();
        while (iter != end) {
            threadPool->cancelTask(iter->jobId);
            ++iter;
        }
    }

    // Pull any remaining buffers out of the thread system.
    while (willLoadedBuffersBeAvailable()) {
        nextLoadedBuffer();
    }
}

QT_END_NAMESPACE

