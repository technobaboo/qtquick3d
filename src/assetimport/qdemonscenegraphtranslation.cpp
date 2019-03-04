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

#include "qdemonscenegraphtranslation_p.h"
#include "qdemonmeshutilities_p.h"

#include <QtGui/QVector3D>

#include <QtCore/QFile>

QT_BEGIN_NAMESPACE

using namespace QDemonAssetImport;

namespace {

static inline QVector3D toColor(const float p[])
{
    return QVector3D(p[0], p[1], p[2]);
}
static inline QVector3D toColor(const float p[], float inMult)
{
    return QVector3D(p[0] * inMult, p[1] * inMult, p[2] * inMult);
}

static void rgbToHsv(float r, float g, float b, float *h, float *s, float *v)
{
    float min, max, delta;

    min = qMin(qMin(r, g), b);
    max = qMax(qMax(r, g), b);
    *v = max; // v
    delta = max - min;
    if (max != 0)
        *s = delta / max; // s
    else {
        // r = g = b = 0		// s = 0, v is undefined
        *s = 0;
        *h = -1;
        return;
    }
    if (r == max)
        *h = (g - b) / delta; // between yellow & magenta
    else if (g == max)
        *h = 2 + (b - r) / delta; // between cyan & yellow
    else
        *h = 4 + (r - g) / delta; // between magenta & cyan
    *h *= 60; // degrees
    if (*h < 0)
        *h += 360;
}

//==============================================================================
/**
 *	Swizzle the vertex value to match Studio format
 */
void swizzleVertex(QVector<float> &ioValues)
{
    int theSize = ioValues.size();
    for (int i = 0; i < theSize; i += 3) {
        // Negate z component
        ioValues[i + 2] = -ioValues[i + 2];
    }
}

//==============================================================================
/**
 *	Negate the vertex value
 */
void negateVertex(QVector<float> &ioValues)
{
    int theSize = ioValues.size();
    for (int i = 0; i < theSize; i += 3) {
        // Negate xyz component
        ioValues[i] = -ioValues[i];
        ioValues[i + 1] = -ioValues[i + 1];
        ioValues[i + 2] = -ioValues[i + 2];
    }
}

//==============================================================================
/**
 *	Swizzle the TexCoord value to match Studio format
 */
void swizzleTexCoord(QVector<float> &ioValues)
{
    int theSize = ioValues.size();
    for (int i = 0; i < theSize; i += 2) {
        // Flip y axis
        ioValues[i + 1] = 1 - ioValues[i + 1];
    }
}

//==============================================================================
/**
 *	Swizzle the Face Indices to match Studio format
 */
void swizzleFaceIndices(QVector<int> &ioValues)
{
    int theSize = ioValues.size();
    int theTemp;
    for (int i = 0; i < theSize; i += 3) {
        // Swap x & z value
        theTemp = ioValues[i];
        ioValues[i] = ioValues[i + 2];
        ioValues[i + 2] = theTemp;
    }
}

//==============================================================================
/**
 *	Convert an axis angle rotation vector into a matrix representation.
 *	This code is modified from FCollada's FMMatrix44::AxisRotationMatrix
 */
void matrixFromAxisAngle(float inDegreeAngle, float inXAxis, float inYAxis, float inZAxis,
                         float inMatrix[4][4])
{
    // If axis or angle is zero, we're going to get a bad matrix, so bail.
    if (fabsf(inDegreeAngle) < 0.001f
        || (fabsf(inXAxis) < 0.001f && fabsf(inYAxis) < 0.001f && fabsf(inZAxis) < 0.001f)) {
        return;
    }

    inDegreeAngle *= 0.0174532925199f;

    QVector3D theAxis(inXAxis, inYAxis, inZAxis);
    if (theAxis.lengthSquared() != 1.0f)
        theAxis.normalize();

    // Formulae inspired from
    // http://www.mines.edu/~gmurray/ArbitraryAxisRotation/ArbitraryAxisRotation.html
    float xSq = theAxis.x() * theAxis.x();
    float ySq = theAxis.y() * theAxis.y();
    float zSq = theAxis.z() * theAxis.z();
    float cT = cosf(inDegreeAngle);
    float sT = sinf(inDegreeAngle);

    inMatrix[0][0] = xSq + (ySq + zSq) * cT;
    inMatrix[0][1] = theAxis.x() * theAxis.y() * (1.0f - cT) + theAxis.z() * sT;
    inMatrix[0][2] = theAxis.x() * theAxis.z() * (1.0f - cT) - theAxis.y() * sT;
    inMatrix[0][3] = 0;
    inMatrix[1][0] = theAxis.x() * theAxis.y() * (1.0f - cT) - theAxis.z() * sT;
    inMatrix[1][1] = ySq + (xSq + zSq) * cT;
    inMatrix[1][2] = theAxis.y() * theAxis.z() * (1.0f - cT) + theAxis.x() * sT;
    inMatrix[1][3] = 0;
    inMatrix[2][0] = theAxis.x() * theAxis.z() * (1.0f - cT) + theAxis.y() * sT;
    inMatrix[2][1] = theAxis.y() * theAxis.z() * (1.0f - cT) - theAxis.x() * sT;
    inMatrix[2][2] = zSq + (xSq + ySq) * cT;
    inMatrix[2][3] = 0;
    inMatrix[3][2] = inMatrix[3][1] = inMatrix[3][0] = 0;
    inMatrix[3][3] = 1;
}

#define FLT_TOLERANCE 0.0001f

template <class T>
T sign(const T &val)
{
    return (val >= T(0)) ? T(1) : T(-1);
}

//==============================================================================
/**
 *	Get a 2x2 determinant
 */
static float det2x2(float a1, float a2, float b1, float b2)
{
    return a1 * b2 - b1 * a2;
}

//==============================================================================
/**
 *	Get the 3x3 determinant
 */
static float det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2,
                    float c3)
{
    return a1 * det2x2(b2, b3, c2, c3) - b1 * det2x2(a2, a3, c2, c3) + c1 * det2x2(a2, a3, b2, b3);
}

