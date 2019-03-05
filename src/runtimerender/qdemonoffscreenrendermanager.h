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
#ifndef QDEMON_OFFSCREEN_RENDER_MANAGER_H
#define QDEMON_OFFSCREEN_RENDER_MANAGER_H

#include <QtDemon/qdemonoption.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>

QT_BEGIN_NAMESPACE
class QDemonResourceManagerInterface;
struct QDemonRenderPickResult;
class QDemonGraphObjectPickQueryInterface;
class QDemonRenderContextInterface;
class QDemonRenderContext;

struct QDemonOffscreenRendererDepthValues
{
    enum Enum {
        NoDepthBuffer = 0,
        Depth16, // 16 bit depth buffer
        Depth24, // 24 bit depth buffer
        Depth32, // 32 bit depth buffer
        Depth24Stencil8 // 24 bit depth buffer 8 bit stencil buffer
    };
};

struct QDemonOffscreenRendererEnvironment
{
    quint32 width;
    quint32 height;
    QDemonRenderTextureFormats::Enum format;
    QDemonOffscreenRendererDepthValues::Enum depth;
    bool stencil;
    AAModeValues::Enum msaaMode;

    QDemonOffscreenRendererEnvironment()
        : width(0)
        , height(0)
        , format(QDemonRenderTextureFormats::Unknown)
        , depth(QDemonOffscreenRendererDepthValues::NoDepthBuffer)
        , stencil(false)
        , msaaMode(AAModeValues::NoAA)
    {
    }

    QDemonOffscreenRendererEnvironment(quint32 inWidth, quint32 inHeight, QDemonRenderTextureFormats::Enum inFormat)
        : width(inWidth)
        , height(inHeight)
        , format(inFormat)
        , depth(QDemonOffscreenRendererDepthValues::Depth16)
        , stencil(false)
        , msaaMode(AAModeValues::NoAA)
    {
    }

    QDemonOffscreenRendererEnvironment(quint32 inWidth,
                                       quint32 inHeight,
                                       QDemonRenderTextureFormats::Enum inFormat,
                                       QDemonOffscreenRendererDepthValues::Enum inDepth,
                                       bool inStencil,
                                       AAModeValues::Enum inAAMode)
        : width(inWidth), height(inHeight), format(inFormat), depth(inDepth), stencil(inStencil), msaaMode(inAAMode)
    {
    }

    QDemonOffscreenRendererEnvironment(const QDemonOffscreenRendererEnvironment &inOther)
        : width(inOther.width)
        , height(inOther.height)
        , format(inOther.format)
        , depth(inOther.depth)
        , stencil(inOther.stencil)
        , msaaMode(inOther.msaaMode)
    {
    }
};

struct QDemonOffscreenRenderFlags
{
    bool hasTransparency = false;
    bool hasChangedSinceLastFrame = false;
    constexpr QDemonOffscreenRenderFlags() = default;
    constexpr QDemonOffscreenRenderFlags(bool transparency, bool hasChanged)
        : hasTransparency(transparency), hasChangedSinceLastFrame(hasChanged)
    {
    }
};

typedef void *QDemonRenderInstanceId;

class QDemonOffscreenRendererInterface
{
public:
    QAtomicInt ref;
    class QDemonOffscreenRendererCallbackInterface
    {
    public:
        virtual void onOffscreenRendererInitialized(const QString &id) = 0;
        virtual void onOffscreenRendererFrame(const QString &id) = 0;

    protected:
        virtual ~QDemonOffscreenRendererCallbackInterface();
    };

