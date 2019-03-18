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
#ifndef QDEMON_RENDER_PROFILER_H
#define QDEMON_RENDER_PROFILER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

/**
 *	Opaque profiling system for rendering.
 */

class QDemonRenderContextInterface;
class QDemonRenderContext;
struct QDemonGpuTimerInfo;

class QDemonRenderGPUProfiler
{
private:
    QDemonRef<QDemonRenderContext> m_renderContext;
    QDemonRef<QDemonRenderContextInterface> m_context;

    typedef QHash<QString, QDemonRef<QDemonGpuTimerInfo>> TStrGpuTimerInfoMap;

    TStrGpuTimerInfoMap m_strToGpuTimerMap;
    QVector<QString> m_timerIds;
    mutable quint32 m_vertexCount;

    QDemonRef<QDemonGpuTimerInfo> getOrCreateGpuTimerInfo(QString &nameID);
    QDemonRef<QDemonGpuTimerInfo> getGpuTimerInfo(const QString &nameID) const;

public:

    QDemonRenderGPUProfiler(const QDemonRef<QDemonRenderContextInterface> &inContext, const QDemonRef<QDemonRenderContext> &inRenderContext);
    ~QDemonRenderGPUProfiler();

    /**
     * @brief start a timer query
     *
     * @param[in] nameID			Timer ID for tracking
     * @param[in] absoluteTime		If true the absolute GPU is queried
     * @param[in] sync				Do a sync before starting the timer
     *
     * @return no return
     */
    void startTimer(QString &nameID, bool absoluteTime, bool sync);

    /**
     * @brief stop a timer query
     *
     * @param[in] nameID			Timer ID for tracking
     *
     * @return no return
     */
    void endTimer(QString &nameID);

    /**
     * @brief Get elapsed timer value. Not this is an averaged time over several frames
     *
     * @param[in] nameID			Timer ID for tracking
     *
     * @return no return
     */
    double elapsed(const QString &nameID) const;

    /**
     * @brief Get ID list of tracked timers
     *
     * @return ID list
     */
    const QVector<QString> &timerIDs() const;

    /**
     * @brief add vertex count to current counter
     *
     * @return
     */
    void addVertexCount(quint32 count);

    /**
     * @brief get current vertex count and reset
     *
     * @return
     */
    quint32 getAndResetVertexCount() const;
};
QT_END_NAMESPACE

#endif
