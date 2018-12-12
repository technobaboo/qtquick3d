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
#ifndef QDEMON_RENDER_BUFFER_LOADED_H
#define QDEMON_RENDER_BUFFER_LOADED_H

QT_BEGIN_NAMESPACE

class IBufferLoaderCallback;

class ILoadedBuffer
{
public:
    virtual QString Path() = 0;
    // Data is released when the buffer itself is released.
    virtual QDemonDataRef<quint8> Data() = 0;
    virtual IBufferLoaderCallback *UserData() = 0;
};

class IBufferLoaderCallback
{
public:
    virtual void OnBufferLoaded(ILoadedBuffer &inBuffer) = 0;
    virtual void OnBufferLoadFailed(QString inPath) = 0;
    virtual void OnBufferLoadCancelled(QString inPath) = 0;
};

// Job of this object is to load buffers all the way to memory as fast as possible.
class IBufferLoader
{
public:
    // nonblocking.  Quiet failure is passed to the input stream factory.
    // Returns handle to loading buffer
    virtual quint64 QueueForLoading(QString inPath,
                                    IBufferLoaderCallback *inUserData = nullptr,
                                    bool inQuietFailure = false) = 0;
    // Cancel a buffer that has not made it to the loaded buffers list.
    virtual void CancelBufferLoad(quint64 inBufferId) = 0;
    // If we were will to wait, will we ever get another buffer
    virtual bool WillLoadedBuffersBeAvailable() = 0;
    // Will nextLoadedBuffer block or not?
    virtual bool AreLoadedBuffersAvailable() = 0;

    // blocking, be careful with this.  No guarantees about timely return here.
    virtual QSharedPointer<ILoadedBuffer> NextLoadedBuffer() = 0;

    static IBufferLoader &Create(IInputStreamFactory &inFactory,
                                 IThreadPool &inThreadPool);
};
QT_END_NAMESPACE

#endif
