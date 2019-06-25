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
#include <qdemonrenderrenderlist.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

QDemonRef<QDemonRenderList> QDemonRenderList::createRenderList()
{
    return QDemonRef<QDemonRenderList>(new QDemonRenderList());
}

QDemonRenderTask::~QDemonRenderTask() = default;

QT_END_NAMESPACE

void QDemonRenderList::beginFrame()
{
    m_nextTaskId = 1;
    m_tasks.clear();
}

quint32 QDemonRenderList::addRenderTask(QDemonRef<QDemonRenderTask> inTask)
{
    quint32 taskId = m_nextTaskId;
    ++m_nextTaskId;
    m_tasks.push_back(QPair<quint32, QDemonRef<QDemonRenderTask>>(taskId, inTask));
    return taskId;
}

void QDemonRenderList::discardRenderTask(quint32 inTaskId)
{
    auto iter = m_tasks.begin();
    const auto end = m_tasks.end();
    while (iter != end && iter->first != inTaskId)
        ++iter;

    if (iter != end)
        m_tasks.erase(iter);
}

void QDemonRenderList::runRenderTasks()
{
    auto iter = m_tasks.crbegin();
    const auto end = m_tasks.crend();
    while (iter != end) {
        iter->second->run();
        ++iter;
    }
    beginFrame();
}
