/****************************************************************************
**
** Copyright (C) 1999-2001 NVIDIA Corporation.
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

//==============================================================================
//	Includes
//==============================================================================

#include "qdemonrenderimagescaler.h"

#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE
//==============================================================================
//	Namespace
//==============================================================================
QDemonImageScaler::QDemonImageScaler()
{
}
//==============================================================================
/**
 *	Scales the given image by the given scale factor.
 *
 *	This method creates a new image based on the parameters given.
 *
 *	@param	inScaleMethod	type of scaling operation
 *	@param	inOldBuffer		points to the old picture
 *  @param  inOldWidth		width of the old picture
 *	@param  inOldHeight		height of the old picture
 *	@param	inNewBuffer		will point to the scaled picture
 *	@param	inNewWidth		width of the new picture
 *	@param	inNewHeight		height of the new picture
 *	@param  inPlanes		number of planes (1 for greyscale, 3 for rgb, etc)
 *  also equivalent to the return value of the CTextureType::PixelSize method.
 */
void QDemonImageScaler::scale(EScaleMethod inScaleMethod, unsigned char *inOldBuffer,
                         unsigned long inOldWidth, unsigned long inOldHeight,
                         unsigned char *&outNewBuffer, unsigned long inNewWidth,
                         unsigned long inNewHeight, unsigned long inPlanes)
{
    switch (inScaleMethod) {
    case SCALEMETHOD_CROP:
        QDemonImageScaler::crop(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer, inNewWidth,
                           inNewHeight, inPlanes);
        break;

    case SCALEMETHOD_BILINEAR:
        QDemonImageScaler::bilinear(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer, inNewWidth,
                               inNewHeight, inPlanes);
        break;

    default:
        Q_ASSERT(false);
        break;
    }
}

//==============================================================================
/**
 *	Scales the given image by the given scale factor.
 *
 *	This method creates a new image based on the parameters given.
 *
 *	@param	inScaleMethod	type of scaling operation
 *	@param	inOldBuffer		points to the old picture
 *  @param  inOldWidth		width of the old picture
 *	@param  inOldHeight		height of the old picture
 *	@param	inNewBuffer		will point to the scaled picture
 *	@param	inNewWidth		width of the new picture
 *	@param	inNewHeight		height of the new picture
 *	@param  inPlanes		number of planes (1 for greyscale, 3 for rgb, etc)
 *  also equivalent to the return value of the CTextureType::PixelSize method.
 */
void QDemonImageScaler::fastScale(EScaleMethod inScaleMethod, unsigned char *inOldBuffer,
                             unsigned long inOldWidth, unsigned long inOldHeight,
                             unsigned char *&outNewBuffer, unsigned long inNewWidth,
                             unsigned long inNewHeight, unsigned long inPlanes)
{
    switch (inScaleMethod) {
    case SCALEMETHOD_CROP:
        QDemonImageScaler::crop(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer, inNewWidth,
                           inNewHeight, inPlanes);
        break;

    case SCALEMETHOD_POINTSAMPLE:
        QDemonImageScaler::fastPointSample(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer,
                                      inNewWidth, inNewHeight, inPlanes);
        break;

    default:
        Q_ASSERT(false);
        break;
    }
}

//==============================================================================
/**
 *	Debug method that simply crops the picture instead of scaling.
 *
 *	Not for use in production.  This is a test method to exercise the framework.
 *
 *	@param	inOldBuffer		points to the old picture
 *  @param  inOldWidth		width of the old picture
 *	@param  inOldHeight		height of the old picture
 *	@param	inNewBuffer		will point to the scaled picture
 *	@param	inNewWidth		width of the new picture
 *	@param	inNewHeight		height of the new picture
 *	@param  inPlanes		number of planes (1 for greyscale, 3 for rgb, etc)
 *  also equivalent to the return value of the CTextureType::PixelSize method.
*/
void QDemonImageScaler::crop(unsigned char *inOldBuffer, unsigned long inOldWidth,
                        unsigned long inOldHeight, unsigned char *&outNewBuffer,
                        unsigned long inNewWidth, unsigned long inNewHeight, unsigned long inPlanes)
{
    Q_UNUSED(inOldHeight);

    Q_ASSERT(inNewWidth <= inOldWidth);
    Q_ASSERT(inNewHeight <= inOldHeight);

    long theMinWidth = qMin(inOldWidth, inNewWidth);

    outNewBuffer = new unsigned char[inNewWidth * inNewHeight * inPlanes];
    ::memset(outNewBuffer, 0, inNewWidth * inNewHeight * inPlanes);

    for (unsigned long theRow = 0; theRow < inNewHeight; ++theRow) {
        ::memcpy(outNewBuffer + theRow * inNewWidth * inPlanes,
                 inOldBuffer + theRow * inOldWidth * inPlanes, theMinWidth * inPlanes);
    }
}

