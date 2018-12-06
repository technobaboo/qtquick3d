/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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

#ifndef QDEMON_QML_RENDER_H
#define QDEMON_QML_RENDER_H

#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

QT_BEGIN_NAMESPACE

class IQt3DS;
class IQ3DSQmlStreamService;
class IQ3DSQmlStreamRenderer;

class Q3DSQmlRender : public IOffscreenRenderer
{
public:
    Q3DSQmlRender(IQt3DSRenderContext &inRenderContext, const char *asset);
    ~Q3DSQmlRender();

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(m_RenderContext.GetAllocator())

    CRegisteredString GetOffscreenRendererType() override;

    SOffscreenRendererEnvironment GetDesiredEnvironment(QVector2D inPresentationScaleFactor) override;

    // Returns true of this object needs to be rendered, false if this object is not dirty
    SOffscreenRenderFlags NeedsRender(const SOffscreenRendererEnvironment &inEnvironment,
                                      QVector2D inPresentationScaleFactor,
                                      const SRenderInstanceId instanceId) override;

    void Render(const SOffscreenRendererEnvironment &inEnvironment,
                QDemonRenderContext &inRenderContext, QVector2D inPresentationScaleFactor,
                SScene::RenderClearCommand inColorBufferNeedsClear,
                const SRenderInstanceId instanceId) override;
    void RenderWithClear(const SOffscreenRendererEnvironment &/*inEnvironment*/,
                         QDemonRenderContext &/*inRenderContext*/,
                         QVector2D /*inPresentationScaleFactor*/,
                         SScene::RenderClearCommand /*inColorBufferNeedsClear*/,
                         QVector3D /*inclearColor*/,
                         const SRenderInstanceId /*instanceId*/) override {}

    IGraphObjectPickQuery *GetGraphObjectPickQuery(const SRenderInstanceId instanceId) override
    {
        Q_UNUSED(instanceId)
        return nullptr;
    }
    bool Pick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions,
              const SRenderInstanceId instanceId) override
    {
        Q_UNUSED(inMouseCoords)
        Q_UNUSED(inViewportDimensions)
        Q_UNUSED(instanceId)
        return false;
    }
    void addCallback(IOffscreenRendererCallback *cb) override
    {
        m_callback = cb;
    }
    static const char *GetRendererName() { return "qml-render"; }
private:

    void initializeRenderer();

    IQt3DSRenderContext &m_RenderContext;
    IQ3DSQmlStreamRenderer *m_qmlStreamRenderer;
    CRegisteredString m_offscreenRenderType;
    CRegisteredString m_assetString;
    IOffscreenRendererCallback *m_callback;
    volatile qint32 mRefCount;
};

QT_END_NAMESPACE

#endif
