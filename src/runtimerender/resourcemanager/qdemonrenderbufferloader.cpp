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
struct SBufferLoader;
struct SBufferLoadResult : public ILoadedBuffer
{
    QString m_Path;
    QSharedPointer<IBufferLoaderCallback> m_UserData;
    QDemonDataRef<quint8> m_Data;

    SBufferLoadResult(QString p, QSharedPointer<IBufferLoaderCallback> ud,
                      QDemonDataRef<quint8> data)
        : m_Path(p)
        , m_UserData(ud)
        , m_Data(data)
    {
    }

    QString Path() override { return m_Path; }
    // Data is released when the buffer itself is released.
    QDemonDataRef<quint8> Data() override { return m_Data; }
    QSharedPointer<IBufferLoaderCallback> UserData() override { return m_UserData; }

    static QSharedPointer<SBufferLoadResult> Allocate(quint32 inBufferSize,
                                       QString p,
                                       QSharedPointer<IBufferLoaderCallback> ud)
    {
        size_t allocSize = sizeof(SBufferLoadResult) + inBufferSize;
        quint8 *allocMem = static_cast<quint8 *>(::malloc(allocSize));
        if (allocMem == nullptr)
            return nullptr;
        quint8 *bufferStart = allocMem + sizeof(SBufferLoadResult);
        QDemonDataRef<quint8> dataBuffer = toDataRef(bufferStart, inBufferSize);
        return QSharedPointer<SBufferLoadResult>(new (allocMem) SBufferLoadResult(p, ud, dataBuffer));
    }
};
struct SLoadedBufferImpl
{
    SBufferLoader &m_Loader;
    quint64 m_JobId;
    quint64 m_LoadId;
    QString m_Path;
    QSharedPointer<IBufferLoaderCallback> m_UserData;
    bool m_Quiet;
    volatile bool m_Cancel;
    QSharedPointer<SBufferLoadResult> m_Result;
    SLoadedBufferImpl *m_NextBuffer;
    SLoadedBufferImpl *m_PreviousBuffer;

    SLoadedBufferImpl(SBufferLoader &l, QString inPath,
                      QSharedPointer<IBufferLoaderCallback> ud, bool inQuiet, quint64 loadId)
        : m_Loader(l)
        , m_JobId(0)
        , m_LoadId(loadId)
        , m_Path(inPath)
        , m_UserData(ud)
        , m_Quiet(inQuiet)
        , m_Cancel(false)
        , m_NextBuffer(nullptr)
        , m_PreviousBuffer(nullptr)
    {
    }
};

DEFINE_INVASIVE_LIST(LoadedBufferImpl);
IMPLEMENT_INVASIVE_LIST(LoadedBufferImpl, m_PreviousBuffer, m_NextBuffer);

struct SBufferLoader : public IBufferLoader
{
    QSharedPointer<IInputStreamFactory> m_Factory;
    QSharedPointer<IThreadPool> m_ThreadPool;

    QMutex m_BuffersToLoadMutex;
    TLoadedBufferImplList m_BuffersToLoad;

    QMutex m_BuffersLoadingMutex;
    TLoadedBufferImplList m_BuffersLoading;

    QMutex m_LoadedBuffersMutex;
    TLoadedBufferImplList m_LoadedBuffers;

    QWaitCondition m_BufferLoadedEvent;

    quint64 m_NextBufferId;

    SBufferLoader(QSharedPointer<IInputStreamFactory> fac, QSharedPointer<IThreadPool> tp)
        : m_Factory(fac)
        , m_ThreadPool(tp)
        , m_BuffersToLoadMutex()
        , m_BuffersLoadingMutex()
        , m_LoadedBuffersMutex()
        , m_BufferLoadedEvent()
        , m_NextBufferId(1)
    {
        m_BufferLoadedEvent.wakeAll();
    }

