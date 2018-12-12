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
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>

#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendercontext.h>

#include <QtDemon/qdemonutils.h>

#include <QtGui/QVector2D>

#include <qmath.h>

QT_BEGIN_NAMESPACE

namespace {

float GetAspectRatio(const QDemonRenderRectF &inViewport)
{
    return inViewport.m_Height != 0 ? inViewport.m_Width / inViewport.m_Height : 0.0f;
}

float GetAspectRatio(const QVector2D &inDimensions)
{
    return inDimensions.y() != 0 ? inDimensions.x() / inDimensions.y() : 0.0f;
}

bool IsCameraVerticalAdjust(CameraScaleModes::Enum inMode, float inDesignAspect,
                            float inActualAspect)
{
    return (inMode == CameraScaleModes::Fit && inActualAspect >= inDesignAspect)
        || inMode == CameraScaleModes::FitVertical;
}

bool IsCameraHorizontalAdjust(CameraScaleModes::Enum inMode, float inDesignAspect,
                              float inActualAspect)
{
    return (inMode == CameraScaleModes::Fit && inActualAspect < inDesignAspect)
        || inMode == CameraScaleModes::FitHorizontal;
}

bool IsFitTypeScaleMode(CameraScaleModes::Enum inMode)
{
    return inMode == CameraScaleModes::Fit || inMode == CameraScaleModes::FitHorizontal
        || inMode == CameraScaleModes::FitVertical;
}

struct SPinCameraResult
{
    QDemonRenderRectF m_Viewport;
    QDemonRenderRectF m_VirtualViewport;
    SPinCameraResult(QDemonRenderRectF v, QDemonRenderRectF vv)
        : m_Viewport(v)
        , m_VirtualViewport(vv)
    {
    }
};
// Scale and transform the projection matrix to respect the camera anchor attribute
// and the scale mode.
SPinCameraResult PinCamera(const QDemonRenderRectF &inViewport, QVector2D inDesignDims,
                           QMatrix4x4 &ioPerspectiveMatrix, CameraScaleModes::Enum inScaleMode,
                           CameraScaleAnchors::Enum inPinLocation)
{
    QDemonRenderRectF viewport(inViewport);
    QDemonRenderRectF idealViewport(inViewport.m_X, inViewport.m_Y, inDesignDims.x(), inDesignDims.y());
    float designAspect = GetAspectRatio(inDesignDims);
    float actualAspect = GetAspectRatio(inViewport);
    if (IsFitTypeScaleMode(inScaleMode)) {
        idealViewport.m_Width = viewport.m_Width;
        idealViewport.m_Height = viewport.m_Height;
    }
    // We move the viewport such that the left, top of the presentation sits against the left top
    // edge
    // We only need to translate in X *if* our actual aspect > design aspect
    // And then we only need to account for whatever centering would happen.

    bool pinLeft = inPinLocation == CameraScaleAnchors::SouthWest
        || inPinLocation == CameraScaleAnchors::West
        || inPinLocation == CameraScaleAnchors::NorthWest;
    bool pinRight = inPinLocation == CameraScaleAnchors::SouthEast
        || inPinLocation == CameraScaleAnchors::East
        || inPinLocation == CameraScaleAnchors::NorthEast;
    bool pinTop = inPinLocation == CameraScaleAnchors::NorthWest
        || inPinLocation == CameraScaleAnchors::North
        || inPinLocation == CameraScaleAnchors::NorthEast;
    bool pinBottom = inPinLocation == CameraScaleAnchors::SouthWest
        || inPinLocation == CameraScaleAnchors::South
        || inPinLocation == CameraScaleAnchors::SouthEast;

    if (inScaleMode == CameraScaleModes::SameSize) {
        // In this case the perspective transform does not center the view,
        // it places it in the lower-left of the viewport.
        float idealWidth = inDesignDims.x();
        float idealHeight = inDesignDims.y();
        if (pinRight)
            idealViewport.m_X -= ((idealWidth - inViewport.m_Width));
        else if (!pinLeft)
            idealViewport.m_X -= ((idealWidth - inViewport.m_Width) / 2.0f);

        if (pinTop)
            idealViewport.m_Y -= ((idealHeight - inViewport.m_Height));
        else if (!pinBottom)
            idealViewport.m_Y -= ((idealHeight - inViewport.m_Height) / 2.0f);
    } else {
        // In this case our perspective matrix will center the view and we need to decenter
        // it as necessary
        // if we are wider than we are high
        if (IsCameraVerticalAdjust(inScaleMode, designAspect, actualAspect)) {
            if (pinLeft || pinRight) {
                float idealWidth = inViewport.m_Height * designAspect;
                qint32 halfOffset = (qint32)((idealWidth - inViewport.m_Width) / 2.0f);
                halfOffset = pinLeft ? halfOffset : -1 * halfOffset;
                idealViewport.m_X += halfOffset;
            }
        } else {
            if (pinTop || pinBottom) {
                float idealHeight = inViewport.m_Width / designAspect;
                qint32 halfOffset = (qint32)((idealHeight - inViewport.m_Height) / 2.0f);
                halfOffset = pinBottom ? halfOffset : -1 * halfOffset;
                idealViewport.m_Y += halfOffset;
            }
        }
    }

    ioPerspectiveMatrix = QDemonRenderContext::ApplyVirtualViewportToProjectionMatrix(
        ioPerspectiveMatrix, viewport, idealViewport);
    return SPinCameraResult(viewport, idealViewport);
}
}

