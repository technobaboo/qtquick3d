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

typedef void *QDemonRenderInstanceId;

class QDemonRenderNodeFilterInterface
{
protected:
    virtual ~QDemonRenderNodeFilterInterface() {}
public:
    virtual bool includeNode(const QDemonGraphNode &inNode) = 0;
};
struct QDemonLayerPickSetup
{
    QMatrix4x4 projectionPreMultiply;
    QMatrix4x4 viewProjection;
    QRect scissorRect;
    QDemonLayerPickSetup(const QMatrix4x4 &inProjPreMult,
                         const QMatrix4x4 &inVP,
                         const QRect &inScissor)
        : projectionPreMultiply(inProjPreMult)
        , viewProjection(inVP)
        , scissorRect(inScissor)
    {
    }
    QDemonLayerPickSetup() {}
};

struct QDemonScaleAndPosition
{
    QVector3D position;
    float scale;
    QDemonScaleAndPosition(const QVector3D &inPos, float inScale)
        : position(inPos)
        , scale(inScale)
    {
    }
    QDemonScaleAndPosition() = default;
};

struct QDemonRenderLayer;
class QDemonRenderWidgetInterface;
class QDemonRenderWidgetContextInterface;
class QDemonRenderContextInterface;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonRendererInterface
{
protected:
    virtual ~QDemonRendererInterface() {}

public:
    virtual void enableLayerCaching(bool inEnabled) = 0;
    virtual bool isLayerCachingEnabled() const = 0;
    virtual void enableLayerGpuProfiling(bool inEnabled) = 0;
    virtual bool isLayerGpuProfilingEnabled() const = 0;

    // Get the camera that rendered this node last render
    virtual QDemonRenderCamera *getCameraForNode(const QDemonGraphNode &inNode) const = 0;
    virtual QDemonOption<QDemonCuboidRect> getCameraBounds(const QDemonGraphObject &inObject) = 0;
    // Called when you have changed the number or order of children of a given node.
    virtual void childrenUpdated(QDemonGraphNode &inParent) = 0;
    virtual float getTextScale(const QDemonText &inText) = 0;

    // The QDemonRenderContextInterface calls these, clients should not.
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // Setup the vertex and index buffers (but not shader state)
    // and render the quad.  The quad is setup so that its edges
    // go from -1,1 in x,y and its UV coordinates will map naturally
    // to an image.
    virtual void renderQuad() = 0;

    // Render a given texture to the scene using a given transform.
    virtual void renderQuad(const QVector2D inDimensions,
                            const QMatrix4x4 &inMVP,
                            QDemonRenderTexture2D &inQuadTexture) = 0;

    // This point rendering works uisng indirect array drawing
    // This means you need to setup a GPU buffer
    // which contains the drawing information
    virtual void renderPointsIndirect() = 0;

    // Returns true if this layer or a sibling was dirty.
    virtual bool prepareLayerForRender(QDemonRenderLayer &inLayer,
                                       const QVector2D &inViewportDimensions,
                                       bool inRenderSiblings = true,
                                       const QDemonRenderInstanceId id = nullptr) = 0;
    virtual void renderLayer(QDemonRenderLayer &inLayer,
                             const QVector2D &inViewportDimensions,
                             bool clear,
                             QVector3D clearColor,
                             bool inRenderSiblings = true,
                             const QDemonRenderInstanceId id = nullptr) = 0;

    // Studio option to disable picking against sub renderers.  This allows better interaction
    // in studio.
    // In pick siblings measn pick the layer siblings; this is the normal behavior.
    // InPickEverything means ignore the node's pick flags; this allows us to only pick things
    // that have handlers
    // in some cases and just pick everything in other things.
    virtual void pickRenderPlugins(bool inPick) = 0;
    virtual QDemonRenderPickResult pick(QDemonRenderLayer &inLayer,
                                        const QVector2D &inViewportDimensions,
                                        const QVector2D &inMouseCoords,
                                        bool inPickSiblings = true,
                                        bool inPickEverything = false,
                                        const QDemonRenderInstanceId id = nullptr) = 0;