    virtual ~SBufferLoader()
    {
        {
            QMutexLocker locker(&m_BuffersToLoadMutex);
            for (TLoadedBufferImplList::iterator iter = m_BuffersToLoad.begin(),
                 end = m_BuffersToLoad.end();
                 iter != end; ++iter) {
                m_ThreadPool->CancelTask(iter->m_JobId);
            }
        }

        // Pull any remaining buffers out of the thread system.
        while (WillLoadedBuffersBeAvailable()) {
            NextLoadedBuffer();
        }
    }

    static void InitializeActiveLoadingBuffer(SLoadedBufferImpl &theBuffer)
    {
        QMutexLocker theLocker(&theBuffer.m_Loader.m_BuffersToLoadMutex);
        QMutexLocker theSecondLocker(&theBuffer.m_Loader.m_BuffersLoadingMutex);
        theBuffer.m_Loader.m_BuffersToLoad.remove(theBuffer);
        theBuffer.m_Loader.m_BuffersLoading.push_back(theBuffer);
    }

    static void SetBufferAsLoaded(SLoadedBufferImpl &theBuffer)
    {
        QMutexLocker theSecondLocker(&theBuffer.m_Loader.m_BuffersLoadingMutex);
        QMutexLocker theLocker(&theBuffer.m_Loader.m_LoadedBuffersMutex);
        theBuffer.m_Loader.m_BuffersLoading.remove(theBuffer);
        theBuffer.m_Loader.m_LoadedBuffers.push_back(theBuffer);
        theBuffer.m_Loader.m_BufferLoadedEvent.wakeAll();
    }

    static void LoadNextBuffer(void *loader)
    {
        SLoadedBufferImpl &theBuffer = *reinterpret_cast<SLoadedBufferImpl *>(loader);

        InitializeActiveLoadingBuffer(theBuffer);
        QSharedPointer<QIODevice> theStream =
                theBuffer.m_Loader.m_Factory->GetStreamForFile(theBuffer.m_Path, theBuffer.m_Quiet);
        if (theStream && theBuffer.m_Cancel == false) {
            theStream->seek(IOStream::positionHelper(*theStream.data(), 0, IOStream::SeekPosition::End));
            qint64 theFileLen = theStream->pos();
            if (theFileLen > 0 && theFileLen < (quint32)std::numeric_limits<quint32>::max()) {
                quint64 required = theFileLen;
                theBuffer.m_Result = SBufferLoadResult::Allocate(required, theBuffer.m_Path, theBuffer.m_UserData);
                quint64 amountRead = 0;
                quint64 total = amountRead;
                if (theBuffer.m_Result && theBuffer.m_Cancel == false) {
                    QDemonDataRef<quint8> theDataBuffer(theBuffer.m_Result->m_Data);
                    theStream->seek(0);
                    amountRead = theStream->read(reinterpret_cast<char *>(theDataBuffer.mData), theDataBuffer.mSize);
                    total += amountRead;
                    // Ensure we keep trying, not all file systems allow huge reads.
                    while (total < required && amountRead > 0 && theBuffer.m_Cancel == false) {
                        QDemonDataRef<quint8> newBuffer(theDataBuffer.mData + total, required - total);
                        amountRead = theStream->read(reinterpret_cast<char *>(newBuffer.mData), newBuffer.mSize);
                        total += amountRead;
                    }
                }
                if (theBuffer.m_Cancel || total != required) {
                    theBuffer.m_Result->m_Data = QDemonDataRef<quint8>();
                }
            }
        }

        // only callback if the file was successfully loaded.
        if (theBuffer.m_UserData) {
            if (theBuffer.m_Cancel == false && theBuffer.m_Result
                    && theBuffer.m_Result->m_Data.size()) {
                theBuffer.m_UserData->OnBufferLoaded(*theBuffer.m_Result);
            } else {
                if (theBuffer.m_Cancel)
                    theBuffer.m_UserData->OnBufferLoadCancelled(theBuffer.m_Path);
                else
                    theBuffer.m_UserData->OnBufferLoadFailed(theBuffer.m_Path);
            }
        }

        SetBufferAsLoaded(theBuffer);
    }
    static void CancelNextBuffer(void *loader)
    {
        SLoadedBufferImpl &theBuffer = *reinterpret_cast<SLoadedBufferImpl *>(loader);
        theBuffer.m_Cancel = true;
        InitializeActiveLoadingBuffer(theBuffer);

        if (theBuffer.m_UserData)
            theBuffer.m_UserData->OnBufferLoadCancelled(theBuffer.m_Path);

        SetBufferAsLoaded(theBuffer);
    }

