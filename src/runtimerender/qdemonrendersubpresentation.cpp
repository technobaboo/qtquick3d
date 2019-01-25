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
#include <qdemonrendersubpresentation.h>
#include <qdemonrenderrenderlist.h>
#ifdef _WIN32
#pragma warning(disable : 4355) // this used in initializer list.  I have never seen this result in
// a physical error
#endif
QT_BEGIN_NAMESPACE

QDemonRenderPickResult CSubPresentationPickQuery::Pick(const QVector2D &inMouseCoords,
                                                      const QVector2D &inViewportDimensions,
                                                      bool inPickEverything)
{
    return m_Renderer.DoGraphQueryPick(inMouseCoords, inViewportDimensions, inPickEverything);
}

CSubPresentationRenderer::CSubPresentationRenderer(IQDemonRenderContext *inRenderContext,
                                                   QSharedPointer<SPresentation> inPresentation)
    : m_RenderContext(inRenderContext)
    , m_Presentation(inPresentation)
    , m_PickQuery(*this)
    , m_OffscreenRendererType(QString::fromLocal8Bit(GetRendererName()))
{
}

SOffscreenRendererEnvironment
CSubPresentationRenderer::GetDesiredEnvironment(QVector2D /*inPresScale*/)
{
    // If we aren't using a clear color, then we are expected to blend with the background
    bool hasTransparency = m_Presentation->m_Scene->m_UseClearColor ? false : true;
    QDemonRenderTextureFormats::Enum format =
            hasTransparency ? QDemonRenderTextureFormats::RGBA8 : QDemonRenderTextureFormats::RGB8;
    return SOffscreenRendererEnvironment((quint32)(m_Presentation->m_PresentationDimensions.x()),
                                         (quint32)(m_Presentation->m_PresentationDimensions.y()),
                                         format, OffscreenRendererDepthValues::Depth16, false,
                                         AAModeValues::NoAA);
}

SOffscreenRenderFlags
CSubPresentationRenderer::NeedsRender(const SOffscreenRendererEnvironment & /*inEnvironment*/,
                                      QVector2D /*inPresScale*/,
                                      const SRenderInstanceId instanceId)
{
    bool hasTransparency = m_Presentation->m_Scene->m_UseClearColor ? false : true;
    QDemonRenderRect theViewportSize(m_RenderContext->GetRenderList()->GetViewport());
    bool wasDirty = m_Presentation->m_Scene->PrepareForRender(
                QVector2D((float)theViewportSize.m_Width, (float)theViewportSize.m_Height),
                m_RenderContext, instanceId);
    return SOffscreenRenderFlags(hasTransparency, wasDirty);
}

// Returns true if the rendered result image has transparency, or false
// if it should be treated as a completely opaque image.
void CSubPresentationRenderer::Render(const SOffscreenRendererEnvironment &inEnvironment,
                                      QDemonRenderContext &inRenderContext, QVector2D,
                                      SScene::RenderClearCommand inClearColorBuffer,
                                      const SRenderInstanceId instanceId)
{
    SSubPresentationHelper theHelper(m_RenderContext, QSize((quint32)inEnvironment.m_Width, (quint32)inEnvironment.m_Height));
    QDemonRenderRect theViewportSize(inRenderContext.GetViewport());
    m_Presentation->m_Scene->Render(
                QVector2D((float)theViewportSize.m_Width, (float)theViewportSize.m_Height),
                m_RenderContext, inClearColorBuffer, instanceId);
    m_LastRenderedEnvironment = inEnvironment;
}

void CSubPresentationRenderer::RenderWithClear(
        const SOffscreenRendererEnvironment &inEnvironment,
        QDemonRenderContext &inRenderContext, QVector2D inPresScale,
        SScene::RenderClearCommand inClearBuffer, QVector3D inClearColor,
        const SRenderInstanceId id)
{
    Q_UNUSED(inEnvironment);
    Q_UNUSED(inPresScale);
    QDemonRenderRect theViewportSize(inRenderContext.GetViewport());
    m_Presentation->m_Scene->RenderWithClear(
                QVector2D((float)theViewportSize.m_Width, (float)theViewportSize.m_Height),
                m_RenderContext, inClearBuffer, inClearColor, id);
}

// You know the viewport dimensions because
QDemonRenderPickResult CSubPresentationRenderer::DoGraphQueryPick(
        const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, bool inPickEverything)
{
    QDemonRenderPickResult thePickResult;

    if (m_Presentation->m_Scene && m_Presentation->m_Scene->m_FirstChild) {
        thePickResult = m_RenderContext->GetRenderer()->Pick(
                    *m_Presentation->m_Scene->m_FirstChild, inViewportDimensions,
                    QVector2D(inMouseCoords.x(), inMouseCoords.y()), true, inPickEverything);
    }
    return thePickResult;
}
QT_END_NAMESPACE
