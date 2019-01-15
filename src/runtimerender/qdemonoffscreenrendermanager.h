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
class IResourceManager;
struct QDemonRenderPickResult;
class IGraphObjectPickQuery;
class IQDemonRenderContext;
class QDemonRenderContext;

struct OffscreenRendererDepthValues
{
    enum Enum {
        NoDepthBuffer = 0,
        Depth16, // 16 bit depth buffer
        Depth24, // 24 bit depth buffer
        Depth32, // 32 bit depth buffer
        Depth24Stencil8 // 24 bit depth buffer 8 bit stencil buffer
    };
};

struct SOffscreenRendererEnvironment
{
    quint32 m_Width;
    quint32 m_Height;
    QDemonRenderTextureFormats::Enum m_Format;
    OffscreenRendererDepthValues::Enum m_Depth;
    bool m_Stencil;
    AAModeValues::Enum m_MSAAMode;

    SOffscreenRendererEnvironment()
        : m_Width(0)
        , m_Height(0)
        , m_Format(QDemonRenderTextureFormats::Unknown)
        , m_Depth(OffscreenRendererDepthValues::NoDepthBuffer)
        , m_Stencil(false)
        , m_MSAAMode(AAModeValues::NoAA)
    {
    }

    SOffscreenRendererEnvironment(quint32 inWidth, quint32 inHeight,
                                  QDemonRenderTextureFormats::Enum inFormat)
        : m_Width(inWidth)
        , m_Height(inHeight)
        , m_Format(inFormat)
        , m_Depth(OffscreenRendererDepthValues::Depth16)
        , m_Stencil(false)
        , m_MSAAMode(AAModeValues::NoAA)
    {
    }

    SOffscreenRendererEnvironment(quint32 inWidth, quint32 inHeight,
                                  QDemonRenderTextureFormats::Enum inFormat,
                                  OffscreenRendererDepthValues::Enum inDepth, bool inStencil,
                                  AAModeValues::Enum inAAMode)
        : m_Width(inWidth)
        , m_Height(inHeight)
        , m_Format(inFormat)
        , m_Depth(inDepth)
        , m_Stencil(inStencil)
        , m_MSAAMode(inAAMode)
    {
    }

    SOffscreenRendererEnvironment(const SOffscreenRendererEnvironment &inOther)
        : m_Width(inOther.m_Width)
        , m_Height(inOther.m_Height)
        , m_Format(inOther.m_Format)
        , m_Depth(inOther.m_Depth)
        , m_Stencil(inOther.m_Stencil)
        , m_MSAAMode(inOther.m_MSAAMode)
    {
    }
};

struct SOffscreenRenderFlags
{
    bool m_HasTransparency;
    bool m_HasChangedSinceLastFrame;
    SOffscreenRenderFlags()
        : m_HasTransparency(false)
        , m_HasChangedSinceLastFrame(false)
    {
    }

    SOffscreenRenderFlags(bool transparency, bool hasChanged)
        : m_HasTransparency(transparency)
        , m_HasChangedSinceLastFrame(hasChanged)
    {
    }
};

typedef void *SRenderInstanceId;

class IOffscreenRenderer
{
public:
    class IOffscreenRendererCallback
    {
    public:
        virtual void onOffscreenRendererInitialized(const QString &id) = 0;
        virtual void onOffscreenRendererFrame(const QString &id) = 0;
    protected:
        virtual ~IOffscreenRendererCallback() {}
    };