//==============================================================================
/**
 *	Plain scaling.
 *
 *	Code adopted from http://www.codeguru.com/bitmap/SmoothBitmapResizing.html
 *	Preliminary formatting completed but still needs work.
 *
 *	@param	inOldBuffer		points to the old picture
 *  @param  inOldWidth		width of the old picture
 *	@param  inOldHeight		height of the old picture
 *	@param	inNewBuffer		will point to the scaled picture
 *	@param	inNewWidth		width of the new picture
 *	@param	inNewHeight		height of the new picture
 *	@param  inPlanes		number of planes (1 for greyscale, 3 for rgb, etc)
 *  also equivalent to the return value of the CTextureType::PixelSize method.
*/
void QDemonImageScaler::bilinear(unsigned char *inOldBuffer, unsigned long inOldWidth,
                            unsigned long inOldHeight, unsigned char *&outNewBuffer,
                            unsigned long inNewWidth, unsigned long inNewHeight,
                            unsigned long inPlanes)
{
    Q_ASSERT(inPlanes > 0);

    outNewBuffer = new unsigned char[inNewWidth * inNewHeight * inPlanes];
    QDemonImageScaler::resize(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer, inNewWidth,
                         inNewHeight, inPlanes);
}

//==============================================================================
/**
 *	Fast removal of selected pixels.
 *
 *	Really fast scanning of every n-th pixel.  This algorithm works basically by
 *	adding a fraction to the source pointer for each pixel destination, using
 *	fixed point arithmetic.
 *
 *	@param	inOldBuffer		points to the old picture
 *  @param  inOldWidth		width of the old picture
 *	@param  inOldHeight		height of the old picture
 *	@param	inNewBuffer		will point to the scaled picture
 *	@param	inNewWidth		width of the new picture
 *	@param	inNewHeight		height of the new picture
 *	@param  inPlanes		number of planes (1 for greyscale, 3 for rgb, etc)
 *  also equivalent to the return value of the CTextureType::PixelSize method.
*/
void QDemonImageScaler::fastPointSample(unsigned char *inOldBuffer, unsigned long inOldWidth,
                                   unsigned long inOldHeight, unsigned char *&outNewBuffer,
                                   unsigned long inNewWidth, unsigned long inNewHeight,
                                   unsigned long inPlanes)
{
    unsigned long theXAccum;
    unsigned long theYAccum;
    unsigned long theXConst;
    unsigned long theYConst;
    unsigned long theRow;
    unsigned long theColumn;
    unsigned long theAdd;
    unsigned long theSrcIndex;
    outNewBuffer = new unsigned char[inNewWidth * inNewHeight * inPlanes];

    // *** Debug ***
    // char dMessage[100];
    //::sprintf( dMessage, "PointSample: %ldx%ld to %ldx%ld\n",inOldInfo.m_Width, inOldInfo.m_Height, outNewInfo.m_Width, outNewInfo.m_Height  );
    //::OutputDebugString( dMessage );

    switch (inPlanes) {
    case 4: {
        long *theSrc;
        long *theDst;
        theSrc = reinterpret_cast<long *>(inOldBuffer);
        theDst = reinterpret_cast<long *>(outNewBuffer);
        theYAccum = 0;
        theXConst = (inOldWidth << 16) / inNewWidth;
        theYConst = (inOldHeight << 16) / inNewHeight;
        unsigned long theAdd;
        for (theRow = 0; theRow < inNewHeight; theRow++) {
            theXAccum = 0;
            theSrcIndex = 0;
            for (theColumn = 0; theColumn < inNewWidth; theColumn++) {

                theXAccum += theXConst;
                theAdd = theXAccum >> 16;
                *theDst = theSrc[theSrcIndex];
                theDst++;
                theSrcIndex += theAdd;
                // Clear out the integer portion of the accumulator.
                theXAccum = theXAccum & 0xFFFF;
            }

            theYAccum += theYConst;
            theAdd = (theYAccum) >> 16;
            theSrc += theAdd * inOldWidth;
            // Clear out the integer portion of the accumulator.
            theYAccum = theYAccum & 0xFFFF;
        }
    } break;

    case 3: {
        unsigned char *theDest;
        unsigned char *theSource;
        theDest = reinterpret_cast<unsigned char *>(outNewBuffer);
        theSource = reinterpret_cast<unsigned char *>(inOldBuffer);
        theYAccum = 0;
        theXConst = (inOldWidth << 16) / inNewWidth;
        theYConst = (inOldHeight << 16) / inNewHeight;
        for (theRow = 0; theRow < inNewHeight; ++theRow) {
            theXAccum = 0;
            theSrcIndex = 0;
            for (theColumn = 0; theColumn < inNewWidth; ++theColumn) {
                theDest[0] = theSource[0];
                theDest[1] = theSource[1];
                theDest[2] = theSource[2];
                theDest += 3;
                theSrcIndex += 3 * (theXAccum) >> 16;
                theXAccum = theXAccum & 0xFFFF;
            }
            theYAccum += theYConst;
            theAdd = (theYAccum) >> 16;
            theSource += theAdd * inOldWidth * 3;
            theYAccum = theYAccum & 0xFFFF;
        }
    } break;

    case 2: {
        short *theDest;
        short *theSource;
        theDest = reinterpret_cast<short *>(outNewBuffer);
        theSource = reinterpret_cast<short *>(inOldBuffer);
        theYAccum = 0;
        theXConst = (inOldWidth << 16) / inNewWidth;
        theYConst = (inOldHeight << 16) / inNewHeight;
        for (unsigned long theY = 0; theY < inNewHeight; ++theY) {
            theXAccum = 0;
            theSrcIndex = 0;
            for (unsigned long theX = 0; theX < inNewWidth; ++theX) {
                *theDest = *theSource;
                ++theDest;
                theXAccum += theXConst;
                theSrcIndex += (theXAccum) >> 16;
                theXAccum = theXAccum & 0xFFFF;
            }
            theYAccum += theYConst;
            theAdd = (theYAccum) >> 16;
            theSource += theAdd * inOldWidth;
            theYAccum = theYAccum & 0xFFFF;
        }
    } break;

    case 1: {
        unsigned char *theDest;
        unsigned char *theSource;
        theDest = reinterpret_cast<unsigned char *>(outNewBuffer);
        theSource = reinterpret_cast<unsigned char *>(inOldBuffer);
        theYAccum = 0;
        theXConst = (inOldWidth << 16) / inNewWidth;
        theYConst = (inOldHeight << 16) / inNewHeight;
        for (unsigned long theY = 0; theY < inNewHeight; ++theY) {
            theXAccum = 0;
            theSrcIndex = 0;
            for (unsigned long theX = 0; theX < inNewWidth; ++theX) {
                *theDest = *theSource;
                ++theDest;
                theXAccum += theXConst;
                theSrcIndex += (theXAccum) >> 16;
                theXAccum = theXAccum & 0xFFFF;
            }
            theYAccum += theYConst;
            theAdd = (theYAccum) >> 16;
            theSource += theAdd * inOldWidth;
            theYAccum = theYAccum & 0xFFFF;
        }
    } break;

    default:
        Q_ASSERT(false);
        break;
    }
}

