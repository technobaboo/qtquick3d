/****************************************************************************
**
** Copyright (C) 2008-2014 NVIDIA Corporation.
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

#include <QtDemonRuntimeRender/qdemonrenderprofiler.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRender/qdemonrendertimerquery.h>
#include <QtDemonRender/qdemonrendersync.h>
#include <QtDemonRender/qdemonrendercontext.h>


#define RECORDED_FRAME_DELAY 3
#define RECORDED_FRAME_DELAY_MASK 0x0003

QT_BEGIN_NAMESPACE

namespace {

struct SGpuTimerInfo
{
    bool m_AbsoluteTime;
    quint16 m_WriteID;
    quint16 m_ReadID;
    quint16 m_AverageTimeWriteID;
    quint64 m_AverageTime[10];
    quint32 m_FrameID[RECORDED_FRAME_DELAY];
    QSharedPointer<QDemonRenderTimerQuery> m_TimerStartQueryObjects[RECORDED_FRAME_DELAY];
    QSharedPointer<QDemonRenderTimerQuery> m_TimerEndQueryObjects[RECORDED_FRAME_DELAY];
    QSharedPointer<QDemonRenderSync> m_TimerSyncObjects[RECORDED_FRAME_DELAY];

    SGpuTimerInfo()
        : m_AbsoluteTime(false)
        , m_WriteID(0)
        , m_ReadID(0)
        , m_AverageTimeWriteID(0)
    {
        memset(m_AverageTime, 0x0, 10 * sizeof(quint64));
    }

    ~SGpuTimerInfo() {}

    void IncrementWriteCounter()
    {
        m_WriteID++;
        m_WriteID %= RECORDED_FRAME_DELAY_MASK;
    }

    void IncrementReadCounter()
    {
        m_ReadID++;
        m_ReadID %= RECORDED_FRAME_DELAY_MASK;
    }

    void IncrementAveragedWriteCounter()
    {
        m_AverageTimeWriteID++;
        m_AverageTimeWriteID %= 10;
    }

    void StartTimerQuery(quint32 frameID)
    {
        m_FrameID[m_WriteID] = frameID;

        if (m_AbsoluteTime)
            m_TimerStartQueryObjects[m_WriteID]->SetTimerQuery();
        else
            m_TimerStartQueryObjects[m_WriteID]->Begin();
    }

    void EndTimerQuery()
    {
        if (m_AbsoluteTime)
            m_TimerEndQueryObjects[m_WriteID]->SetTimerQuery();
        else
            m_TimerStartQueryObjects[m_WriteID]->End();

        IncrementWriteCounter();
    }

    void AddSync()
    {
        m_TimerSyncObjects[m_WriteID]->Sync();
        m_TimerSyncObjects[m_WriteID]->Wait();
    }

    double GetAveragedElapsedTimeInMs()
    {
        double time =
                double(((m_AverageTime[0] + m_AverageTime[1] + m_AverageTime[2] + m_AverageTime[3]
                + m_AverageTime[4] + m_AverageTime[5] + m_AverageTime[6] + m_AverageTime[7]
                + m_AverageTime[8] + m_AverageTime[9])
                / 10)
                / 1e06);

        return time;
    }

    double GetElapsedTimeInMs(quint32 frameID)
    {
        double time = 0;

        if (((frameID - m_FrameID[m_ReadID]) < 2) || (m_ReadID == m_WriteID))
            return time;

        if (m_AbsoluteTime) {
            quint64 startTime, endTime;

            m_TimerStartQueryObjects[m_ReadID]->GetResult(&startTime);
            m_TimerEndQueryObjects[m_ReadID]->GetResult(&endTime);

            m_AverageTime[m_AverageTimeWriteID] = endTime - startTime;
        } else {
            quint64 elapsedTime;

            m_TimerStartQueryObjects[m_ReadID]->GetResult(&elapsedTime);

            m_AverageTime[m_AverageTimeWriteID] = elapsedTime;
        }

        IncrementReadCounter();
        IncrementAveragedWriteCounter();

        return GetAveragedElapsedTimeInMs();
    }
};

class QDemonRenderGpuProfiler : public IRenderProfiler
{
    typedef QHash<QString, QSharedPointer<SGpuTimerInfo>> TStrGpuTimerInfoMap;

private:
    QSharedPointer<QDemonRenderContext> m_RenderContext;
    QSharedPointer<IQDemonRenderContext> m_Context;

    TStrGpuTimerInfoMap m_StrToGpuTimerMap;
    IRenderProfiler::TStrIDVec m_StrToIDVec;
    mutable quint32 m_VertexCount;

public:
    QDemonRenderGpuProfiler(QSharedPointer<IQDemonRenderContext> inContext,
                            QSharedPointer<QDemonRenderContext> inRenderContext)
        : m_RenderContext(inRenderContext)
        , m_Context(inContext)
        , m_VertexCount(0)
    {
    }

    virtual ~QDemonRenderGpuProfiler() { m_StrToGpuTimerMap.clear(); }

    void StartTimer(QString &nameID, bool absoluteTime, bool sync) override
    {
        QSharedPointer<SGpuTimerInfo> theGpuTimerData = GetOrCreateGpuTimerInfo(nameID);

        if (theGpuTimerData) {
            if (sync)
                theGpuTimerData->AddSync();

            theGpuTimerData->m_AbsoluteTime = absoluteTime;
            theGpuTimerData->StartTimerQuery(m_Context->GetFrameCount());
        }
    }

    void EndTimer(QString &nameID) override
    {
        QSharedPointer<SGpuTimerInfo> theGpuTimerData = GetOrCreateGpuTimerInfo(nameID);

        if (theGpuTimerData) {
            theGpuTimerData->EndTimerQuery();
        }
    }

    double GetElapsedTime(const QString &nameID) const override
    {
        double time = 0;
        QSharedPointer<SGpuTimerInfo> theGpuTimerData = GetGpuTimerInfo(nameID);

        if (theGpuTimerData) {
            time = theGpuTimerData->GetElapsedTimeInMs(m_Context->GetFrameCount());
        }

        return time;
    }

    const TStrIDVec &GetTimerIDs() const override { return m_StrToIDVec; }

    void AddVertexCount(quint32 count) override { m_VertexCount += count; }

    quint32 GetAndResetTriangleCount() const override
    {
        quint32 tris = m_VertexCount / 3;
        m_VertexCount = 0;
        return tris;
    }

private:
    QSharedPointer<SGpuTimerInfo> GetOrCreateGpuTimerInfo(QString &nameID)
    {
        TStrGpuTimerInfoMap::const_iterator theIter = m_StrToGpuTimerMap.find(nameID);
        if (theIter != m_StrToGpuTimerMap.end())
            return theIter.value();

        QSharedPointer<SGpuTimerInfo> theGpuTimerData = QSharedPointer<SGpuTimerInfo>(new SGpuTimerInfo());

        if (theGpuTimerData) {
            // create queries
            for (quint32 i = 0; i < RECORDED_FRAME_DELAY; i++) {
                theGpuTimerData->m_TimerStartQueryObjects[i] = m_RenderContext->CreateTimerQuery();
                theGpuTimerData->m_TimerEndQueryObjects[i] = m_RenderContext->CreateTimerQuery();
                theGpuTimerData->m_TimerSyncObjects[i] = m_RenderContext->CreateSync();
                theGpuTimerData->m_FrameID[i] = 0;
            }
            m_StrToGpuTimerMap.insert(nameID, theGpuTimerData);
            m_StrToIDVec.push_back(nameID);
        }

        return theGpuTimerData;
    }

    QSharedPointer<SGpuTimerInfo> GetGpuTimerInfo(const QString &nameID) const
    {
        TStrGpuTimerInfoMap::const_iterator theIter = m_StrToGpuTimerMap.find(nameID);
        if (theIter != m_StrToGpuTimerMap.end())
            return theIter.value();

        return nullptr;
    }
};
}

QSharedPointer<IRenderProfiler> IRenderProfiler::CreateGpuProfiler(QSharedPointer<IQDemonRenderContext> inContext,
                                                                   QSharedPointer<QDemonRenderContext> inRenderContext)
{
    return QSharedPointer<IRenderProfiler>(new QDemonRenderGpuProfiler(inContext, inRenderContext));
}

QT_END_NAMESPACE