SCamera::SCamera()
    : SNode(GraphObjectTypes::Camera)
    , m_ClipNear(10)
    , m_ClipFar(10000)
    , m_FOV(60)
    , m_FOVHorizontal(false)
    , m_ScaleMode(CameraScaleModes::Fit)
    , m_ScaleAnchor(CameraScaleAnchors::Center)
{
    TORAD(m_FOV);
    m_Projection = QMatrix4x4();
    m_Position = QVector3D(0, 0, -600);
}

// Code for testing
SCameraGlobalCalculationResult SCamera::CalculateGlobalVariables(const QDemonRenderRectF &inViewport,
                                                                 const QVector2D &inDesignDimensions)
{
    bool wasDirty = SNode::CalculateGlobalVariables();
    return SCameraGlobalCalculationResult(wasDirty,
                                          CalculateProjection(inViewport, inDesignDimensions));
}

bool SCamera::CalculateProjection(const QDemonRenderRectF &inViewport, const QVector2D &inDesignDimensions)
{
    bool retval = false;
    if (m_Flags.IsOrthographic())
        retval = ComputeFrustumOrtho(inViewport, inDesignDimensions);
    else
        retval = ComputeFrustumPerspective(inViewport, inDesignDimensions);
    if (retval) {
        float *writePtr(m_Projection.data());
        m_FrustumScale.setX(writePtr[0]);
        m_FrustumScale.setY(writePtr[5]);
        PinCamera(inViewport, inDesignDimensions, m_Projection, m_ScaleMode, m_ScaleAnchor);
    }
    return retval;
}

//==============================================================================
/**
 *	Compute the projection matrix for a perspective camera
 *	@return true if the computed projection matrix is valid
 */
bool SCamera::ComputeFrustumPerspective(const QDemonRenderRectF &inViewport,
                                        const QVector2D &inDesignDimensions)
{
    m_Projection = QMatrix4x4();
    float theAngleInRadians = verticalFov(inViewport) / 2.0f;
    float theDeltaZ = m_ClipFar - m_ClipNear;
    float theSine = sinf(theAngleInRadians);
    float designAspect = GetAspectRatio(inDesignDimensions);
    float theAspectRatio = designAspect;
    if (IsFitTypeScaleMode(m_ScaleMode))
        theAspectRatio = GetAspectRatio(inViewport);

    if ((theDeltaZ != 0) && (theSine != 0) && (theAspectRatio != 0)) {
        float *writePtr(m_Projection.data());
        writePtr[10] = -(m_ClipFar + m_ClipNear) / theDeltaZ;
        writePtr[11] = -1;
        writePtr[14] = -2 * m_ClipNear * m_ClipFar / theDeltaZ;
        writePtr[15] = 0;

        if (IsCameraVerticalAdjust(m_ScaleMode, designAspect, theAspectRatio)) {
            float theCotangent = cosf(theAngleInRadians) / theSine;
            writePtr[0] = theCotangent / theAspectRatio;
            writePtr[5] = theCotangent;
        } else {
            float theCotangent = cosf(theAngleInRadians) / theSine;
            writePtr[0] = theCotangent / designAspect;
            writePtr[5] = theCotangent * (theAspectRatio / designAspect);
        }
        return true;
    } else {
        Q_ASSERT(false);
        return false;
    }
}