//==============================================================================
/**
 *	@param	inWidth
 *	@param	inHeight
 *	@return	unsigned char*
 */
unsigned char *QDemonImageScaler::allocateBuffer(long inWidth, long inHeight)
{
    unsigned char *theBuffer = new unsigned char[inHeight * inWidth * 4];
    return theBuffer;
}

//==============================================================================
/**
 *	@param	ioBuffer the buffer to release
 */
void QDemonImageScaler::releaseBuffer(unsigned char *&ioBuffer)
{
    delete[] ioBuffer;
    ioBuffer = nullptr;
}

//==============================================================================
/**
 *	@param	inOldBuffer		points to the old picture
 *  @param  inOldWidth		width of the old picture
 *	@param  inOldHeight		height of the old picture
 *	@param	inNewBuffer		will point to the scaled picture
 *	@param	inNewWidth		width of the new picture
 *	@param	inNewHeight		height of the new picture
 *	@param  inPlanes		number of planes (1 for greyscale, 3 for rgb, etc)
 *  also equivalent to the return value of the CTextureType::PixelSize method.
 */
void QDemonImageScaler::resize(unsigned char *inOldBuffer, unsigned long inOldWidth,
                          unsigned long inOldHeight, unsigned char *&outNewBuffer,
                          unsigned long inNewWidth, unsigned long inNewHeight,
                          unsigned long inPlanes)
{
    Q_ASSERT(inPlanes == 4);

    // only do the temporary allocation if necessary
    if (inOldWidth < inNewWidth || inOldHeight < inNewHeight) {
        QDemonImageScaler::expandRowsAndColumns(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer,
                                           inNewWidth, inNewHeight, inPlanes);
        return;
    } else {
        // The downsampling algorithms *do* assume four planes.
        if (inOldWidth > inNewWidth && inOldHeight > inNewHeight) {
            QByteArray theBuffer;
            theBuffer.resize(inNewWidth * inOldHeight * 4);
            unsigned char *theTempBuffer = reinterpret_cast<unsigned char *>(theBuffer.data());
            QDemonImageScaler::reduceCols(inOldBuffer, inOldWidth, inOldHeight, theTempBuffer,
                                     inNewWidth);
            QDemonImageScaler::reduceRows(theTempBuffer, inNewWidth, inOldHeight, outNewBuffer,
                                     inNewHeight);
        } else if (inOldWidth > inNewWidth) {
            QDemonImageScaler::reduceCols(inOldBuffer, inOldWidth, inOldHeight, outNewBuffer,
                                     inNewWidth);
        } else if (inOldHeight > inNewHeight) {
            QDemonImageScaler::reduceRows(inOldBuffer, inNewWidth, inOldHeight, outNewBuffer,
                                     inNewHeight);
        }
    }
}

