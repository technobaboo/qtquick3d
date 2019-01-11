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
#ifndef QDEMON_RENDERER_H
#define QDEMON_RENDERER_H

#include <QtDemon/qdemondataref.h>
#include <QtDemon/qdemonflags.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobjectpickquery.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderray.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>

#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

typedef void *SRenderInstanceId;

class IQDemonRenderNodeFilter
{
protected:
    virtual ~IQDemonRenderNodeFilter() {}
public:
    virtual bool IncludeNode(const SNode &inNode) = 0;
};
struct SLayerPickSetup
{
    QMatrix4x4 m_ProjectionPreMultiply;
    QMatrix4x4 m_ViewProjection;
    QDemonRenderRect m_ScissorRect;
    SLayerPickSetup(const QMatrix4x4 &inProjPreMult, const QMatrix4x4 &inVP,
                    const QDemonRenderRect &inScissor)
        : m_ProjectionPreMultiply(inProjPreMult)
        , m_ViewProjection(inVP)
        , m_ScissorRect(inScissor)
    {
    }
    SLayerPickSetup() {}
};

struct SScaleAndPosition
{
    QVector3D m_Position;
    float m_Scale;
    SScaleAndPosition(const QVector3D &inPos, float inScale)
        : m_Position(inPos)
        , m_Scale(inScale)
    {
    }
    SScaleAndPosition() {}
};

struct SLayer;
class IRenderWidget;
class IRenderWidgetContext;
class IQDemonRenderContext;

class IQDemonRenderer
{
protected:
    virtual ~IQDemonRenderer() {}

public:
    virtual void EnableLayerCaching(bool inEnabled) = 0;
    virtual bool IsLayerCachingEnabled() const = 0;
    virtual void EnableLayerGpuProfiling(bool inEnabled) = 0;
    virtual bool IsLayerGpuProfilingEnabled() const = 0;

    // Get the camera that rendered this node last render
    virtual SCamera *GetCameraForNode(const SNode &inNode) const = 0;
    virtual QDemonOption<SCuboidRect> GetCameraBounds(const SGraphObject &inObject) = 0;
    // Called when you have changed the number or order of children of a given node.
    virtual void ChildrenUpdated(SNode &inParent) = 0;
    virtual float GetTextScale(const SText &inText) = 0;

    // The IQDemonRenderContext calls these, clients should not.
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;

    // Setup the vertex and index buffers (but not shader state)
    // and render the quad.  The quad is setup so that its edges
    // go from -1,1 in x,y and its UV coordinates will map naturally
    // to an image.
    virtual void RenderQuad() = 0;

    // Render a given texture to the scene using a given transform.
    virtual void RenderQuad(const QVector2D inDimensions, const QMatrix4x4 &inMVP,
                            QDemonRenderTexture2D &inQuadTexture) = 0;

    // This point rendering works uisng indirect array drawing
    // This means you need to setup a GPU buffer
    // which contains the drawing information
    virtual void RenderPointsIndirect() = 0;

    // Returns true if this layer or a sibling was dirty.
    virtual bool PrepareLayerForRender(SLayer &inLayer, const QVector2D &inViewportDimensions,
                                       bool inRenderSiblings = true,
                                       const SRenderInstanceId id = nullptr) = 0;
    virtual void RenderLayer(SLayer &inLayer, const QVector2D &inViewportDimensions, bool clear,
                             QVector3D clearColor, bool inRenderSiblings = true,
                             const SRenderInstanceId id = nullptr) = 0;

    // Studio option to disable picking against sub renderers.  This allows better interaction
    // in studio.
    // In pick siblings measn pick the layer siblings; this is the normal behavior.
    // InPickEverything means ignore the node's pick flags; this allows us to only pick things
    // that have handlers
    // in some cases and just pick everything in other things.
    virtual void PickRenderPlugins(bool inPick) = 0;
    virtual QDemonRenderPickResult Pick(SLayer &inLayer, const QVector2D &inViewportDimensions,
                                       const QVector2D &inMouseCoords, bool inPickSiblings = true,
                                       bool inPickEverything = false,
                                       const SRenderInstanceId id = nullptr) = 0;