    // Return the relative hit position, in UV space, of a mouse pick against this object.
    // We need the node in order to figure out which layer rendered this object.
    // We need mapper objects if this is a in a subpresentation because we have to know how
    // to map the mouse coordinates into the subpresentation.  So for instance if inNode is in
    // a subpres then we need to know which image is displaying the subpres in order to map
    // the mouse coordinates into the subpres's render space.
    virtual QDemonOption<QVector2D> facePosition(QDemonGraphNode &inNode, QDemonBounds3 inBounds,
                                                 const QMatrix4x4 &inGlobalTransform,
                                                 const QVector2D &inViewportDimensions,
                                                 const QVector2D &inMouseCoords,
                                                 QDemonDataRef<QDemonGraphObject *> inMapperObjects,
                                                 QDemonRenderBasisPlanes::Enum inIsectPlane) = 0;

    virtual QVector3D unprojectToPosition(QDemonGraphNode &inNode, QVector3D &inPosition,
                                          const QVector2D &inMouseVec) const = 0;
    virtual QVector3D unprojectWithDepth(QDemonGraphNode &inNode, QVector3D &inPosition,
                                         const QVector3D &inMouseVec) const = 0;
    virtual QVector3D projectPosition(QDemonGraphNode &inNode, const QVector3D &inPosition) const = 0;

    // Roughly equivalent of gluPickMatrix, allows users to setup a perspective transform that
    // will draw some sub component
    // of the layer.  Used in combination with an expected viewport of 0,0,width,height the
    // viewproj matrix returned will center
    // around the center of the viewport and render just the part of the layer around this area.
    // The return value is optional because if the mouse point is completely outside the layer
    // obviously this method is irrelevant.
    virtual QDemonOption<QDemonLayerPickSetup> getLayerPickSetup(QDemonRenderLayer &inLayer,
                                                            const QVector2D &inMouseCoords,
                                                            const QSize &inPickDims) = 0;

    // Return the layer's viewport rect after the layer's member variables have been applied.
    // Uses the last rendered viewport rect.
    virtual QDemonOption<QRectF> getLayerRect(QDemonRenderLayer &inLayer) = 0;
    // Testing function to allow clients to render a layer using a custom view project instead
    // of the one that would be setup
    // using the layer's camera in conjunction with the layer's position,scale.
    virtual void runLayerRender(QDemonRenderLayer &inLayer, const QMatrix4x4 &inViewProjection) = 0;

    // Render the layer's rect onscreen.  Will only render one frame, you need to call this
    // every frame
    // for this to work and be persistent.
    virtual void renderLayerRect(QDemonRenderLayer &inLayer, const QVector3D &inColor) = 0;
    // Render widgets are things that are draw on the layer's widget texture which is then
    // rendered to the
    // scene's widget texture.  You must add them every frame you wish them to be rendered; the
    // list of
    // widgets is cleared every frame.
    virtual void addRenderWidget(QDemonRenderWidgetInterface &inWidget) = 0;

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
    virtual QDemonScaleAndPosition getWorldToPixelScaleFactor(QDemonRenderLayer &inLayer,
                                                         const QVector3D &inWorldPoint) = 0;
    // Called before a layer goes completely out of scope to release any rendering resources
    // related to the layer.
    virtual void releaseLayerRenderResources(QDemonRenderLayer &inLayer, const QDemonRenderInstanceId id) = 0;

    // render a screen aligned 2D text
    virtual void renderText2D(float x, float y, QDemonOption<QVector3D> inColor, const QString &text) = 0;
    // render Gpu profiler values
    virtual void renderGpuProfilerStats(float x, float y, QDemonOption<QVector3D> inColor) = 0;

    // Get the mouse coordinates as they relate to a given layer
    virtual QDemonOption<QVector2D> getLayerMouseCoords(QDemonRenderLayer &inLayer,
                                                        const QVector2D &inMouseCoords,
                                                        const QVector2D &inViewportDimensions,
                                                        bool forceImageIntersect = false) const = 0;

    virtual QSharedPointer<QDemonRenderWidgetContextInterface> getRenderWidgetContext() = 0;

    static bool isGlEsContext(QDemonRenderContextType inContextType);
    static bool isGlEs3Context(QDemonRenderContextType inContextType);
    static bool isGl2Context(QDemonRenderContextType inContextType);
    static const char *getGlslVesionString(QDemonRenderContextType inContextType);

    static QSharedPointer<QDemonRendererInterface> createRenderer(QDemonRenderContextInterface *inContext);
};
QT_END_NAMESPACE

#endif