void QDemonImageScaler::expandRowsAndColumns(unsigned char *inBuffer, unsigned long inWidth,
                                        unsigned long inHeight, unsigned char *outBuffer,
                                        unsigned long inDstWidth, unsigned long inDstHeight,
                                        unsigned long inPlanes)
{
    if (inDstWidth < inWidth || inDstHeight < inHeight) {
        return;
    }
    /*if( inPlanes == 4 )
    {
            FastExpandRowsAndColumns( inBuffer, inWidth, inHeight,
                                                              outBuffer, inDstWidth, inDstHeight );
            return;
    }*/
    unsigned long theYPosition;
    unsigned short theYRatio;
    unsigned short theYInvRatio;
    unsigned long theXPosition;
    unsigned short theXRatio;
    unsigned short theXInvRatio;

    unsigned long theRow;
    unsigned long theColumn;
    unsigned long theSrcIndex;
    unsigned long theDstIndex;
    unsigned long theSrcLineLength;
    unsigned long theTemp;
    unsigned long thePixel;

    theDstIndex = 0;
    theSrcIndex = 0;
    theSrcLineLength = inWidth * inPlanes;
    theYPosition = inDstHeight;
    theYRatio = 1 << 8;
    theYInvRatio = 0;
    theXInvRatio = 0;
    // Here we go....
    // This algorithm will be quite a bit hairy, if you want
    // to understand it, then look at the two expand alogorithms above
    // and realize the this is just the logical combination of the two
    for (theRow = 0; theRow < inDstHeight; theRow++) {
        // Run through all the rows, multiplying if necessary the two ratio's together
        theXPosition = inDstWidth;
        if (theYPosition < inHeight) {
            // We have crossed a row boundary
            theYRatio = (unsigned short)((theYPosition << 8) / inHeight);
            theYInvRatio = (unsigned short)((1 << 8) - theYRatio);

            for (theColumn = 0; theColumn < inDstWidth; theColumn++) {
                if (theXPosition < inWidth) {
                    theXRatio = (unsigned short)((theXPosition << 8) / inWidth);
                    theXInvRatio = (unsigned short)((1 << 8) - theXRatio);

                    // The combination of both the x and y ratio's
                    unsigned long theLeftRatio = (theXRatio * theYRatio) >> 8;
                    unsigned long theRightRatio = (theXInvRatio * theYRatio) >> 8;
                    unsigned long theLowLeftRatio = (theXRatio * theYInvRatio) >> 8;
                    unsigned long theLowRightRatio = (theXInvRatio * theYInvRatio) >> 8;
                    // We are on a row and column boundary, thus each pixel here is the
                    // combination of four pixels (left right, low left, low right)
                    for (thePixel = 0; thePixel < inPlanes; thePixel++) {
                        // Left side first
                        theTemp = (theLeftRatio * inBuffer[theSrcIndex]);
                        theTemp += (theRightRatio * inBuffer[theSrcIndex + inPlanes]);
                        theTemp += (theLowLeftRatio * inBuffer[theSrcIndex + theSrcLineLength]);
                        theTemp += (theLowRightRatio
                                    * inBuffer[theSrcIndex + theSrcLineLength + inPlanes]);
                        outBuffer[theDstIndex] = (unsigned char)(theTemp >> 8);
                        theDstIndex++;
                        theSrcIndex++;
                    }
                    // Reset our position calculation
                    theXPosition = inDstWidth - inWidth + theXPosition;
                } else {
                    for (thePixel = 0; thePixel < inPlanes; thePixel++) {
                        theTemp = theYRatio * inBuffer[theSrcIndex + thePixel];
                        theTemp +=
                                theYInvRatio * inBuffer[theSrcIndex + theSrcLineLength + thePixel];
                        outBuffer[theDstIndex] = (unsigned char)(theTemp >> 8);
                        theDstIndex++;
                    }
                    // Reset our position calculation
                    theXPosition -= inWidth;
                }
            }
            // Reset our position calculation
            theYPosition = inDstHeight - inHeight + theYPosition;
            // Make the src index point to the next line
            theSrcIndex += inPlanes;
        } // Ends the if to check if we are crossing a row boundary
        // Else we are not crossing a row boundary
        else {
            for (theColumn = 0; theColumn < inDstWidth; theColumn++) {
                // If we are crossing a column boundary
                if (theXPosition < inWidth) {
                    theXRatio = (unsigned short)((theXPosition << 8) / inWidth);
                    theXInvRatio = (unsigned short)((1 << 8) - theXRatio);
                    for (thePixel = 0; thePixel < inPlanes; thePixel++) {
                        theTemp = theXRatio * inBuffer[theSrcIndex];
                        theTemp += theXInvRatio * inBuffer[theSrcIndex + inPlanes];
                        outBuffer[theDstIndex] = (unsigned char)(theTemp >> 8);
                        theSrcIndex++;
                        theDstIndex++;
                    }

                    theXPosition = inDstWidth - inWidth + theXPosition;
                }
                // Else we are not crossing a column boundary
                else {
                    for (thePixel = 0; thePixel < inPlanes; thePixel++) {
                        outBuffer[theDstIndex] = inBuffer[theSrcIndex + thePixel];
                        theDstIndex++;
                    }
                    theXPosition -= inWidth;
                }
            }
            // reset our y position indicator
            theYPosition -= inHeight;
            // reset the src index to the beginning the next line
            theSrcIndex += inPlanes;
            // reset src index to the beginning of this line
            theSrcIndex -= theSrcLineLength;
        } // End of else for row boundary
    } // End of for loop for iterating through all rows
}