//==============================================================================
/**
 *	Compute the projection matrix for a orthographic camera
 *	@return true if the computed projection matrix is valid
 */
bool SCamera::ComputeFrustumOrtho(const QDemonRenderRectF &inViewport, const QVector2D &inDesignDimensions)
{
    m_Projection = QMatrix4x4();

    float theDeltaZ = m_ClipFar - m_ClipNear;
    float halfWidth = inDesignDimensions.x() / 2.0f;
    float halfHeight = inDesignDimensions.y() / 2.0f;
    float designAspect = GetAspectRatio(inDesignDimensions);
    float theAspectRatio = designAspect;
    if (IsFitTypeScaleMode(m_ScaleMode))
        theAspectRatio = GetAspectRatio(inViewport);
    if (theDeltaZ != 0) {
        float *writePtr(m_Projection.data());
        writePtr[10] = -2.0f / theDeltaZ;
        writePtr[11] = 0.0f;
        writePtr[14] = -(m_ClipNear + m_ClipFar) / theDeltaZ;
        writePtr[15] = 1.0f;
        if (IsCameraVerticalAdjust(m_ScaleMode, designAspect, theAspectRatio)) {
            writePtr[0] = 1.0f / (halfHeight * theAspectRatio);
            writePtr[5] = 1.0f / halfHeight;
        } else {
            writePtr[0] = 1.0f / halfWidth;
            writePtr[5] = 1.0f / (halfWidth / theAspectRatio);
        }
        return true;
    } else {
        Q_ASSERT(false);
        return false;
    }
}

float SCamera::GetOrthographicScaleFactor(const QDemonRenderRectF &inViewport,
                                          const QVector2D &inDesignDimensions) const
{
    if (m_ScaleMode == CameraScaleModes::SameSize)
        return 1.0f;
    QMatrix4x4 temp();
    float designAspect = GetAspectRatio(inDesignDimensions);
    float theAspectRatio = GetAspectRatio(inViewport);
    if (m_ScaleMode == CameraScaleModes::Fit) {
        if (theAspectRatio >= designAspect) {
            return inViewport.m_Width < inDesignDimensions.x() ? theAspectRatio / designAspect : 1.0f;

        } else {
            return inViewport.m_Height < inDesignDimensions.y() ? designAspect / theAspectRatio
                                                              : 1.0f;
        }
    } else if (m_ScaleMode == CameraScaleModes::FitVertical) {
        return (float)inDesignDimensions.y() / (float)inViewport.m_Height;
    } else {
        return (float)inDesignDimensions.x() / (float)inViewport.m_Width;
    }
}

float SCamera::GetTextScaleFactor(const QDemonRenderRectF &inViewport,
                                  const QVector2D &inDesignDimensions) const
{
    return qMax(1.0f, 1.0f / GetOrthographicScaleFactor(inViewport, inDesignDimensions));
}

QMatrix3x3 SCamera::GetLookAtMatrix(const QVector3D &inUpDir, const QVector3D &inDirection) const
{
    QVector3D theDirection(inDirection);

    theDirection.normalize();

    const QVector3D &theUpDir(inUpDir);

    // gram-shmidt orthogonalization
    QVector3D theCrossDir = QVector3D::crossProduct(theDirection, theUpDir);
    theCrossDir.normalize();
    QVector3D theFinalDir = QVector3D::crossProduct(theCrossDir, theDirection);
    theFinalDir.normalize();
    float multiplier = 1.0f;
    if (m_Flags.IsLeftHanded())
        multiplier = -1.0f;

    theDirection *= multiplier;
    float matrixData[9] = {theCrossDir.x(), theCrossDir.y(), theCrossDir.z(),
                           theFinalDir.x(), theFinalDir.y(), theFinalDir.z(),
                           theDirection.x(), theDirection.y(), theDirection.z()};

    QMatrix3x3 theResultMatrix(matrixData);
    return theResultMatrix;
}