    virtual ~QDemonOffscreenRendererInterface();

public:
    virtual void addCallback(QDemonOffscreenRendererCallbackInterface *cb) = 0;
    // Arbitrary const char* returned to indicate the type of this renderer
    // Can be overloaded to form the basis of an RTTI type system.
    // Not currently used by the rendering system.
    virtual QString getOffscreenRendererType() = 0;
    virtual QDemonOffscreenRendererEnvironment getDesiredEnvironment(QVector2D inPresentationScaleFactor) = 0;
    // Returns true of this object needs to be rendered, false if this object is not dirty
    virtual QDemonOffscreenRenderFlags needsRender(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                                   QVector2D inPresentationScaleFactor,
                                                   const QDemonRenderInstanceId instanceId) = 0;
    // Returns true if the rendered result image has transparency, or false
    // if it should be treated as a completely opaque image.
    // It is the IOffscreenRenderer's job to clear any buffers (color, depth, stencil) that it
    // needs to.  It should not assume that it's buffers are clear;
    // Sometimes we scale the width and height of the main presentation in order to fit a
    // window.
    // If we do so, the scale factor tells the subpresentation renderer how much the system has
    // scaled.
    virtual void render(const QDemonOffscreenRendererEnvironment &inEnvironment,
                        QDemonRenderContext &inRenderContext,
                        QVector2D inPresentationScaleFactor,
                        QDemonRenderScene::RenderClearCommand inColorBufferNeedsClear,
                        const QDemonRenderInstanceId instanceId) = 0;
    virtual void renderWithClear(const QDemonOffscreenRendererEnvironment &inEnvironment,
                                 QDemonRenderContext &inRenderContext,
                                 QVector2D inPresentationScaleFactor,
                                 QDemonRenderScene::RenderClearCommand inColorBufferNeedsClear,
                                 QVector3D inclearColor,
                                 const QDemonRenderInstanceId instanceId) = 0;

    // Implementors should implement one of the two interfaces below.

    // If this renderer supports picking that can return graph objects
    // then return an interface here.
    virtual QDemonGraphObjectPickQueryInterface *getGraphObjectPickQuery(const QDemonRenderInstanceId instanceId) = 0;

    // If you *don't* support the GraphObjectPickIterator interface, then you should implement
    // this interface
    // The system will just ask you to pick.
    // If you return true, then we will assume that you swallowed the pick and will continue no
    // further.
    // else we will assume you did not and will continue the picking algorithm.
    virtual bool pick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, const QDemonRenderInstanceId instanceId) = 0;
};

struct QDemonOffscreenRenderResult
{
    QDemonRef<QDemonOffscreenRendererInterface> renderer;
    QDemonRef<QDemonRenderTexture2D> texture;
    bool hasTransparency = false;
    bool hasChangedSinceLastFrame = false;

    QDemonOffscreenRenderResult(QDemonRef<QDemonOffscreenRendererInterface> inRenderer,
                                QDemonRef<QDemonRenderTexture2D> inTexture,
                                bool inTrans,
                                bool inDirty)
        : renderer(inRenderer), texture(inTexture), hasTransparency(inTrans), hasChangedSinceLastFrame(inDirty)
    {
    }
    QDemonOffscreenRenderResult() = default;
};

struct QDemonOffscreenRendererKey;

/**
 *	The offscreen render manager attempts to satisfy requests for a given image under a given
 *key.
 *	Renderers are throttled such that they render at most once per frame and potentially less
 *than
 *	that if they don't require a new render.
 */
class Q_DEMONRUNTIMERENDER_EXPORT QDemonOffscreenRenderManagerInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonOffscreenRenderManagerInterface();
    // returns true if the renderer has not been registered.
    // No return value means there was an error registering this id.
    virtual QDemonOption<bool> maybeRegisterOffscreenRenderer(const QDemonOffscreenRendererKey &inKey,
                                                              QDemonRef<QDemonOffscreenRendererInterface> inRenderer) = 0;
    virtual void registerOffscreenRenderer(const QDemonOffscreenRendererKey &inKey,
                                           QDemonRef<QDemonOffscreenRendererInterface> inRenderer) = 0;
    virtual bool hasOffscreenRenderer(const QDemonOffscreenRendererKey &inKey) = 0;
    virtual QDemonRef<QDemonOffscreenRendererInterface> getOffscreenRenderer(const QDemonOffscreenRendererKey &inKey) = 0;
    virtual void releaseOffscreenRenderer(const QDemonOffscreenRendererKey &inKey) = 0;

    // This doesn't trigger rendering right away.  A node is added to the render graph that
    // points to this item.
    // Thus rendering is deffered until the graph is run but we promise to render to this
    // resource.
    virtual QDemonOffscreenRenderResult getRenderedItem(const QDemonOffscreenRendererKey &inKey) = 0;
    // Called by the UICRenderContext, clients don't need to call this.
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    static QDemonRef<QDemonOffscreenRenderManagerInterface> createOffscreenRenderManager(const QDemonRef<QDemonResourceManagerInterface> &inManager,
                                                                                         QDemonRenderContextInterface *inContext);
};

QT_END_NAMESPACE
#endif
