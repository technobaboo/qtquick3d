/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#include <qdemonrenderthreadpool.h>

#include <QtCore/QThreadPool>
#include <QtCore/QRunnable>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

namespace {
class QDemonThreadPool;
class QDemonTask : public QRunnable
{
public:
    QDemonTask(void *inUserData, QDemonTaskCallback inFunction, QDemonTaskCallback inCancelFunction, quint64 id, QDemonThreadPool *threadPool)
        : m_userData(inUserData)
        , m_function(inFunction)
        , m_cancelFunction(inCancelFunction)
        , m_id(id)
        , m_taskState(TaskStates::Queued)
        , m_threadPool(threadPool)
    {
        setAutoDelete(false);
    }

    TaskStates taskState()
    {
        QMutexLocker locker(&m_mutex);
        return m_taskState;
    }

    void run() override;

    void doFunction()
    {
        if (m_function)
            m_function(m_userData);
    }

    bool doCancel()
    {
        {
            QMutexLocker locker(&m_mutex);
            if (m_taskState == TaskStates::Running)
                return false;
        }

        if (m_cancelFunction)
            m_cancelFunction(m_userData);

        return true;
    }

private:
    void *m_userData;
    QDemonTaskCallback m_function;
    QDemonTaskCallback m_cancelFunction;
    quint64 m_id;
    TaskStates m_taskState;
    QMutex m_mutex;
    QDemonThreadPool *m_threadPool;
};

class QDemonThreadPool : public QDemonAbstractThreadPool
{
public:
    QDemonThreadPool(quint32 numThreads);

    ~QDemonThreadPool() override;

    quint64 addTask(void *inUserData, QDemonTaskCallback inFunction, QDemonTaskCallback inCancelFunction) override;

    TaskStates getTaskState(quint64 inTaskId) override;

    CancelReturnValues cancelTask(quint64 inTaskId) override;

    // Called from another thread!
    void taskFinished(quint64 inTaskId);

private:
    static quint64 generateTaskID()
    {
        static quint64 taskID = 0;
        return taskID++;
    }

    QThreadPool m_threadPool;
    QHash<quint64, QDemonTask *> m_taskMap;
    QMutex m_mutex;
};

void QDemonTask::run()
{
    {
        QMutexLocker locker(&m_mutex);
        m_taskState = TaskStates::Running;
    }

    doFunction();

    m_threadPool->taskFinished(m_id);
}

QDemonThreadPool::QDemonThreadPool(quint32 numThreads)
{
    m_threadPool.setMaxThreadCount(int(numThreads));
}

QDemonThreadPool::~QDemonThreadPool()
{
    QMutexLocker locker(&m_mutex);
    for (auto task : m_taskMap.values()) {
        if (m_threadPool.tryTake(task))
            task->doCancel();
        delete task;
    }
}

quint64 QDemonThreadPool::addTask(void *inUserData, QDemonTaskCallback inFunction, QDemonTaskCallback inCancelFunction)
{
    QMutexLocker locker(&m_mutex);
    const quint64 taskID = generateTaskID();
    auto task = new QDemonTask(inUserData, inFunction, inCancelFunction, taskID, this);
    m_taskMap.insert(taskID, task);
    m_threadPool.start(task);
    return taskID;
}

TaskStates QDemonThreadPool::getTaskState(quint64 inTaskId)
{
    QMutexLocker locker(&m_mutex);
    auto task = m_taskMap.value(inTaskId, nullptr);
    if (!task)
        return TaskStates::UnknownTask;
    return task->taskState();
}

CancelReturnValues QDemonThreadPool::cancelTask(quint64 inTaskId)
{
    QMutexLocker locker(&m_mutex);
    auto task = m_taskMap.value(inTaskId, nullptr);
    if (!task)
        return CancelReturnValues::TaskNotFound;

    if (m_threadPool.tryTake(task))
        if (task->doCancel()) {
            auto task = m_taskMap.value(inTaskId);
            delete task;
            m_taskMap.remove(inTaskId);
            return CancelReturnValues::TaskCanceled;
        }

    return CancelReturnValues::TaskRunning;
}

void QDemonThreadPool::taskFinished(quint64 inTaskId)
{
    QMutexLocker locker(&m_mutex);
    auto task = m_taskMap.value(inTaskId);
    delete task;
    m_taskMap.remove(inTaskId);
}
}

QDemonAbstractThreadPool::~QDemonAbstractThreadPool() = default;

QDemonRef<QDemonAbstractThreadPool> QDemonAbstractThreadPool::createThreadPool(quint32 inNumThreads)
{
    return QDemonRef<QDemonAbstractThreadPool>(new QDemonThreadPool(inNumThreads));
}

QT_END_NAMESPACE