void SCamera::LookAt(const QVector3D &inCameraPos, const QVector3D &inUpDir, const QVector3D &inTargetPos)
{
    QVector3D theDirection = inTargetPos - inCameraPos;
    if (m_Flags.IsLeftHanded())
        theDirection.setZ(theDirection.z() * -1.0f);
    m_Rotation = GetRotationVectorFromRotationMatrix(GetLookAtMatrix(inUpDir, theDirection));
    m_Position = inCameraPos;
    MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
}

void SCamera::CalculateViewProjectionMatrix(QMatrix4x4 &outMatrix) const
{
    QMatrix4x4 globalInverse = mat44::getInverse(m_GlobalTransform);
    outMatrix = m_Projection * globalInverse;
}

SCuboidRect SCamera::GetCameraBounds(const QDemonRenderRectF &inViewport,
                                     const QVector2D &inDesignDimensions) const
{
    QMatrix4x4 unused;
    SPinCameraResult theResult =
        PinCamera(inViewport, inDesignDimensions, unused, m_ScaleMode, m_ScaleAnchor);
    // find the normalized edges of the view frustum given the renormalization that happens when
    // pinning the camera.
    SCuboidRect normalizedCuboid(-1, 1, 1, -1);
    QVector2D translation(theResult.m_Viewport.m_X - theResult.m_VirtualViewport.m_X,
                       theResult.m_Viewport.m_Y - theResult.m_VirtualViewport.m_Y);
    if (m_ScaleMode == CameraScaleModes::SameSize) {
        // the cuboid ranges are the actual divided by the ideal in this case
        float xRange = 2.0f * (theResult.m_Viewport.m_Width / theResult.m_VirtualViewport.m_Width);
        float yRange =
            2.0f * (theResult.m_Viewport.m_Height / theResult.m_VirtualViewport.m_Height);
        normalizedCuboid = SCuboidRect(-1, -1 + yRange, -1 + xRange, -1);
        translation.setX(translation.x() / (theResult.m_VirtualViewport.m_Width / 2.0f));
        translation.setY(translation.y() / (theResult.m_VirtualViewport.m_Height / 2.0f));
        normalizedCuboid.Translate(translation);
    }
    // fit.  This means that two parameters of the normalized cuboid will be -1, 1.
    else {
        // In this case our perspective matrix will center the view and we need to decenter
        // it as necessary
        float actualAspect = GetAspectRatio(inViewport);
        float designAspect = GetAspectRatio(inDesignDimensions);
        // if we are wider than we are high
        float idealWidth = inViewport.m_Width;
        float idealHeight = inViewport.m_Height;

        if (IsCameraVerticalAdjust(m_ScaleMode, designAspect, actualAspect)) {
            // then we just need to setup the left, right parameters of the cuboid because we know
            // the top
            // bottom are -1,1 due to how fit works.
            idealWidth = (float)ITextRenderer::NextMultipleOf4(
                (quint32)(inViewport.m_Height * designAspect + .5f));
            // halfRange should always be greater than 1.0f.
            float halfRange = inViewport.m_Width / idealWidth;
            normalizedCuboid.m_Left = -halfRange;
            normalizedCuboid.m_Right = halfRange;
            translation.setX(translation.x() / (idealWidth / 2.0f));
        } else {
            idealHeight = (float)ITextRenderer::NextMultipleOf4(
                (quint32)(inViewport.m_Width / designAspect + .5f));
            float halfRange = inViewport.m_Height / idealHeight;
            normalizedCuboid.m_Bottom = -halfRange;
            normalizedCuboid.m_Top = halfRange;
            translation.setY(translation.y() / (idealHeight / 2.0f));
        }
        normalizedCuboid.Translate(translation);
    }
    // Given no adjustment in the virtual rect, then this is what we would have.

    return normalizedCuboid;
}