    // Return the relative hit position, in UV space, of a mouse pick against this object.
    // We need the node in order to figure out which layer rendered this object.
    // We need mapper objects if this is a in a subpresentation because we have to know how
    // to map the mouse coordinates into the subpresentation.  So for instance if inNode is in
    // a subpres then we need to know which image is displaying the subpres in order to map
    // the mouse coordinates into the subpres's render space.
    virtual QDemonOption<QVector2D> FacePosition(SNode &inNode, QDemonBounds3 inBounds,
                                           const QMatrix4x4 &inGlobalTransform,
                                           const QVector2D &inViewportDimensions,
                                           const QVector2D &inMouseCoords,
                                           QDemonDataRef<SGraphObject *> inMapperObjects,
                                           SBasisPlanes::Enum inIsectPlane) = 0;

    virtual QVector3D UnprojectToPosition(SNode &inNode, QVector3D &inPosition,
                                          const QVector2D &inMouseVec) const = 0;
    virtual QVector3D UnprojectWithDepth(SNode &inNode, QVector3D &inPosition,
                                         const QVector3D &inMouseVec) const = 0;
    virtual QVector3D ProjectPosition(SNode &inNode, const QVector3D &inPosition) const = 0;

    // Roughly equivalent of gluPickMatrix, allows users to setup a perspective transform that
    // will draw some sub component
    // of the layer.  Used in combination with an expected viewport of 0,0,width,height the
    // viewproj matrix returned will center
    // around the center of the viewport and render just the part of the layer around this area.
    // The return value is optional because if the mouse point is completely outside the layer
    // obviously this method is irrelevant.
    virtual QDemonOption<SLayerPickSetup> GetLayerPickSetup(SLayer &inLayer,
                                                      const QVector2D &inMouseCoords,
                                                      const QSize &inPickDims) = 0;

    // Return the layer's viewport rect after the layer's member variables have been applied.
    // Uses the last rendered viewport rect.
    virtual QDemonOption<QDemonRenderRectF> GetLayerRect(SLayer &inLayer) = 0;
    // Testing function to allow clients to render a layer using a custom view project instead
    // of the one that would be setup
    // using the layer's camera in conjunction with the layer's position,scale.
    virtual void RunLayerRender(SLayer &inLayer, const QMatrix4x4 &inViewProjection) = 0;

    // Render the layer's rect onscreen.  Will only render one frame, you need to call this
    // every frame
    // for this to work and be persistent.
    virtual void RenderLayerRect(SLayer &inLayer, const QVector3D &inColor) = 0;
    // Render widgets are things that are draw on the layer's widget texture which is then
    // rendered to the
    // scene's widget texture.  You must add them every frame you wish them to be rendered; the
    // list of
    // widgets is cleared every frame.
    virtual void AddRenderWidget(IRenderWidget &inWidget) = 0;

    // Get a scale factor so you can have objects precisely 50 pixels.  Note that this scale
    // factor
    // only applies to things drawn parallel to the camera plane; If you aren't parallel then
    // there isn't
    // a single scale factor that will work.
    // For perspective-rendered objects, we shift the object forward or backwards along the
    // vector from the camera
    // to the object so that we are working in a consistent mathematical space.  So if the
    // camera is orthographic,
    // you are done.
    // If the camera is perspective, then this method will tell you want you need to scale
    // things by to account for
    // the FOV and also where the origin of the object needs to be to ensure the scale factor is
    // relevant.
    virtual SScaleAndPosition GetWorldToPixelScaleFactor(SLayer &inLayer,
                                                         const QVector3D &inWorldPoint) = 0;
    // Called before a layer goes completely out of scope to release any rendering resources
    // related to the layer.
    virtual void ReleaseLayerRenderResources(SLayer &inLayer, const SRenderInstanceId id) = 0;

    // render a screen aligned 2D text
    virtual void RenderText2D(float x, float y, QDemonOption<QVector3D> inColor,
                              const char *text) = 0;
    // render Gpu profiler values
    virtual void RenderGpuProfilerStats(float x, float y,
                                        QDemonOption<QVector3D> inColor) = 0;

    // Get the mouse coordinates as they relate to a given layer
    virtual QDemonOption<QVector2D> GetLayerMouseCoords(SLayer &inLayer, const QVector2D &inMouseCoords,
                                                  const QVector2D &inViewportDimensions,
                                                  bool forceImageIntersect = false) const = 0;

    virtual IRenderWidgetContext &GetRenderWidgetContext() = 0;

    static bool IsGlEsContext(QDemonRenderContextType inContextType);
    static bool IsGlEs3Context(QDemonRenderContextType inContextType);
    static bool IsGl2Context(QDemonRenderContextType inContextType);
    static const char *GetGlslVesionString(QDemonRenderContextType inContextType);

    static QSharedPointer<IQDemonRenderer> CreateRenderer(IQDemonRenderContext &inContext);
};
QT_END_NAMESPACE

#endif