    // nonblocking.  Quiet failure is passed to the input stream factory.
    quint64 QueueForLoading(QString inPath,
                            QSharedPointer<IBufferLoaderCallback> inUserData = nullptr,
                            bool inQuietFailure = false) override
    {
        SLoadedBufferImpl &theBuffer = *new SLoadedBufferImpl(*this, inPath, inUserData, inQuietFailure, m_NextBufferId);
        ++m_NextBufferId;
        {
            QMutexLocker theLocker(&m_BuffersToLoadMutex);
            m_BuffersToLoad.push_back(theBuffer);
        }
        theBuffer.m_JobId = m_ThreadPool->AddTask(&theBuffer, LoadNextBuffer, CancelNextBuffer);
        return theBuffer.m_LoadId;
    }

    void CancelBufferLoad(quint64 inBufferId) override
    {
        {
            QMutexLocker theLocker(&m_BuffersToLoadMutex);
            SLoadedBufferImpl *theLoadedBuffer = nullptr;
            for (TLoadedBufferImplList::iterator iter = m_BuffersToLoad.begin(),
                 end = m_BuffersToLoad.end();
                 iter != end && theLoadedBuffer == nullptr; ++iter) {
                if (iter->m_LoadId == inBufferId) {
                    theLoadedBuffer = &(*iter);
                    // both cancellation attempts are necessary.  The user will still get
                    // a load result, it will just have no data.
                    theLoadedBuffer->m_Cancel = true;
                    m_ThreadPool->CancelTask(theLoadedBuffer->m_JobId);
                }
            }
        }
    }

    // If we were will to wait, will we ever get another buffer
    bool WillLoadedBuffersBeAvailable() override
    {
        QMutexLocker theLocker(&m_BuffersToLoadMutex);
        QMutexLocker theSecondLocker(&m_BuffersLoadingMutex);
        return AreLoadedBuffersAvailable() || m_BuffersToLoad.empty() == false
                || m_BuffersLoading.empty() == false;
    }
    // Will nextLoadedBuffer block or not?
    bool AreLoadedBuffersAvailable() override
    {
        QMutexLocker theLocker(&m_LoadedBuffersMutex);
        return m_LoadedBuffers.empty() == false;
    }

    // blocking, be careful with this.  No order guarantees here.
    QSharedPointer<ILoadedBuffer> NextLoadedBuffer() override
    {
        while (!AreLoadedBuffersAvailable()) {
            m_BufferLoadedEvent.wait(&m_LoadedBuffersMutex);
        }
        SLoadedBufferImpl *theBuffer;
        {
            QMutexLocker theLocker(&m_LoadedBuffersMutex);
            theBuffer = m_LoadedBuffers.back_ptr();
            m_LoadedBuffers.remove(*theBuffer);
        }
        QSharedPointer<ILoadedBuffer> retval(theBuffer->m_Result);
        if (retval == nullptr) {
            retval = SBufferLoadResult::Allocate(0, theBuffer->m_Path, theBuffer->m_UserData);
        }
        delete theBuffer;
        return retval;
    }
};
}

QSharedPointer<IBufferLoader> IBufferLoader::Create(QSharedPointer<IInputStreamFactory> &inFactory, QSharedPointer<IThreadPool> inThreadPool)
{
    return QSharedPointer<IBufferLoader>(new SBufferLoader(inFactory, inThreadPool));
}

QT_END_NAMESPACE