//==============================================================================
/**
 *	Decomposes the scale portion of a matrix.
 *	Modified from FCollada's FMMatrix44::Decompose
 */
void decomposeScale(QVector3D &outScale, float inMatrix[4][4])
{
    outScale.setX(sqrtf(inMatrix[0][0] * inMatrix[0][0] + inMatrix[0][1] * inMatrix[0][1]
                  + inMatrix[0][2] * inMatrix[0][2]));
    outScale.setY(sqrtf(inMatrix[1][0] * inMatrix[1][0] + inMatrix[1][1] * inMatrix[1][1]
                  + inMatrix[1][2] * inMatrix[1][2]));
    outScale.setZ(sqrtf(inMatrix[2][0] * inMatrix[2][0] + inMatrix[2][1] * inMatrix[2][1]
                  + inMatrix[2][2] * inMatrix[2][2]));

    float isInverted =
        sign(det3x3(inMatrix[0][0], inMatrix[0][1], inMatrix[0][2], inMatrix[1][0], inMatrix[1][1],
                    inMatrix[1][2], inMatrix[2][0], inMatrix[2][1], inMatrix[2][2]));

    if (isInverted < 0.0f) {
        outScale.setX(-outScale.x());
        outScale.setY(-outScale.y());
        outScale.setZ(-outScale.z());
    }
}

//==============================================================================
/**
*	Translate SubPropertyName to index.
*/
int getSubPropertyIndex(const char *inSubPropertyName)
{
    if (qstricmp(inSubPropertyName, "x") == 0 || qstricmp(inSubPropertyName, "r") == 0)
        return 0;
    else if (qstricmp(inSubPropertyName, "y") == 0 || qstricmp(inSubPropertyName, "g") == 0)
        return 1;
    else if (qstricmp(inSubPropertyName, "z") == 0 || qstricmp(inSubPropertyName, "b") == 0)
        return 2;
    else if (qstricmp(inSubPropertyName, "a") == 0)
        return 3;
    return 0;
}
}

QDemonSceneGraphTranslation::QDemonSceneGraphTranslation()
{
    m_meshBuilder = QDemonMeshUtilities::QDemonMeshBuilder::createMeshBuilder();
    m_documentBuffer = new QBuffer(&m_document);
    if (!m_documentBuffer->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug("failed to open buffer");
    }
    m_stream.setDevice(m_documentBuffer);

    reset();
}

QDemonSceneGraphTranslation::~QDemonSceneGraphTranslation()
{
    m_documentBuffer->close();
}

void QDemonSceneGraphTranslation::reset()
{
    m_tabDepth = 0;
    m_document.clear();
    m_documentBuffer->seek(0);

    // add header includes
    m_stream << QStringLiteral("import QtQuick 2.0\n");
    m_stream << QStringLiteral("import QtStudio3D 2.1\n\n\n");
}

void QDemonSceneGraphTranslation::release()
{

}

void QDemonSceneGraphTranslation::pushGroup(const QString &inName)
{
    insertTab();
    m_stream << QStringLiteral("Group3D {\n");
    m_tabDepth++;
    insertTab();
    m_stream << QStringLiteral("id: %1\n").arg(inName);
}

void QDemonSceneGraphTranslation::setGroupSkeletonId(int inId)
{
    Q_UNUSED(inId)
}

void QDemonSceneGraphTranslation::setIgnoresParentTransform(bool inValue)
{
    Q_UNUSED(inValue)
}

