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
#include <qdemontextrenderer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>

QT_BEGIN_NAMESPACE

// http://acius2.blogspot.com/2007/11/calculating-next-power-of-2.html
quint32 QDemonTextRendererInterface::nextPowerOf2(quint32 input)
{
    // Algorithm doesn't work for 0 or std::numeric_limits<quint32>::max()
    Q_ASSERT(input > 0 && input < std::numeric_limits<quint32>::max());
    input--;
    input = (input >> 1) | input;
    input = (input >> 2) | input;
    input = (input >> 4) | input;
    input = (input >> 8) | input;
    input = (input >> 16) | input;
    input++; // input is now the next highest power of 2.
    return input;
}

quint32 QDemonTextRendererInterface::nextMultipleOf4(quint32 inValue)
{
    quint32 remainder(inValue % 4);
    if (remainder != 0)
        inValue = inValue + (4 - remainder);

    return inValue;
}

QDemonTextTextureDetails QDemonTextRendererInterface::uploadData(QDemonDataRef<quint8> inTextureData,
                                                                 QDemonRenderTexture2D &inTexture,
                                                                 quint32 inDataWidth,
                                                                 quint32 inDataHeight,
                                                                 quint32 inTextWidth,
                                                                 quint32 inTextHeight,
                                                                 QDemonRenderTextureFormat inFormat,
                                                                 bool inFlipYAxis)
{
    if (inTextWidth == 0 || inTextHeight == 0) {
        quint32 black[] = { 0, 0, 0, 0 };
        inTexture.setTextureData(toU8DataRef(black, 4), 0, 2, 2, QDemonRenderTextureFormat::RGBA8);
        return QDemonTextTextureDetails(2, 2, false, QVector2D(1.0f, 1.0f));
    }
    Q_ASSERT(nextMultipleOf4(inDataWidth) == inDataWidth);
    quint32 theNecessaryHeight = nextMultipleOf4(inTextHeight);
    quint32 dataStride = inDataWidth * inFormat.getSizeofFormat();
    if (inTextureData.size() < dataStride * inDataHeight) {
        Q_ASSERT(false);
        return QDemonTextTextureDetails();
    }

    QDemonTextureDetails theTextureDetails = inTexture.textureDetails();
    quint32 theUploadSize = theNecessaryHeight * dataStride;

    QDemonDataRef<quint8> theUploadData = QDemonDataRef<quint8>(inTextureData.begin(), theUploadSize);
    inTexture.setTextureData(theUploadData, 0, inDataWidth, theNecessaryHeight, inFormat);
    inTexture.setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
    inTexture.setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
    return QDemonTextTextureDetails(inTextWidth, inTextHeight, inFlipYAxis, QVector2D(1.0f, 1.0f));
}

QT_END_NAMESPACE