// Assuming the number of planes is four

void QDemonImageScaler::fastExpandRowsAndColumns(unsigned char *inBuffer, unsigned long inWidth,
                                            unsigned long inHeight, unsigned char *outBuffer,
                                            unsigned long inDstWidth, unsigned long inDstHeight)
{

    if (inDstWidth < inWidth || inDstHeight < inHeight) {
        return;
    }
    unsigned long theYPosition;
    unsigned short theYRatio;
    unsigned short theYInvRatio;
    unsigned long theXPosition;
    unsigned short theXRatio;
    unsigned short theXInvRatio;
    // The combination of both the x and y ratio's
    unsigned long theLeftRatio;
    unsigned long theRightRatio;
    unsigned long theLowLeftRatio;
    unsigned long theLowRightRatio;

    unsigned long theRow;
    unsigned long theColumn;
    unsigned long theSrcIndex;
    unsigned long theDstIndex;
    unsigned long theSrcLineLength;
    unsigned long theTemp;

    theDstIndex = 0;
    theSrcIndex = 0;
    theSrcLineLength = inWidth * 4;
    theYPosition = inDstHeight;
    theYInvRatio = 0;
    theXInvRatio = 0;
    // Here we go....
    // This algorithm will be quite a bit hairy, if you want
    // to understand it, then look at the two expand alogorithms above
    // and realize the this is just the logical combination of the two
    for (theRow = 0; theRow < inDstHeight; theRow++) {
        // Run through all the rows, multiplying if necessary the two ratio's together
        theXPosition = inDstWidth;
        if (theYPosition < inHeight) {
            // We have crossed a row boundary
            theYRatio = (unsigned short)((theYPosition << 8) / inHeight);
            theYInvRatio = (unsigned short)((1 << 8) - theYRatio);

            for (theColumn = 0; theColumn < inDstWidth; theColumn++) {
                if (theXPosition < inWidth) {
                    theXRatio = (unsigned short)((theXPosition << 8) / inWidth);
                    theXInvRatio = (unsigned short)((1 << 8) - theXRatio);
                    theLeftRatio = (theXRatio * theYRatio) >> 8;
                    theRightRatio = (theXInvRatio * theYRatio) >> 8;
                    theLowLeftRatio = (theXRatio * theYInvRatio) >> 8;
                    theLowRightRatio = (theXInvRatio * theYInvRatio) >> 8;
                    // We are on a row and column boundary, thus each pixel here is the
                    // combination of four pixels (left right, low left, low right)

                    // Left side first
                    theTemp = (inBuffer[theSrcIndex]);
                    theTemp += (inBuffer[theSrcIndex + 4]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength + 4]);
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 2);
                    theDstIndex++;
                    theSrcIndex++;
                    // Left side first
                    theTemp = (inBuffer[theSrcIndex]);
                    theTemp += (inBuffer[theSrcIndex + 4]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength + 4]);
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 2);
                    theDstIndex++;
                    theSrcIndex++;
                    // Left side first
                    theTemp = (inBuffer[theSrcIndex]);
                    theTemp += (inBuffer[theSrcIndex + 4]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength + 4]);
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 2);
                    theDstIndex++;
                    theSrcIndex++;
                    // Left side first
                    theTemp = (inBuffer[theSrcIndex]);
                    theTemp += (inBuffer[theSrcIndex + 4]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength]);
                    theTemp += (inBuffer[theSrcIndex + theSrcLineLength + 4]);
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 2);
                    theDstIndex++;
                    theSrcIndex++;
                    // Reset our position calculation
                    theXPosition = inDstWidth - inWidth + theXPosition;
                } else {

                    theTemp = inBuffer[theSrcIndex];
                    theTemp += inBuffer[theSrcIndex + theSrcLineLength];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theDstIndex++;
                    theTemp = inBuffer[theSrcIndex + 1];
                    theTemp += inBuffer[theSrcIndex + theSrcLineLength + 1];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theDstIndex++;
                    theTemp = inBuffer[theSrcIndex + 2];
                    theTemp += inBuffer[theSrcIndex + theSrcLineLength + 2];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theDstIndex++;
                    theTemp = inBuffer[theSrcIndex + 3];
                    theTemp += inBuffer[theSrcIndex + theSrcLineLength + 3];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theDstIndex++;
                    // Reset our position calculation
                    theXPosition -= inWidth;
                }
            }
            // Reset our position calculation
            theYPosition = inDstHeight - inHeight + theYPosition;
            // Make the src index point to the next line
            theSrcIndex += 4;
        } // Ends the if to check if we are crossing a row boundary
        // Else we are not crossing a row boundary
        else {
            for (theColumn = 0; theColumn < inDstWidth; theColumn++) {
                // If we are crossing a column boundary
                if (theXPosition < inWidth) {
                    theXRatio = (unsigned short)((theXPosition << 8) / inWidth);
                    theXInvRatio = (unsigned short)((1 << 8) - theXRatio);

                    theTemp = inBuffer[theSrcIndex];
                    theTemp += inBuffer[theSrcIndex + 4];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theSrcIndex++;
                    theDstIndex++;
                    theTemp = inBuffer[theSrcIndex];
                    theTemp += inBuffer[theSrcIndex + 4];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theSrcIndex++;
                    theDstIndex++;
                    theTemp = inBuffer[theSrcIndex];
                    theTemp += inBuffer[theSrcIndex + 4];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theSrcIndex++;
                    theDstIndex++;
                    theTemp = inBuffer[theSrcIndex];
                    theTemp += inBuffer[theSrcIndex + 4];
                    outBuffer[theDstIndex] = (unsigned char)(theTemp >> 1);
                    theSrcIndex++;
                    theDstIndex++;

                    theXPosition = inDstWidth - inWidth + theXPosition;
                }
                // Else we are not crossing a column boundary
                else {
                    *((long *)(outBuffer + theDstIndex)) = *((long *)(inBuffer + theSrcIndex));
                    theDstIndex += 4;
                    theXPosition -= inWidth;
                }
            }
            // reset our y position indicator
            theYPosition -= inHeight;
            // reset the src index to the beginning the next line
            theSrcIndex += 4;
            // reset src index to the beginning of this line
            theSrcIndex -= theSrcLineLength;
        } // End of else for row boundary
    } // End of for loop for iterating through all rows
}

