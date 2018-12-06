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
#ifndef QDEMON_RENDER_THREAD_POOL_H
#define QDEMON_RENDER_THREAD_POOL_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemon/qdemonrefcounted.h>

QT_BEGIN_NAMESPACE

typedef void (*TTaskFunction)(void *inUserData);

struct TaskStates
{
    enum Enum {
        UnknownTask = 0,
        Queued,
        Running,
    };
};

struct CancelReturnValues
{
    enum Enum {
        TaskCanceled = 0,
        TaskRunning,
        TaskNotFound,
    };
};

class IThreadPool : public QDemonRefCounted
{
protected:
    virtual ~IThreadPool() {}
public:
    // Add a task to be run at some point in the future.
    // Tasks will be run roughly in order they are given.
    // The returned value is a handle that can be used to query
    // details about the task
    // Cancel function will be called if the thread pool is destroyed or
    // of the task gets canceled.
    virtual quint64 AddTask(void *inUserData, TTaskFunction inFunction,
                            TTaskFunction inCancelFunction) = 0;
    virtual TaskStates::Enum GetTaskState(quint64 inTaskId) = 0;
    virtual CancelReturnValues::Enum CancelTask(quint64 inTaskId) = 0;

    static IThreadPool &CreateThreadPool(NVFoundationBase &inFoundation,
                                         quint32 inNumThreads = 4);
};
QT_END_NAMESPACE
#endif