    virtual ~IOffscreenRenderer();

public:
    virtual void addCallback(IOffscreenRendererCallback *cb) = 0;
    // Arbitrary const char* returned to indicate the type of this renderer
    // Can be overloaded to form the basis of an RTTI type system.
    // Not currently used by the rendering system.
    virtual QString GetOffscreenRendererType() = 0;
    virtual SOffscreenRendererEnvironment
    GetDesiredEnvironment(QVector2D inPresentationScaleFactor) = 0;
    // Returns true of this object needs to be rendered, false if this object is not dirty
    virtual SOffscreenRenderFlags
    NeedsRender(const SOffscreenRendererEnvironment &inEnvironment,
                QVector2D inPresentationScaleFactor,
                const SRenderInstanceId instanceId) = 0;
    // Returns true if the rendered result image has transparency, or false
    // if it should be treated as a completely opaque image.
    // It is the IOffscreenRenderer's job to clear any buffers (color, depth, stencil) that it
    // needs to.  It should not assume that it's buffers are clear;
    // Sometimes we scale the width and height of the main presentation in order to fit a
    // window.
    // If we do so, the scale factor tells the subpresentation renderer how much the system has
    // scaled.
    virtual void Render(const SOffscreenRendererEnvironment &inEnvironment,
                        QDemonRenderContext &inRenderContext, QVector2D inPresentationScaleFactor,
                        SScene::RenderClearCommand inColorBufferNeedsClear,
                        const SRenderInstanceId instanceId) = 0;
    virtual void RenderWithClear(const SOffscreenRendererEnvironment &inEnvironment,
                                 QDemonRenderContext &inRenderContext, QVector2D inPresentationScaleFactor,
                                 SScene::RenderClearCommand inColorBufferNeedsClear,
                                 QVector3D inclearColor,
                                 const SRenderInstanceId instanceId) = 0;

    // Implementors should implement one of the two interfaces below.

    // If this renderer supports picking that can return graph objects
    // then return an interface here.
    virtual IGraphObjectPickQuery *GetGraphObjectPickQuery(const SRenderInstanceId instanceId) = 0;

    // If you *don't* support the GraphObjectPickIterator interface, then you should implement
    // this interface
    // The system will just ask you to pick.
    // If you return true, then we will assume that you swallowed the pick and will continue no
    // further.
    // else we will assume you did not and will continue the picking algorithm.
    virtual bool Pick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions,
                      const SRenderInstanceId instanceId) = 0;
};

struct SOffscreenRenderResult
{
    QSharedPointer<IOffscreenRenderer> m_Renderer;
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    bool m_HasTransparency;
    bool m_HasChangedSinceLastFrame;

    SOffscreenRenderResult(QSharedPointer<IOffscreenRenderer> inRenderer, QSharedPointer<QDemonRenderTexture2D> inTexture,
                           bool inTrans, bool inDirty)
        : m_Renderer(inRenderer)
        , m_Texture(inTexture)
        , m_HasTransparency(inTrans)
        , m_HasChangedSinceLastFrame(inDirty)
    {
    }
    SOffscreenRenderResult()
        : m_Renderer(nullptr)
        , m_Texture(nullptr)
        , m_HasTransparency(false)
        , m_HasChangedSinceLastFrame(false)
    {
    }
};

struct SOffscreenRendererKey;

/**
     *	The offscreen render manager attempts to satisfy requests for a given image under a given
     *key.
     *	Renderers are throttled such that they render at most once per frame and potentially less
     *than
     *	that if they don't require a new render.
     */
class IOffscreenRenderManager
{
public:
    virtual ~IOffscreenRenderManager();
    // returns true if the renderer has not been registered.
    // No return value means there was an error registering this id.
    virtual QDemonOption<bool> MaybeRegisterOffscreenRenderer(const SOffscreenRendererKey &inKey,
                                                              QSharedPointer<IOffscreenRenderer> inRenderer) = 0;
    virtual void RegisterOffscreenRenderer(const SOffscreenRendererKey &inKey,
                                           QSharedPointer<IOffscreenRenderer> inRenderer) = 0;
    virtual bool HasOffscreenRenderer(const SOffscreenRendererKey &inKey) = 0;
    virtual QSharedPointer<IOffscreenRenderer> GetOffscreenRenderer(const SOffscreenRendererKey &inKey) = 0;
    virtual void ReleaseOffscreenRenderer(const SOffscreenRendererKey &inKey) = 0;

    // This doesn't trigger rendering right away.  A node is added to the render graph that
    // points to this item.
    // Thus rendering is deffered until the graph is run but we promise to render to this
    // resource.
    virtual SOffscreenRenderResult GetRenderedItem(const SOffscreenRendererKey &inKey) = 0;
    // Called by the UICRenderContext, clients don't need to call this.
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    static QSharedPointer<IOffscreenRenderManager> CreateOffscreenRenderManager(QSharedPointer<IResourceManager> inManager, QSharedPointer<IQDemonRenderContext> inContext);
};

QT_END_NAMESPACE
#endif