void SCamera::SetupOrthographicCameraForOffscreenRender(QDemonRenderTexture2D &inTexture,
                                                        QMatrix4x4 &outVP)
{
    STextureDetails theDetails(inTexture.GetTextureDetails());
    SCamera theTempCamera;
    SetupOrthographicCameraForOffscreenRender(inTexture, outVP, theTempCamera);
}

void SCamera::SetupOrthographicCameraForOffscreenRender(QDemonRenderTexture2D &inTexture,
                                                        QMatrix4x4 &outVP, SCamera &outCamera)
{
    STextureDetails theDetails(inTexture.GetTextureDetails());
    SCamera theTempCamera;
    theTempCamera.m_Flags.SetOrthographic(true);
    theTempCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
    QVector2D theDimensions((float)theDetails.m_Width, (float)theDetails.m_Height);
    theTempCamera.CalculateGlobalVariables(
        QDemonRenderRect(0, 0, theDetails.m_Width, theDetails.m_Height), theDimensions);
    theTempCamera.CalculateViewProjectionMatrix(outVP);
    outCamera = theTempCamera;
}

SRay SCamera::Unproject(const QVector2D &inViewportRelativeCoords, const QDemonRenderRectF &inViewport,
                        const QVector2D &inDesignDimensions) const
{
    SRay theRay;
    QMatrix4x4 tempVal;
    SPinCameraResult result =
        PinCamera(inViewport, inDesignDimensions, tempVal, m_ScaleMode, m_ScaleAnchor);
    QVector2D globalCoords = inViewport.ToAbsoluteCoords(inViewportRelativeCoords);
    QVector2D normalizedCoords =
        result.m_VirtualViewport.AbsoluteToNormalizedCoordinates(globalCoords);
    QVector3D &outOrigin(theRay.m_Origin);
    QVector3D &outDir(theRay.m_Direction);
    QVector2D inverseFrustumScale(1.0f / m_FrustumScale.x(), 1.0f / m_FrustumScale.y());
    QVector2D scaledCoords(inverseFrustumScale.x() * normalizedCoords.x(),
                        inverseFrustumScale.y() * normalizedCoords.y());

    if (m_Flags.IsOrthographic()) {
        outOrigin.setX(scaledCoords.x());
        outOrigin.setY(scaledCoords.y());
        outOrigin.setZ(0.0f);

        outDir.setX(0.0f);
        outDir.setY(0.0f);
        outDir.setZ(-1.0f);
    } else {
        outOrigin.setX(0.0f);
        outOrigin.setY(0.0f);
        outOrigin.setZ(0.0f);

        outDir.setX(scaledCoords.x());
        outDir.setY(scaledCoords.y());
        outDir.setZ(-1.0f);
    }

    outOrigin = m_GlobalTransform.transform(outOrigin);
    QMatrix3x3 theNormalMatrix;
    CalculateNormalMatrix(theNormalMatrix);

    outDir = theNormalMatrix.transform(outDir);
    outDir.normalize();
    /*
    char printBuf[2000];
    sprintf_s( printBuf, "normCoords %f %f outDir %f %f %f\n"
            , normalizedCoords.x, normalizedCoords.y, outDir.x, outDir.y, outDir.z );
    OutputDebugStringA( printBuf );
    */

    return theRay;
}

QVector3D SCamera::UnprojectToPosition(const QVector3D &inGlobalPos, const SRay &inRay) const
{
    QVector3D theCameraDir = GetDirection();
    QVector3D theObjGlobalPos = inGlobalPos;
    float theDistance = -1.0f * QVector3D::dotProduct(theObjGlobalPos, theCameraDir);
    QDemonPlane theCameraPlane(theCameraDir, theDistance);
    return inRay.Intersect(theCameraPlane);
}

float SCamera::verticalFov(float aspectRatio) const
{
    if (m_FOVHorizontal)
        return 2.0f * qAtan(qTan(qreal(m_FOV) / 2.0) / qreal(aspectRatio));
    else
        return m_FOV;
}

float SCamera::verticalFov(const QDemonRenderRectF &inViewport) const
{
    return verticalFov(GetAspectRatio(inViewport));
}

QT_END_NAMESPACE