void QDemonSceneGraphTranslation::popGroup()
{
    m_tabDepth--;
    insertTab();
    m_stream << QStringLiteral("}\n");
}

void QDemonSceneGraphTranslation::pushModel(const QString &inName)
{
    insertTab();
    m_stream << QStringLiteral("Model3D {\n");
    m_tabDepth++;
    insertTab();
    m_stream << QStringLiteral("id: %1\n").arg(inName);
}

void QDemonSceneGraphTranslation::setModelSkeletonRoot(int inId)
{
    Q_UNUSED(inId)
}

void QDemonSceneGraphTranslation::popModel()
{
    m_tabDepth--;
    insertTab();
    m_stream << QStringLiteral("}\n");
}

void QDemonSceneGraphTranslation::pushMaterial(const QString &inName)
{
    insertTab();
    m_stream << QStringLiteral("Material3D {\n");
    m_tabDepth++;
    insertTab();
    m_stream << QStringLiteral("id: %1\n").arg(inName);
}

void QDemonSceneGraphTranslation::popMaterial(int inStartFaceIndex, int inNumberOfFaces)
{
    Q_UNUSED(inStartFaceIndex)
    Q_UNUSED(inNumberOfFaces)

    m_tabDepth--;
    insertTab();
    m_stream << QStringLiteral("}\n");
}

void QDemonSceneGraphTranslation::pushTexture(const QString &inName, const QString &inSourcePath, int inMapType)
{
    Q_UNUSED(inSourcePath)
    Q_UNUSED(inMapType)

    insertTab();
    m_stream << QStringLiteral("Q3DSImage {\n");
    m_tabDepth++;
    insertTab();
    m_stream << QStringLiteral("id: %1\n").arg(inName);
}

void QDemonSceneGraphTranslation::popTexture()
{
    m_tabDepth--;
    insertTab();
    m_stream << QStringLiteral("}\n");
}

void QDemonSceneGraphTranslation::markInvalid()
{

}

void QDemonSceneGraphTranslation::setTransforms(const QVector<AbstractNodeTransform *> &inTransforms)
{
    Q_UNUSED(inTransforms)
}

void QDemonSceneGraphTranslation::setGeometry(const QVector<float> &ioVertices,
                                            const QVector<float> &ioNormals,
                                            const QVector<float> &ioTexCoords,
                                            const QVector<float> &ioTexCoords2,
                                            const QVector<float> &ioTexTangents,
                                            const QVector<float> &ioTexBinormals,
                                            const QVector<float> &ioWeights,
                                            const QVector<float> &ioBoneIndex,
                                            const QVector<float> &ioColors,
                                            const QVector<quint32> &ioFaceIndicies)
{
    Q_UNUSED(ioVertices)
    Q_UNUSED(ioNormals)
    Q_UNUSED(ioTexCoords)
    Q_UNUSED(ioTexCoords2)
    Q_UNUSED(ioTexTangents)
    Q_UNUSED(ioTexBinormals)
    Q_UNUSED(ioWeights)
    Q_UNUSED(ioBoneIndex)
    Q_UNUSED(ioColors)
    Q_UNUSED(ioFaceIndicies)
}

void QDemonSceneGraphTranslation::setMaterial(const MaterialParameters &inMaterialParameters)
{
    Q_UNUSED(inMaterialParameters)
}

void QDemonSceneGraphTranslation::setTexture(int inMapType, const TextureParameters &inTextureParameters)
{
    Q_UNUSED(inMapType)
    Q_UNUSED(inTextureParameters)
}

void QDemonSceneGraphTranslation::setJointNode(JointInfo &inJointInfo)
{
    Q_UNUSED(inJointInfo)
}

int QDemonSceneGraphTranslation::cacheAnimationTrack()
{
    return 0;
}

void QDemonSceneGraphTranslation::applyAnimationTrack(int inAnimationTrackIndex)
{
    Q_UNUSED(inAnimationTrackIndex)
}

void QDemonSceneGraphTranslation::setAnimationTrack(const QString &inBasePropertyName, const QString &inSubPropertyName)
{
    Q_UNUSED(inBasePropertyName)
    Q_UNUSED(inSubPropertyName)
}

void QDemonSceneGraphTranslation::cacheAnimationKey(const QString &inBaseProperty, const QString &inSubPropertyName, const KeyframeParameters &inParameters)
{
    Q_UNUSED(inBaseProperty)
    Q_UNUSED(inSubPropertyName)
    Q_UNUSED(inParameters)
}

bool QDemonSceneGraphTranslation::save(const QString &filename)
{
    m_stream.flush();

    QFile outputFile(filename);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    outputFile.write(m_document);
    outputFile.close();
    return true;
}

void QDemonSceneGraphTranslation::insertTab()
{
    for (int i = 0; i < m_tabDepth; ++i)
        m_stream << "    ";
}

QT_END_NAMESPACE