//==============================================================================
/**
 *	@param	inSrcBuffer
 */
void QDemonImageScaler::reduceCols(unsigned char *inSrcBuffer, long inSrcWidth, long inSrcHeight,
                              unsigned char *&outDstBuffer, long inDstWidth)
{
    long theDDAConst = static_cast<long>(1024.0 * inDstWidth / inSrcWidth);
    long theDDAAccum = 0L;
    long thePixelCount;

    long theSrcRow;
    long theSrcCol;
    long theDstCol;

    long theRedAccum;
    long theGreenAccum;
    long theBlueAccum;
    long theAlphaAccum;

    unsigned char *theDstPointer = outDstBuffer;
    unsigned char *theSrcPointer = inSrcBuffer;
    unsigned char *theSrcRowPointer;
    unsigned char *theDstRowPointer;

    long theSrcStepSize = 4;
    long theDstStepSize = 4;

    for (theSrcRow = 0; theSrcRow < inSrcHeight; ++theSrcRow) {

        theSrcRowPointer = theSrcPointer + (theSrcRow * inSrcWidth * theSrcStepSize);
        theDstRowPointer = theDstPointer + (theSrcRow * inDstWidth * theDstStepSize);

        theSrcCol = 0L;
        theDstCol = 0L;
        theRedAccum = 0L;
        theGreenAccum = 0L;
        theBlueAccum = 0L;
        theAlphaAccum = 0L;
        thePixelCount = 0L;
        theDDAAccum = 0L;

        while (theSrcCol < inSrcWidth) {
            while ((theDDAAccum < 1024L) && (theSrcCol < inSrcWidth)) {
                theRedAccum += 1024L * theSrcRowPointer[(theSrcCol * theSrcStepSize) + 0];
                theGreenAccum += 1024L * theSrcRowPointer[(theSrcCol * theSrcStepSize) + 1];
                theBlueAccum += 1024L * theSrcRowPointer[(theSrcCol * theSrcStepSize) + 2];
                theAlphaAccum += 1024L * theSrcRowPointer[(theSrcCol * theSrcStepSize) + 3];

                theDDAAccum += theDDAConst;
                thePixelCount += 1024L;
                ++theSrcCol;
            }

            theDDAAccum = (theSrcCol < inSrcWidth) ? (theDDAAccum - 1024L) : (0L);
            thePixelCount -= theDDAAccum;

            theRedAccum -=
                    theDDAAccum * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 0];
            theGreenAccum -=
                    theDDAAccum * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 1];
            theBlueAccum -=
                    theDDAAccum * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 2];
            theAlphaAccum -=
                    theDDAAccum * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 3];

            theDstRowPointer[(theDstCol * theDstStepSize) + 0] =
                    (unsigned char)(theRedAccum / thePixelCount);
            theDstRowPointer[(theDstCol * theDstStepSize) + 1] =
                    (unsigned char)(theGreenAccum / thePixelCount);
            theDstRowPointer[(theDstCol * theDstStepSize) + 2] =
                    (unsigned char)(theBlueAccum / thePixelCount);
            theDstRowPointer[(theDstCol * theDstStepSize) + 3] =
                    (unsigned char)(theAlphaAccum / thePixelCount);

            thePixelCount = 1024L - theDDAAccum;
            ++theDstCol;

            if (theDstCol >= inDstWidth) {
                break;
            }

            theRedAccum =
                    thePixelCount * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 0];
            theGreenAccum =
                    thePixelCount * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 1];
            theBlueAccum =
                    thePixelCount * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 2];
            theAlphaAccum =
                    thePixelCount * (long)theSrcRowPointer[((theSrcCol - 1) * theSrcStepSize) + 3];
        }
    }
}

