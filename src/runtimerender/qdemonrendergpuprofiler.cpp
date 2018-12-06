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
#include <Qt3DSContainers.h>
#include <Qt3DSFoundation.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <qdemonrendertimerquery.h>
#include <qdemonrendersync.h>

#define RECORDED_FRAME_DELAY 3
#define RECORDED_FRAME_DELAY_MASK 0x0003

QT_BEGIN_NAMESPACE

namespace {

using eastl::make_pair;

struct SGpuTimerInfo
{
    NVFoundationBase &m_Foundation;
    volatile qint32 mRefCount;
    bool m_AbsoluteTime;
    quint16 m_WriteID;
    quint16 m_ReadID;
    quint16 m_AverageTimeWriteID;
    quint64 m_AverageTime[10];
    quint32 m_FrameID[RECORDED_FRAME_DELAY];
    QDemonScopedRefCounted<QDemonRenderTimerQuery> m_TimerStartQueryObjects[RECORDED_FRAME_DELAY];
    QDemonScopedRefCounted<QDemonRenderTimerQuery> m_TimerEndQueryObjects[RECORDED_FRAME_DELAY];
    QDemonScopedRefCounted<QDemonRenderSync> m_TimerSyncObjects[RECORDED_FRAME_DELAY];

    SGpuTimerInfo(NVFoundationBase &inFoundation)
        : m_Foundation(inFoundation)
        , mRefCount(0)
        , m_AbsoluteTime(false)
        , m_WriteID(0)
        , m_ReadID(0)
        , m_AverageTimeWriteID(0)
    {
        memset(m_AverageTime, 0x0, 10 * sizeof(quint64));
    }

    ~SGpuTimerInfo() {}

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Foundation.getAllocator())

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

class Qt3DSCRenderGpuProfiler : public IRenderProfiler
{
    typedef nvhash_map<CRegisteredString, QDemonScopedRefCounted<SGpuTimerInfo>> TStrGpuTimerInfoMap;

private:
    NVFoundationBase &m_Foundation;
    QDemonScopedRefCounted<QDemonRenderContext> m_RenderContext;
    IQt3DSRenderContext &m_Context;
    volatile qint32 mRefCount;

    TStrGpuTimerInfoMap m_StrToGpuTimerMap;
    IRenderProfiler::TStrIDVec m_StrToIDVec;
    mutable quint32 m_VertexCount;

public:
    Qt3DSCRenderGpuProfiler(NVFoundationBase &inFoundation, IQt3DSRenderContext &inContext,
                            QDemonRenderContext &inRenderContext)
        : m_Foundation(inFoundation)
        , m_RenderContext(inRenderContext)
        , m_Context(inContext)
        , mRefCount(0)
        , m_StrToGpuTimerMap(inContext.GetAllocator(), "Qt3DSRenderGpuProfiler::m_StrToGpuTimerMap")
        , m_StrToIDVec(inContext.GetAllocator(), "Qt3DSRenderGpuProfiler::m_StrToIDVec")
        , m_VertexCount(0)
    {
    }

    virtual ~Qt3DSCRenderGpuProfiler() { m_StrToGpuTimerMap.clear(); }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Foundation.getAllocator())

    void StartTimer(CRegisteredString &nameID, bool absoluteTime, bool sync) override
    {
        SGpuTimerInfo *theGpuTimerData = GetOrCreateGpuTimerInfo(nameID);

        if (theGpuTimerData) {
            if (sync)
                theGpuTimerData->AddSync();

            theGpuTimerData->m_AbsoluteTime = absoluteTime;
            theGpuTimerData->StartTimerQuery(m_Context.GetFrameCount());
        }
    }

    void EndTimer(CRegisteredString &nameID) override
    {
        SGpuTimerInfo *theGpuTimerData = GetOrCreateGpuTimerInfo(nameID);

        if (theGpuTimerData) {
            theGpuTimerData->EndTimerQuery();
        }
    }

    double GetElapsedTime(const CRegisteredString &nameID) const override
    {
        double time = 0;
        SGpuTimerInfo *theGpuTimerData = GetGpuTimerInfo(nameID);

        if (theGpuTimerData) {
            time = theGpuTimerData->GetElapsedTimeInMs(m_Context.GetFrameCount());
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
    SGpuTimerInfo *GetOrCreateGpuTimerInfo(CRegisteredString &nameID)
    {
        TStrGpuTimerInfoMap::const_iterator theIter = m_StrToGpuTimerMap.find(nameID);
        if (theIter != m_StrToGpuTimerMap.end())
            return const_cast<SGpuTimerInfo *>(theIter->second.mPtr);

        SGpuTimerInfo *theGpuTimerData =
                QDEMON_NEW(m_Context.GetAllocator(), SGpuTimerInfo)(m_Foundation);

        if (theGpuTimerData) {
            // create queries
            for (quint32 i = 0; i < RECORDED_FRAME_DELAY; i++) {
                theGpuTimerData->m_TimerStartQueryObjects[i] = m_RenderContext->CreateTimerQuery();
                theGpuTimerData->m_TimerEndQueryObjects[i] = m_RenderContext->CreateTimerQuery();
                theGpuTimerData->m_TimerSyncObjects[i] = m_RenderContext->CreateSync();
                theGpuTimerData->m_FrameID[i] = 0;
            }
            m_StrToGpuTimerMap.insert(make_pair(nameID, theGpuTimerData));
            m_StrToIDVec.push_back(nameID);
        }

        return theGpuTimerData;
    }

    SGpuTimerInfo *GetGpuTimerInfo(const CRegisteredString &nameID) const
    {
        TStrGpuTimerInfoMap::const_iterator theIter = m_StrToGpuTimerMap.find(nameID);
        if (theIter != m_StrToGpuTimerMap.end())
            return const_cast<SGpuTimerInfo *>(theIter->second.mPtr);

        return nullptr;
    }
};
}

IRenderProfiler &IRenderProfiler::CreateGpuProfiler(NVFoundationBase &inFnd,
                                                    IQt3DSRenderContext &inContext,
                                                    QDemonRenderContext &inRenderContext)
{
    return *QDEMON_NEW(inFnd.getAllocator(), Qt3DSCRenderGpuProfiler)(inFnd, inContext, inRenderContext);
}

QT_END_NAMESPACE
