/****************************************************************************
**
** Copyright (C) 2008-2016 NVIDIA Corporation.
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

#pragma once
#ifndef QDEMON_RENDER_LOAD_DDS_H
#define QDEMON_RENDER_LOAD_DDS_H

namespace qt3ds {
namespace render {

/** The maximum number of mipmap levels (per texture or per cubemap face) */
#define QDEMON_DDS_MAX_MIPMAPS (16)

/** The number of cubemap faces that must exist in a cubemap-bearing DDS file */
#define QDEMON_DDS_NUM_CUBEMAP_FACES (6)

    /** The master DDS structure for loading and saving

            This is the master DDS structure.  It shouldn't be allocated by hand,
            always use NVHHDDSAlloc/NVHHDDSAllocData/NVHHDDSFree to manage them properly.
    */

    struct Qt3DSDDSImage
    {
        /** Width of the overall texture in texels */
        int width;
        /** Height of the overall texture in texels */
        int height;
        /** Number of color/alpha components per texel 1-4 */
        int components;
        /** The GL type of each color component (noncompressed textures only) */
        int componentFormat;
        /** The number of bytes per pixel (noncompressed textures only) */
        int bytesPerPixel;
        /** Nonzero if the format is DXT-compressed */
        int compressed;
        /** The number of levels in the mipmap pyramid (including the base level) */
        int numMipmaps;
        /** If nonzero, then the file contains 6 cubemap faces */
        int cubemap;
        /** The GL format of the loaded texture data */
        int format;
        /** Nonzero if the texture data includes alpha */
        int alpha;
        /** Base of the allocated block of all texel data */
        void *dataBlock;
        /** Pointers to the mipmap levels for the texture or each cubemap face */
        void *data[QDEMON_DDS_MAX_MIPMAPS * QDEMON_DDS_NUM_CUBEMAP_FACES];
        /** Array of sizes of the mipmap levels for the texture or each cubemap face */
        int size[QDEMON_DDS_MAX_MIPMAPS * QDEMON_DDS_NUM_CUBEMAP_FACES];
        /** Array of widths of the mipmap levels for the texture or each cubemap face */
        int mipwidth[QDEMON_DDS_MAX_MIPMAPS * QDEMON_DDS_NUM_CUBEMAP_FACES];
        /** Array of heights of the mipmap levels for the texture or each cubemap face */
        int mipheight[QDEMON_DDS_MAX_MIPMAPS * QDEMON_DDS_NUM_CUBEMAP_FACES];
    };
}
}

#endif
