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

namespace {

struct QDemonRenderList : public QDemonRenderListInterface
{
    typedef QPair<quint32, QDemonRef<QDemonRenderTask>> TTaskIdTaskPair;
    typedef QVector<TTaskIdTaskPair> TTaskList;

    TTaskList m_tasks;
    quint32 m_nextTaskId;
    bool m_scissorEnabled;
    QRect m_scissorRect;
    QRect m_viewport;

    QDemonRenderList()
        : m_nextTaskId(1)
        , m_scissorEnabled(false)
    {
    }

    void beginFrame() override
    {
        m_nextTaskId = 1;
        m_tasks.clear();
    }

    quint32 addRenderTask(QDemonRef<QDemonRenderTask> inTask) override
    {
        quint32 taskId = m_nextTaskId;
        ++m_nextTaskId;
        m_tasks.push_back(QPair<quint32, QDemonRef<QDemonRenderTask>>(taskId, inTask));
        return taskId;
    }

    void discardRenderTask(quint32 inTaskId) override
    {
        auto iter = m_tasks.begin();
        const auto end = m_tasks.end();
        while (iter != end && iter->first != inTaskId)
            ++iter;

        if (iter != end)
            m_tasks.erase(iter);
    }
    // This runs through the added tasks in reverse order.  This is used to render dependencies
    // before rendering to the main render target.
    void runRenderTasks() override
    {
        auto iter = m_tasks.rbegin();
        const auto end = m_tasks.rend();
        while (iter != end) {
            iter->second->run();
            ++iter;
        }
        beginFrame();
    }

    void setScissorTestEnabled(bool enabled) override { m_scissorEnabled = enabled; }
    void setScissorRect(QRect rect) override { m_scissorRect = rect; }
    void setViewport(QRect rect) override { m_viewport = rect; }
    bool isScissorTestEnabled() const override { return m_scissorEnabled; }
    QRect getScissor() const override { return m_scissorRect; }
    QRect getViewport() const override { return m_viewport; }
};
}

QDemonRef<QDemonRenderListInterface> QDemonRenderListInterface::createRenderList()
{
    return QDemonRef<QDemonRenderListInterface>(new QDemonRenderList());
}

QDemonRenderTask::~QDemonRenderTask()
{

}

QT_END_NAMESPACE
