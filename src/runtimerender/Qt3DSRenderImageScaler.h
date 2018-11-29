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
//	Prefix
//==============================================================================
#ifndef __IMAGESCALER_H_
#define __IMAGESCALER_H_
#include <Qt3DSRender.h>

namespace qt3ds {
namespace render {
    //==============================================================================
    //	Class
    //==============================================================================
    //==============================================================================
    /**
     *	@class	CImageScaler
     */
    //==============================================================================
    class CImageScaler
    {
        NVAllocatorCallback &m_Allocator;

    public:
        //==============================================================================
        //	Methods
        //==============================================================================
        enum EScaleMethod {
            SCALEMETHOD_CROP = -1, // Debug only, not a scaler
            SCALEMETHOD_POINTSAMPLE = 0,
            SCALEMETHOD_BILINEAR = 1,
        };

        //==============================================================================
        //	Methods
        //==============================================================================

        // Access

    public:
        CImageScaler(NVAllocatorCallback &inAlloc);

        void Scale(EScaleMethod inScaleMethod, unsigned char *inOldBuffer, unsigned long inOldWidth,
                   unsigned long inOldHeight, unsigned char *&outNewBuffer,
                   unsigned long inNewWidth, unsigned long inNewHeight, unsigned long inChannels);

        void FastScale(EScaleMethod inScaleMethod, unsigned char *inOldBuffer,
                       unsigned long inOldWidth, unsigned long inOldHeight,
                       unsigned char *&outNewBuffer, unsigned long inNewWidth,
                       unsigned long inNewHeight, unsigned long inChannels);

        void Crop(unsigned char *inOldBuffer, unsigned long inOldWidth, unsigned long inOldHeight,
                  unsigned char *&outNewBuffer, unsigned long inNewWidth, unsigned long inNewHeight,
                  unsigned long inPlanes);

        void Bilinear(unsigned char *inOldBuffer, unsigned long inOldWidth,
                      unsigned long inOldHeight, unsigned char *&outNewBuffer,
                      unsigned long inNewWidth, unsigned long inNewHeight, unsigned long inPlanes);

        void FastPointSample(unsigned char *inOldBuffer, unsigned long inOldWidth,
                             unsigned long inOldHeight, unsigned char *&outNewBuffer,
                             unsigned long inNewWidth, unsigned long inNewHeight,
                             unsigned long inPlanes);

        unsigned char *AllocateBuffer(long inWidth, long inHeight);
        void ReleaseBuffer(unsigned char *&ioBuffer);
        void Resize(unsigned char *inOldBuffer, unsigned long inOldWidth, unsigned long inOldHeight,
                    unsigned char *&outNewBuffer, unsigned long inNewWidth,
                    unsigned long inNewHeight, unsigned long inPlanes);

        // variable numbers of planes, i.e. greyscale, rb, rbg, and rgba or argb
        // Bilinear algorithms, good for quality
        void ExpandRowsAndColumns(unsigned char *inBuffer, unsigned long inWidth,
                                  unsigned long inHeight, unsigned char *outBuffer,
                                  unsigned long inDstWidth, unsigned long inDstHeight,
                                  unsigned long inPlanes);

        // The method implemented above, but with some optimizations
        // specifically, fixed the number of planes at 4
        // eliminated the new/delete allocations
        void FastExpandRowsAndColumns(unsigned char *inBuffer, unsigned long inWidth,
                                      unsigned long inHeight, unsigned char *outBuffer,
                                      unsigned long inDstWidth, unsigned long inDstHeight);

        void ReduceCols(unsigned char *inSrcBuffer, long inSrcWidth, long inSrcHeight,
                        unsigned char *&outDstBuffer, long inDstWidth);
        void ReduceRows(unsigned char *inSrcBuffer, long inSrcWidth, long inSrcHeight,
                        unsigned char *&outDstBuffer, long inDstHeight);
    };
}
}

#endif // !defined(__IMAGESCALER_H_)