//==============================================================================
/**
 *	@param	inSrcBuffer
 */
void QDemonImageScaler::reduceRows(unsigned char *inSrcBuffer, long inSrcWidth, long inSrcHeight,
                              unsigned char *&outDstBuffer, long inDstHeight)
{
    long theDDAConst = static_cast<long>(1024.0 * inDstHeight / inSrcHeight);
    long theDDAAccum = 0;
    long thePixelCount;

    long theSrcRow;
    long theSrcCol;
    long theDstRow;

    long theRedAccum;
    long theGreenAccum;
    long theBlueAccum;
    long theAlphaAccum;

    unsigned char *theDstPointer = outDstBuffer;
    unsigned char *theSrcPointer = inSrcBuffer;
    unsigned char *theSrcColPointer = nullptr;
    unsigned char *theDstColPointer = nullptr;

    long theStepSize = 4;
    long theSrcStride = 4 * inSrcWidth;
    long theDstStride = 4 * inSrcWidth;

    for (theSrcCol = 0; theSrcCol < inSrcWidth; ++theSrcCol) {
        theSrcColPointer = theSrcPointer + (theSrcCol * theStepSize);
        theDstColPointer = theDstPointer + (theSrcCol * theStepSize);

        theSrcRow = 0L;
        theDstRow = 0L;
        theRedAccum = 0L;
        theGreenAccum = 0L;
        theBlueAccum = 0L;
        theAlphaAccum = 0L;
        thePixelCount = 0L;

        theDDAAccum = 0L;

        while (theSrcRow < inSrcHeight) {
            while ((theDDAAccum < 1024L) && (theSrcRow < inSrcHeight)) {
                theRedAccum += 1024L * theSrcColPointer[(theSrcRow * theSrcStride) + 0];
                theGreenAccum += 1024L * theSrcColPointer[(theSrcRow * theSrcStride) + 1];
                theBlueAccum += 1024L * theSrcColPointer[(theSrcRow * theSrcStride) + 2];
                theAlphaAccum += 1024L * theSrcColPointer[(theSrcRow * theSrcStride) + 3];

                theDDAAccum += theDDAConst;
                thePixelCount += 1024L;
                ++theSrcRow;
            }

            theDDAAccum = (theSrcRow < inSrcHeight) ? (theDDAAccum - 1024L) : (0L);
            thePixelCount -= theDDAAccum;

            theRedAccum -=
                    theDDAAccum * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 0];
            theGreenAccum -=
                    theDDAAccum * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 1];
            theBlueAccum -=
                    theDDAAccum * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 2];
            theAlphaAccum -=
                    theDDAAccum * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 3];

            theDstColPointer[(theDstRow * theDstStride) + 0] =
                    (unsigned char)(theRedAccum / thePixelCount);
            theDstColPointer[(theDstRow * theDstStride) + 1] =
                    (unsigned char)(theGreenAccum / thePixelCount);
            theDstColPointer[(theDstRow * theDstStride) + 2] =
                    (unsigned char)(theBlueAccum / thePixelCount);
            theDstColPointer[(theDstRow * theDstStride) + 3] =
                    (unsigned char)(theAlphaAccum / thePixelCount);

            thePixelCount = 1024L - theDDAAccum;
            ++theDstRow;

            if (theDstRow >= inDstHeight) {
                break;
            }

            theRedAccum =
                    thePixelCount * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 0];
            theGreenAccum =
                    thePixelCount * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 1];
            theBlueAccum =
                    thePixelCount * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 2];
            theAlphaAccum =
                    thePixelCount * (long)theSrcColPointer[((theSrcRow - 1) * theSrcStride) + 3];
        }
    }
}

QT_END_NAMESPACE
