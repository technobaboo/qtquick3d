/****************************************************************************
**
** Copyright (C) 2015 NVIDIA Corporation.
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

#include <QtDemonRender/qdemonrenderpathfonttext.h>
#include <QtDemonRender/qdemonrenderpathfontspecification.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderpathrender.h>
#include <QtDemon/qdemonutils.h>

#include <QtGui/QMatrix3x3>

QT_BEGIN_NAMESPACE

// see NVprSDK for explanation
// Math from page 54-56 of "Digital Image Warping" by George Wolberg,
// though credited to Paul Heckert's "Fundamentals of Texture
// Mapping and Image Warping" 1989 Master's thesis.
static QMatrix3x3 mapSquareToQuad(QVector2D inSquare[4])
{
    QMatrix3x3 ret;

    QVector2D d1(inSquare[1] - inSquare[2]);
    QVector2D d2(inSquare[3] - inSquare[2]);
    QVector2D d3(inSquare[0] - inSquare[1] + inSquare[2] - inSquare[3]);

    float denom = d1.x() * d2.y() - d2.x() * d1.y();
    if (denom == 0.0) {
        return QMatrix3x3();
    }

    ret(2, 0) = (d3.x() * d2.y() - d2.x() * d3.y()) / denom;
    ret(2, 1) = (d1.x() * d3.y() - d3.x() * d1.y()) / denom;
    ret(2, 2) = 1.0;
    ret(0, 0) = inSquare[1].x() - inSquare[0].x() + ret(2, 0) * inSquare[1].x();
    ret(1, 0) = inSquare[1].y() - inSquare[0].y() + ret(2, 0) * inSquare[1].y();
    ret(0, 1) = inSquare[3].x() - inSquare[0].x() + ret(2, 1) * inSquare[3].x();
    ret(1, 1) = inSquare[3].y() - inSquare[0].y() + ret(2, 1) * inSquare[3].y();
    ret(0, 2) = inSquare[0].x();
    ret(1, 2) = inSquare[0].y();

    return ret;
}

static QMatrix3x3 mapQuadToSquare(QVector2D inSquare[4])
{
    return mat33::getInverse(mapSquareToQuad(inSquare));
}

static QMatrix3x3 mapQuadToQuad(QVector2D fromSquare[4], QVector2D toSquare[4])
{
    return (mapSquareToQuad(toSquare) * mapQuadToSquare(fromSquare));
}

static QMatrix4x4 mapBoxToQuad(QVector4D inBox, QVector2D inSquare[4])
{
    QVector2D fromSquare[4] = { QVector2D(inBox.x(), inBox.y()), QVector2D(inBox.z(), inBox.y()),
                                QVector2D(inBox.z(), inBox.w()), QVector2D(inBox.x(), inBox.w()) };

    QMatrix3x3 ret = mapQuadToQuad(fromSquare, inSquare);

    return QMatrix4x4(ret);
}

QDemonRenderPathFontItem::QDemonRenderPathFontItem()
    : m_NumGlyphs(0)
    , m_GlyphIDs(nullptr)
    , m_TranslateXY(nullptr)
{
}

QDemonRenderPathFontItem::~QDemonRenderPathFontItem()
{
    if (m_TranslateXY)
        ::free(m_TranslateXY);
    if (m_GlyphIDs)
        ::free(m_GlyphIDs);
}

void QDemonRenderPathFontItem::InitTextItem(size_t glyphCount, const quint32 *glyphIDs,
                                            QDemonRenderPathFormatType::Enum type, float *posArray,
                                            QVector2D pixelBound, QVector2D logicalBound, float emScale)
{
    m_NumGlyphs = glyphCount;

    // allocate glyphs array
    if (m_GlyphIDs)
        ::free(m_GlyphIDs);

    // allocate position array
    if (m_TranslateXY)
        ::free(m_TranslateXY);

    m_GlyphIDs = static_cast<quint32 *>(::malloc(glyphCount * getSizeofType(type)));
    m_TranslateXY = static_cast<float *>(::malloc(2 * (glyphCount + 1) * sizeof(float)));

    if (!m_GlyphIDs || !m_TranslateXY)
        return;

    quint32 *pTheGlyphIDs = (quint32 *)m_GlyphIDs;
    quint32 *pInGlyphs = (quint32 *)glyphIDs;

    /// copy glyphs array
    for (size_t i = 0; i < glyphCount; i++) {
        pTheGlyphIDs[i] = pInGlyphs[i];
    }

    // copy position array
    // we copy what we got from our layout system
    if (posArray != nullptr) {
        for (size_t i = 0, k = 0; i < glyphCount * 2; i += 2, k++) {
            m_TranslateXY[i] = posArray[i] * emScale;
            m_TranslateXY[i + 1] = posArray[i + 1] * emScale;
        }
    }

    // setup transform
    QVector2D square[4] = { QVector2D(0.0, 0.0), QVector2D(pixelBound.x(), 0.0),
                            QVector2D(pixelBound.x(), pixelBound.y()), QVector2D(0.0, pixelBound.y()) };
    QVector4D box(0.0, 0.0, logicalBound.x() * emScale, logicalBound.y() * emScale);

    m_ModelMatrix = mapBoxToQuad(box, square);
}

const QMatrix4x4 QDemonRenderPathFontItem::GetTransform()
{
    return QMatrix4x4(m_ModelMatrix(0, 0), m_ModelMatrix(1, 0), 0.0, m_ModelMatrix(2, 0),
                      m_ModelMatrix(0, 1), m_ModelMatrix(1, 1), 0.0, m_ModelMatrix(2, 1),
                      0.0, 0.0, 1.0, 0.0,
                      m_ModelMatrix(0, 2), m_ModelMatrix(1, 2), 0.0, m_ModelMatrix(2, 2));
}

quint32
QDemonRenderPathFontItem::getSizeofType(QDemonRenderPathFormatType::Enum type)
{
    switch (type) {
    case QDemonRenderPathFormatType::Byte:
        return sizeof(qint8);
    case QDemonRenderPathFormatType::UByte:
        return sizeof(quint8);
    case QDemonRenderPathFormatType::Bytes2:
        return sizeof(quint16);
    case QDemonRenderPathFormatType::Uint:
        return sizeof(quint32);
    case QDemonRenderPathFormatType::Utf8:
        return sizeof(quint32);
    default:
        Q_ASSERT(false);
        return 1;
    }
}

QDemonRenderPathFontItem *QDemonRenderPathFontItem::CreatePathFontItem(QDemonRenderContextImpl &context)
{
    Q_ASSERT(context.IsPathRenderingSupported());

    return new QDemonRenderPathFontItem();
}
QT_END_NAMESPACE
